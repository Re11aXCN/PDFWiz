#include "PWConverterWidget.h"
#include <vector>
#include <thread>

#include <QFileDialog>
#include <QStackedWidget>
#include <QButtonGroup>
#include <QVBoxLayout>
#include <QPainter>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QThreadPool>
#include <QRunnable>

#include <NXText.h>
#include <NXContentDialog.h>
#include <magic_enum/magic_enum.hpp>
#include <absl/container/flat_hash_map>
#include "PWLogger.h"
#include "PWCentralWidget.h"

constexpr int MAIN_WIDGET_WIDTH = 1080;
constexpr int MAIN_WIDGET_HEIGHT = 650;

constexpr int SLAVE_MODULE_WIDGET_WIDTH = 980;
constexpr int SLAVE_MODULE_WIDGET_HEIGHT = 515;
constexpr int SLAVE_MODULE_WIDGET_X = (MAIN_WIDGET_WIDTH - SLAVE_MODULE_WIDGET_WIDTH) >> 1;
constexpr int SLAVE_MODULE_WIDGET_BUTTON_AREA_HEIGHT = 85;
constexpr int SLAVE_MODULE_WIDGET_TABLE_AREA_HEIGHT = 335;

#define TABLE_VIEW_HEIGHT 335
#define COMPONENPWWIDGEPWHEIGHT 140
static QSet<QString> sTranslateExtensions = { "doc", "docx", "rtf", "txt" };
static QSet<QString> sImageActionExtensions = { "jpg", "jpeg", "png", "bmp", "tiff", "tif", "gif" };

using namespace WizConverter::Module::Enums;
namespace Scope::Utils {
    static QString GetFileTypeByExtension(const QString& extension) {
        static const absl::flat_hash_map<QString, QString> extensionToType = {
            {"doc", "word"}, {"docx", "word"}, {"rtf", "word"},
            {"xls", "excel"}, {"xlsx", "excel"},
            {"ppt", "powerpoint"}, {"pptx", "powerpoint"},
            {"jpg", "image"}, {"jpeg", "image"}, {"png", "image"},
            {"bmp", "image"}, {"svg", "image"}, {"tiff", "image"},
            {"tif", "image"}, {"gif", "image"},
            {"txt", "text"},
            {"dwg", "cad"}, {"dxf", "cad"},
            {"epub", "epub"},
            {"html", "html"}, {"htm", "html"},
            {"md", "markdown"}
        };
        auto it = extensionToType.find(extension.toLower());
        return it != extensionToType.end() ? it->second : QString{};
    }

    static QString GetExpectedFileTypeForSlave2(const QString& slaveName) {
        static const absl::flat_hash_map<QString, QString> slaveToFileType = {
            {"WordToPDF", "word"},
            {"ExcelToPDF", "excel"},
            {"PowerpointToPDF", "powerpoint"},
            {"ImageToPDF", "image"},
            {"TxtToPDF", "text"},
            {"CadToPDF", "cad"},
            {"EpubToPDF", "epub"},
            {"HtmlToPDF", "html"},
            {"MarkdownToPDF", "markdown"}
        };
        auto it = slaveToFileType.find(slaveName);
        return it != slaveToFileType.end() ? it->second : QString{};
    }

    static QSet<QString> ParseExtensionsFromFilter(const QString& filter) {
        QSet<QString> extensions;
        QRegularExpression regex(R"(\*\.(\w+))");
        QRegularExpressionMatchIterator it = regex.globalMatch(filter);

        while (it.hasNext()) {
            QRegularExpressionMatch match = it.next();
            extensions.insert(match.captured(1).toLower());
        }
        return extensions;
    }
    template<typename T>
    struct PropertyNameValue {
        QString Name;
        T Value;
    };
    template<typename T, typename Widget>
    static std::optional<PropertyNameValue<T>> GetPropertyValue(Widget widget, const QString& targetPropertyName = "") {
        // 1. 首先检查动态属性
        QVariant dynamicValue = widget->property(targetPropertyName.toUtf8().constData());
        if (dynamicValue.isValid() && dynamicValue.canConvert<T>()) {
            return std::make_optional(PropertyNameValue<T>{ targetPropertyName, dynamicValue.value<T>() });
        }

        // 2. 如果没有指定属性名，查找第一个可转换的属性
        if (targetPropertyName.isEmpty()) {
            // 先在内省属性中查找
            const QMetaObject* metaObject = widget->metaObject();
            int propertyCount = metaObject->propertyCount();
            for (int i = 0; i < propertyCount; ++i) {
                QMetaProperty property = metaObject->property(i);
                QVariant value = property.read(widget);
                if (value.canConvert<T>()) {
                    return std::make_optional(PropertyNameValue<T>{ property.name(), value.value<T>() });
                }
            }

            // 然后在动态属性中查找
            auto dynamicNames = widget->dynamicPropertyNames();
            for (const QByteArray& name : dynamicNames) {
                QVariant value = widget->property(name.constData());
                if (value.canConvert<T>()) {
                    return std::make_optional(PropertyNameValue<T>{ QString(name), value.value<T>() });
                }
            }
        }

        return std::nullopt;
    }
}

PWConverterWidget::PWConverterWidget(QWidget* parent)
    : NXWidget{ parent }
{
    setAcceptDrops(true);
    setIsFixedSize(true);
    setWindowTitle("");
    setFixedSize(MAIN_WIDGET_WIDTH, MAIN_WIDGET_HEIGHT);
    setWindowButtonFlags(NXAppBarType::StayTopButtonHint | NXAppBarType::MinimizeButtonHint | NXAppBarType::CloseButtonHint);
    
    _initUI();
}

PWConverterWidget::~PWConverterWidget()
{
}

void PWConverterWidget::paintEvent(QPaintEvent* event)
{
    NXWidget::paintEvent(event);
    QPainter painter(this);
    painter.save();
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
    painter.setPen(Qt::NoPen);
    QRect appBarRect(0, 0, width(), getAppBarHeight() + 1);
    painter.setBrush(QColor(0xFA, 0xFA, 0xFA));
    painter.drawRect(appBarRect);

    painter.setPen(QColor(0xC3, 0xC3, 0xE7));
    painter.drawLine(0, appBarRect.bottom(), width(), appBarRect.bottom());

    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(0xFF, 0xFF, 0xFF));
    painter.drawRect(QRect{ 
        0, getAppBarHeight() + SLAVE_MODULE_WIDGET_BUTTON_AREA_HEIGHT,
        width(), SLAVE_MODULE_WIDGET_HEIGHT
        });

    painter.restore();
}

void PWConverterWidget::dragEnterEvent(QDragEnterEvent* event)
{
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

void PWConverterWidget::dropEvent(QDropEvent* event)
{
    if (event->mimeData()->hasUrls()) {
    }
}

bool PWConverterWidget::eventFilter(QObject* obj, QEvent* event)
{
    return false;
}

void PWConverterWidget::_initUI()
{
    _pMasterButtonGroup = new QButtonGroup(this);
    _pMasterStackedWidget = new QStackedWidget(this);

    _pMasterButtonGroup->setExclusive(true);
    _pMasterStackedWidget->move(0, getAppBarHeight());
    _pMasterStackedWidget->setFixedSize(width(), height() - getAppBarHeight());

    QFile file(":/Resource/InitComponentConfig.json");
    if (!file.open(QIODevice::ReadOnly)) {
        //PW_ERROR("Failed to open InitComponentConfig.json");
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();

    if (doc.isNull() || !doc.isObject()) {
        //PW_ERROR("Failed to parse InitComponentConfig.json");
        return;
    }
    _initButtonComponent(doc.object());
}

void PWConverterWidget::_initButtonComponent(const QJsonObject& buttonConfig)
{
    // 解析json配置, 操作Component
    QJsonObject masterModuleComponent = buttonConfig["ModuleComponent"].toObject();
    QJsonObject selectOutputComponent = buttonConfig["SelectOutputComponent"].toObject();
    QJsonObject tableActionComponent = buttonConfig["TableActionComponent"].toObject();
    QJsonObject executeComponent = buttonConfig["ExecuteComponent"].toObject();
    QJsonObject selectOutputDirButtonObj = selectOutputComponent["SelectOutputDirButton"].toObject();
    QJsonObject addFilesButtonObj = tableActionComponent["AddFilesButton"].toObject();
    QJsonObject removeFilesButtonObj = tableActionComponent["RemoveFilesButton"].toObject();
    QJsonObject executeButtonObj = executeComponent["ExecuteButton"].toObject();

    // App 图标
    QPixmap iconPixmap = QPixmap(":/Resource/Image/PDFWiz-logo.svg");
    iconPixmap.setDevicePixelRatio(devicePixelRatio());
    iconPixmap.scaled(QSize(23, 23), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    // Master Module Button Area Widget
    QWidget* customAppBarWidget = new QWidget(this);
    QLabel* windowTitleLabel = new QLabel("PDF转换器", customAppBarWidget);
    QLabel* windowIconLabel = new QLabel(customAppBarWidget);
    windowIconLabel->setFixedSize(23, 23);
    windowIconLabel->setPixmap(iconPixmap);
    windowIconLabel->setScaledContents(true);
    QHBoxLayout* customAppBarLayout = new QHBoxLayout(customAppBarWidget);
    customAppBarLayout->setContentsMargins(0, 0, 0, 0);
    customAppBarLayout->addSpacing(15);
    customAppBarLayout->addWidget(windowIconLabel);
    customAppBarLayout->addSpacing(5);
    customAppBarLayout->addWidget(windowTitleLabel);
    customAppBarLayout->addSpacing(10);
    this->appBar()->setCustomWidget(NXAppBarType::LeftArea, customAppBarWidget);

    const auto& masterModuleMap = EnumTraits<MasterModule>::Map;
    // 解析主模块按钮
    for (auto masterIt = masterModuleMap.begin(); masterIt != masterModuleMap.end()-3; ++masterIt) {
        QJsonObject moduleObj = masterModuleComponent[(*masterIt).Name].toObject();
        PWToolButton* masterModuleButton = new PWToolButton(
            moduleObj["buttonText"].toString(),
            moduleObj["buttonIcon"].toString(),
            _parseToolButtonMetaData(masterModuleComponent["master_metadata"].toObject()), this);
        customAppBarLayout->addWidget(masterModuleButton);
        customAppBarLayout->addStretch();
        // 每个 MasterModule::Type 都有一组Add、Remove按钮应用于表格
        PWToolButton* addFilesButton = new PWToolButton(
            addFilesButtonObj["buttonText"].toString(),
            addFilesButtonObj["buttonIcon"].toString(),
            _parseToolButtonMetaData(tableActionComponent["metadata"].toObject()), this);
        PWToolButton* removeFilesButton = new PWToolButton(
            removeFilesButtonObj["buttonText"].toString(),
            removeFilesButtonObj["buttonIcon"].toString(),
            _parseToolButtonMetaData(tableActionComponent["metadata"].toObject()), this);

        _pMasterButtonGroup->addButton(masterModuleButton, static_cast<int>((*masterIt).Value));
        _pSlaveMetadataMap[(*masterIt).Value] = { .ButtonGroup = new QButtonGroup(this), .StackedWidget = new QStackedWidget(_pMasterStackedWidget) };
        _pSlaveMetadataMap[(*masterIt).Value].StackedWidget->setFixedSize(MAIN_WIDGET_WIDTH, SLAVE_MODULE_WIDGET_HEIGHT);
        _pSlaveMetadataMap[(*masterIt).Value].StackedWidget->move(_pMasterStackedWidget->x() + SLAVE_MODULE_WIDGET_X, _pMasterStackedWidget->y() + SLAVE_MODULE_WIDGET_BUTTON_AREA_HEIGHT);
        
        // 划分 顶部区域 给从模块按钮
        QWidget* slaveModuleButtonAreaWidget = new QWidget(this);
        slaveModuleButtonAreaWidget->setFixedSize(MAIN_WIDGET_WIDTH, SLAVE_MODULE_WIDGET_BUTTON_AREA_HEIGHT);
        QHBoxLayout* slaveModuleButtonAreaLayout = new QHBoxLayout(slaveModuleButtonAreaWidget);
        slaveModuleButtonAreaLayout->setSpacing(0);
        slaveModuleButtonAreaLayout->setContentsMargins(50, 15, 50, 15);

        auto lazyConstructComponentFunc = [this](MasterModule::Type masterType, auto&& slaveData, const QJsonObject& slaveBtnJsonObj) {
            PWCentralWidget* centralWidget = new PWCentralWidget(this);
            qDebug() << "Constructing " << centralWidget;
            centralWidget->setProperty(slaveData.Name, QVariant::fromValue(centralWidget));
            centralWidget->setFixedSize(SLAVE_MODULE_WIDGET_WIDTH, SLAVE_MODULE_WIDGET_TABLE_AREA_HEIGHT);
            centralWidget->setMask(QPixmap::fromImage(QImage(slaveBtnJsonObj["tableMask"].toString())));
            centralWidget->setFileFilter(slaveBtnJsonObj["fileFilter"].toString());
            centralWidget->setFileBereadyState(slaveBtnJsonObj["fileBereadyState"].toString());
            centralWidget->setModuleType({ masterType, QVariant::fromValue(slaveData.Value) });
            QWidget* centralWidgetWrapepr = new QWidget(this);
            QVBoxLayout* centralWidgetLayout = new QVBoxLayout(centralWidgetWrapepr);
            centralWidgetLayout->setAlignment(Qt::AlignCenter);
            centralWidgetLayout->setSpacing(0);
            centralWidgetLayout->setContentsMargins(0, 15, 0, 0);
            centralWidgetLayout->addWidget(centralWidget);
            centralWidgetLayout->addStretch();

            QStackedWidget* stackedWidget = _pSlaveMetadataMap[masterType].StackedWidget;
            if (QWidget* dummyWidget = stackedWidget->widget(static_cast<int>(slaveData.Value))) {
                stackedWidget->removeWidget(dummyWidget);
                dummyWidget->deleteLater();
            }
            stackedWidget->insertWidget(static_cast<int>(slaveData.Value), centralWidgetWrapepr);
            stackedWidget->setCurrentIndex(static_cast<int>(slaveData.Value));
            /*PWToolButton* selectOutputDirButton = new PWToolButton(
                selectOutputDirButtonObj["buttonText"].toString(),
                selectOutputDirButtonObj["buttonIcon"].toString(),
                _parseToolButtonMetaData(selectOutputComponent["metadata"].toObject()), this);

            PWToolButton* executeButton = new PWToolButton(
                slaveModuleButtonObj["executeButtonText"].toString(),
                executeButtonObj["buttonIcon"].toString(),
                _parseToolButtonMetaData(executeComponent["metadata"].toObject()), this);*/

            };
        // 解析子模块按钮
        std::visit([&](auto&& array) {
            using T = std::decay_t<decltype(array)>;
            if constexpr (!std::is_same_v<T, std::monostate>) {
                for (auto slaveIt = array.begin(); slaveIt != array.end(); ++slaveIt) {
                    QJsonObject slaveModuleButtonObj = moduleObj["slaveModuleComponent"].toObject()[(*slaveIt).Name].toObject();

                    PWToolButton* slaveModuleButton = new PWToolButton(
                        slaveModuleButtonObj["buttonText"].toString(),
                        slaveModuleButtonObj["buttonIcon"].toString(),
                        _parseToolButtonMetaData(masterModuleComponent["slave_metadata"].toObject()), this);

                    slaveModuleButtonAreaLayout->addWidget(slaveModuleButton);
                    slaveModuleButtonAreaLayout->addSpacing(3);
                    _pSlaveMetadataMap[(*masterIt).Value].ButtonGroup->addButton(slaveModuleButton, static_cast<int>((*slaveIt).Value));
                    if (slaveIt == array.begin()) {
                        lazyConstructComponentFunc((*masterIt).Value, *slaveIt, slaveModuleButtonObj);
                        // 标记该子模块已创建
                        _pSlaveMetadataMap[(*masterIt).Value].CreatedModules[static_cast<int>((*slaveIt).Value)] = true;
                    }
                    else {
                        // 预留位置, 用于后续扩展
                        _pSlaveMetadataMap[(*masterIt).Value].StackedWidget->insertWidget(static_cast<int>((*slaveIt).Value), new QWidget(this));
                        QObject::connect(slaveModuleButton, &QToolButton::pressed, this,
                            [func = lazyConstructComponentFunc,
                            masterType = (*masterIt).Value,
                            slaveData = *slaveIt,
                            slaveBtnJsonObj = std::move(slaveModuleButtonObj), this]()
                        {
                            func(masterType, slaveData, slaveBtnJsonObj);
                            // 标记该子模块已创建
                            std::lock_guard<std::mutex> locker(_pSlaveModuleMutex);
                            _pSlaveMetadataMap[masterType].CreatedModules[static_cast<int>(slaveData.Value)] = true;
                        }, Qt::SingleShotConnection);
                    }
                }
            }
            else throw std::runtime_error("Invalid slave module type map");
            }, getSlaveModuleType((*masterIt).Value));

        slaveModuleButtonAreaLayout->addStretch();
        slaveModuleButtonAreaLayout->addWidget(addFilesButton);
        slaveModuleButtonAreaLayout->addSpacing(5);
        slaveModuleButtonAreaLayout->addWidget(removeFilesButton);

        // 初始显示第一个子模块
        _pSlaveMetadataMap[(*masterIt).Value].StackedWidget->setCurrentIndex(0);
        _pSlaveMetadataMap[(*masterIt).Value].ButtonGroup->button(0)->setChecked(true);

        QWidget* slaveModuleWidget = new QWidget(this);
        QVBoxLayout* slaveModuleLayout = new QVBoxLayout(slaveModuleWidget);
        slaveModuleLayout->setSpacing(0);
        slaveModuleLayout->setContentsMargins(0, 0, 0, 0);
        slaveModuleLayout->insertWidget(0, slaveModuleButtonAreaWidget);
        slaveModuleLayout->insertWidget(1, _pSlaveMetadataMap[(*masterIt).Value].StackedWidget);// 除了<MasterModuleButton区域+SlaveModuleButton区域>的区域
        _pMasterStackedWidget->insertWidget(static_cast<int>((*masterIt).Value), slaveModuleWidget); // 除了<MasterModuleButton区域>的区域
        
        QObject::connect(_pSlaveMetadataMap[(*masterIt).Value].ButtonGroup, &QButtonGroup::idReleased, this, [this, masterType = (*masterIt).Value](int id) {
            _pSlaveMetadataMap[masterType].StackedWidget->setCurrentIndex(id);
            });
        QObject::connect(addFilesButton, &QToolButton::released, this, [type = (*masterIt).Value, this, addFilesButton]() {
                this->_handleAddFiles(type);
            });
        QObject::connect(removeFilesButton, &QToolButton::released, this, [type = (*masterIt).Value, this]() {
                this->_handleRemoveFiles(type);
            });
    }
    // 初始显示第一个主模块
    _pMasterStackedWidget->setCurrentIndex(0);
    _pMasterButtonGroup->button(0)->setChecked(true);

    QObject::connect(_pMasterButtonGroup, &QButtonGroup::idReleased, this, [this](int id) {
        _pMasterStackedWidget->setCurrentIndex(id);
        });
}

PWToolButton::MetaData PWConverterWidget::_parseToolButtonMetaData(const QJsonObject& metadataObj)
{
    PWToolButton::MetaData metaData;

    // 解析字体像素大小
    if (metadataObj.contains("fontPixelSize")) {
        metaData.FontPixelSize = metadataObj["fontPixelSize"].toInt();
    }

    // 解析文本区域
    if (metadataObj.contains("textRect")) {
        QJsonObject textRectObj = metadataObj["textRect"].toObject();
        metaData.TextRect = QRect(
            textRectObj["x"].toInt(),
            textRectObj["y"].toInt(),
            textRectObj["width"].toInt(),
            textRectObj["height"].toInt()
        );
    }

    // 解析按钮尺寸
    if (metadataObj.contains("buttonSize")) {
        QJsonObject sizeObj = metadataObj["buttonSize"].toObject();
        metaData.ButtonSize = QSize(
            sizeObj["width"].toInt(),
            sizeObj["height"].toInt()
        );
    }

    // 解析字体
    if (metadataObj.contains("font")) {
        metaData.Font = QFont(metadataObj["font"].toString());
    }

    // 解析样式表
    if (metadataObj.contains("qss")) {
        metaData.Qss = metadataObj["qss"].toString();
    }

    // 解析文本颜色
    if (metadataObj.contains("textColor")) {
        metaData.TextColor = _parseToolButtonTextColor(metadataObj["textColor"].toObject());
    }

    return metaData;
}

QHash<PWToolButton::MetaData::State, QColor> PWConverterWidget::_parseToolButtonTextColor(const QJsonObject& textColorObj)
{
    QHash<PWToolButton::MetaData::State, QColor> textColor;

    if (textColorObj.contains("normal")) {
        textColor[PWToolButton::MetaData::Normal] = QColor(textColorObj["normal"].toString());
    }
    if (textColorObj.contains("hovered")) {
        textColor[PWToolButton::MetaData::Hovered] = QColor(textColorObj["hovered"].toString());
    }
    if (textColorObj.contains("pressed")) {
        textColor[PWToolButton::MetaData::Pressed] = QColor(textColorObj["pressed"].toString());
    }
    if (textColorObj.contains("unavailable")) {
        const QString& colorStr = textColorObj["unavailable"].toString();
        if (!colorStr.isEmpty()) {
            textColor[PWToolButton::MetaData::Unavailable] = QColor(colorStr);
        }
    }

    return textColor;
}

void PWConverterWidget::_handleAddFiles(WizConverter::Module::Enums::MasterModule::Type masterType) {
    // 获取所有文件路径
    QStringList fileAbsolutePaths = QFileDialog::getOpenFileNames(this,
        "选择文件",
        QDir::homePath(),
        "All Files (*.*)");

    if (fileAbsolutePaths.isEmpty()) return;

    // 异步处理文件过滤和分发
    _asyncFilterAndDistributeFiles(masterType, fileAbsolutePaths);
}

void PWConverterWidget::_handleRemoveFiles(WizConverter::Module::Enums::MasterModule::Type masterType)
{
    auto& slaveMetadata = _pSlaveMetadataMap[masterType];
    QStackedWidget* stackedWidget = slaveMetadata.StackedWidget;
    if (!stackedWidget) return;

    enum class RemoveType { All, AllSelected, AllNotSelected };
    auto removeFilesFunc = [](QStackedWidget* moduleStackedWidget, RemoveType removeType) static {
        for (int i = 0; i < moduleStackedWidget->count(); ++i) {
            QWidget* wrapperWidget = moduleStackedWidget->widget(i);
            if (!wrapperWidget) continue;
            PWCentralWidget* tableWidget = wrapperWidget->findChild<PWCentralWidget*>("PWCentralWidget");
            if (!tableWidget) continue;

            if (auto tableWidgetPtr = Scope::Utils::GetPropertyValue<PWCentralWidget*>(tableWidget);
                tableWidgetPtr.has_value())
            {
                switch (removeType) {
                case RemoveType::All: tableWidgetPtr.value().Value->removeAll(); break;
                case RemoveType::AllSelected: tableWidgetPtr.value().Value->removeAllSelected(); break;
                case RemoveType::AllNotSelected: tableWidgetPtr.value().Value->removeAllNotSelected(); break;
                }
            }
        }
        };

    NXContentDialog removeFilesDialog(this);
    QWidget contentWidget(this);
    QVBoxLayout contentVLayout(&contentWidget);
    contentVLayout.setContentsMargins(15, 0, 15, 10);
    NXText title("清除方式", this);
    title.setTextStyle(NXTextType::TitleLarge);
    NXText subTitle("请选择清除方式, 关闭窗口退出", this);
    subTitle.setTextStyle(NXTextType::Subtitle);
    contentVLayout.addWidget(&title);
    contentVLayout.addSpacing(2);
    contentVLayout.addWidget(&subTitle);
    contentVLayout.addStretch();
    removeFilesDialog.appBar()->setAppBarHeight(40);
    removeFilesDialog.appBar()->setWindowButtonFlags(NXAppBarType::CloseButtonHint);
    removeFilesDialog.setCentralWidget(&contentWidget);
    removeFilesDialog.setLeftButtonText("清除所有已选");
    removeFilesDialog.setMiddleButtonText("清除所有未选");
    removeFilesDialog.setRightButtonText("清除所有");
    QObject::connect(&removeFilesDialog, &NXContentDialog::leftButtonClicked, this, [&]() {
        removeFilesFunc(stackedWidget, RemoveType::AllSelected);
        });
    QObject::connect(&removeFilesDialog, &NXContentDialog::middleButtonClicked, this, [&]() {
        removeFilesFunc(stackedWidget, RemoveType::AllNotSelected);
        removeFilesDialog.close();
        });
    QObject::connect(&removeFilesDialog, &NXContentDialog::rightButtonClicked, this, [&]() {
        removeFilesFunc(stackedWidget, RemoveType::All);
        });
    removeFilesDialog.exec();
}

// 主模块一：所有子模块表格都接收PDF文件
void PWConverterWidget::_distributeFilesToPDFToWord(QStackedWidget* stackedWidget, const QStringList& filePaths)
{
    // 过滤出符合条件的文件
    QStringList filteredFiles;
    for (const QString& filePath : filePaths) {
        QFileInfo fileInfo(filePath);
        if (fileInfo.suffix().toLower() == "pdf") {
            filteredFiles.append(filePath);
        }
    }

    if (filteredFiles.isEmpty()) return;

    // 分发给所有表格
    for (int i = 0; i < stackedWidget->count(); ++i) {
        QWidget* wrapperWidget = stackedWidget->widget(i);
        if (!wrapperWidget) continue;
        PWCentralWidget* tableWidget = wrapperWidget->findChild<PWCentralWidget*>("PWCentralWidget");
        if (!tableWidget) continue;

        if (auto tableWidgetPtr = Scope::Utils::GetPropertyValue<PWCentralWidget*>(tableWidget);
            tableWidgetPtr.has_value())
        {
            tableWidgetPtr.value().Value->addFiles(filteredFiles);
        }
    }
}

// 主模块二：根据文件类型分发到不同的子模块表格
void PWConverterWidget::_distributeFilesToWordToPDF(QStackedWidget* stackedWidget, const QStringList& filePaths) {
    // 按文件类型分组
    QHash<QString, QStringList> filesByType;

    for (const QString& filePath : filePaths) {
        QFileInfo fileInfo(filePath);
        const QString& fileType = Scope::Utils::GetFileTypeByExtension(fileInfo.suffix().toLower());

        if (!fileType.isEmpty()) {
            filesByType[fileType].append(filePath);
        }
    }

    // 分发到对应的子模块表格
    for (int i = 0; i < stackedWidget->count(); ++i) {
        QWidget* wrapperWidget = stackedWidget->widget(i);
        if (!wrapperWidget) continue;
        PWCentralWidget* tableWidget = wrapperWidget->findChild<PWCentralWidget*>("PWCentralWidget");
        if (!tableWidget) continue;

        if (auto tableWidgetPtr = Scope::Utils::GetPropertyValue<PWCentralWidget*>(tableWidget);
            tableWidgetPtr.has_value())
        {
            auto [slaveName, ptr] = tableWidgetPtr.value();
            const QString& expectedFileType = Scope::Utils::GetExpectedFileTypeForSlave2(slaveName);
            if (filesByType.contains(expectedFileType)) {
                ptr->addFiles(filesByType[expectedFileType]);
            }
        }
    }
}

// 主模块三：PDFAction的特殊处理
void PWConverterWidget::_distributeFilesToPDFAction(QStackedWidget* stackedWidget, const QStringList& filePaths) {
    // 分离PDF文件和其他文档文件
    QStringList pdfFiles;
    QStringList documentFiles;
    
    for (const QString& filePath : filePaths) {
        QFileInfo fileInfo(filePath);
        const QString& suffix = fileInfo.suffix().toLower();

        if (suffix == "pdf") {
            pdfFiles.append(filePath);
            documentFiles.append(filePath); // PDF文件也属于文档文件
        }
        else if (sTranslateExtensions.contains(suffix)) {
            documentFiles.append(filePath);
        }
    }

    // 分发文件
    for (int i = 0; i < stackedWidget->count(); ++i) {
        QWidget* wrapperWidget = stackedWidget->widget(i);
        if (!wrapperWidget) continue;
        PWCentralWidget* tableWidget = wrapperWidget->findChild<PWCentralWidget*>("PWCentralWidget");
        if (!tableWidget) continue;

        if (auto tableWidgetPtr = Scope::Utils::GetPropertyValue<PWCentralWidget*>(tableWidget);
            tableWidgetPtr.has_value())
        {
            auto [slaveName, ptr] = tableWidgetPtr.value();
            ptr->addFiles(slaveName == "DocumentTranslate" ? documentFiles : pdfFiles);
        }
    }
}

// 主模块四：ImageAction的特殊处理
void PWConverterWidget::_distributeFilesToImageAction(QStackedWidget* stackedWidget, const QStringList& filePaths) {
    // 分离PDF文件和图片文件
    QStringList pdfFiles;
    QStringList imageFiles;
    for (const QString& filePath : filePaths) {
        QFileInfo fileInfo(filePath);
        const QString& suffix = fileInfo.suffix().toLower();

        if (suffix == "pdf") {
            pdfFiles.append(filePath);
        }
        else if (sImageActionExtensions.contains(suffix)) {
            imageFiles.append(filePath);
        }
    }

    // 分发文件
    for (int i = 0; i < stackedWidget->count(); ++i) {
        QWidget* wrapperWidget = stackedWidget->widget(i);
        if (!wrapperWidget) continue;
        PWCentralWidget* tableWidget = wrapperWidget->findChild<PWCentralWidget*>("PWCentralWidget");
        if (!tableWidget) continue;

        if (auto tableWidgetPtr = Scope::Utils::GetPropertyValue<PWCentralWidget*>(tableWidget);
            tableWidgetPtr.has_value())
        {
            auto [slaveName, ptr] = tableWidgetPtr.value();
            ptr->addFiles(slaveName == "PDFToImage" ? pdfFiles : imageFiles);
        }
    }
}

void PWConverterWidget::_asyncFilterAndDistributeFiles(WizConverter::Module::Enums::MasterModule::Type masterType, const QStringList& filePaths) {
    
    // 异步文件过滤和分发任务类
    class AsyncFileFilterTask : public QRunnable {
    public:
        AsyncFileFilterTask(PWConverterWidget* widget, MasterModule::Type masterType, const QStringList& filePaths)
            : _pWidget(widget), _pMasterType(masterType), _pFilePaths(filePaths) {
            setAutoDelete(true);
        }

        void run() override {
            if (!_pWidget) return;

            // 获取当前子模块索引
            int currentSlaveIndex = -1;
            QMetaObject::invokeMethod(_pWidget, [this, &currentSlaveIndex]() {
                auto& slaveMetadata = _pWidget->_pSlaveMetadataMap[_pMasterType];
                if (slaveMetadata.StackedWidget) {
                    currentSlaveIndex = slaveMetadata.StackedWidget->currentIndex();
                }
            }, Qt::BlockingQueuedConnection);

            if (currentSlaveIndex < 0) return;

            // 第一步：处理当前模块的文件
            _pWidget->_processCurrentModuleFiles(_pMasterType, currentSlaveIndex, _pFilePaths);
            
            // 第二步：异步分发到其他子模块
            _pWidget->_distributeToOtherSlaveModules(_pMasterType, currentSlaveIndex, _pFilePaths);
        }

    private:
        PWConverterWidget* _pWidget;
        MasterModule::Type _pMasterType;
        QStringList _pFilePaths;
    };
    AsyncFileFilterTask* task = new AsyncFileFilterTask(this, masterType, filePaths);
    QThreadPool::globalInstance()->start(task);
}

void PWConverterWidget::_distributeFilesToCurrentModule(WizConverter::Module::Enums::MasterModule::Type masterType, int slaveIndex, const QStringList& filePaths) {
    auto& slaveMetadata = _pSlaveMetadataMap[masterType];
    QStackedWidget* stackedWidget = slaveMetadata.StackedWidget;

    if (!stackedWidget || slaveIndex < 0 || slaveIndex >= stackedWidget->count()) return;

    QWidget* wrapperWidget = stackedWidget->widget(slaveIndex);
    if (!wrapperWidget) return;
    PWCentralWidget* tableWidget = wrapperWidget->findChild<PWCentralWidget*>("PWCentralWidget");
    if (!tableWidget) return;

    if (auto tableWidgetPtr = Scope::Utils::GetPropertyValue<PWCentralWidget*>(tableWidget);
        tableWidgetPtr.has_value())
    {
        tableWidgetPtr.value().Value->addFiles(filePaths);
    }
}

void PWConverterWidget::_processCurrentModuleFiles(WizConverter::Module::Enums::MasterModule::Type masterType, int currentSlaveIndex, const QStringList& filePaths) {
    
    QStringList currentModuleFiles;
    
    // 根据主模块类型和当前子模块过滤文件
    for (const QString& filePath : filePaths) {
        QFileInfo fileInfo(filePath);
        const QString& suffix = fileInfo.suffix().toLower();
        const QString& fileType = Scope::Utils::GetFileTypeByExtension(suffix);
        
        bool shouldAddToCurrentModule = false;
        
        switch (masterType) {
        case MasterModule::Type::PDFToWord:
            // PDFToWord主模块：所有子模块都接收PDF文件
            shouldAddToCurrentModule = (suffix == "pdf");
            break;
            
        case MasterModule::Type::WordToPDF: {
            // WordToPDF主模块：根据文件类型匹配对应的子模块
            QString expectedType = Scope::Utils::GetExpectedFileTypeForSlave2(
                QString::fromUtf8(magic_enum::enum_name(static_cast<WordToPDF::Type>(currentSlaveIndex)).data()));
            shouldAddToCurrentModule = (fileType == expectedType);
            break;
        }
        
        case MasterModule::Type::PDFAction:
            // PDFAction主模块：PDF文件给所有模块，翻译文件只给DocumentTranslate模块
            if (suffix == "pdf") {
                shouldAddToCurrentModule = true;
            } else if (sTranslateExtensions.contains(suffix)) {
                shouldAddToCurrentModule = (static_cast<PDFAction::Type>(currentSlaveIndex) == PDFAction::Type::DocumentTranslate);
            }
            break;
            
        case MasterModule::Type::ImageAction: {
            // ImageAction主模块：复杂的分发逻辑
            if (static_cast<ImageAction::Type>(currentSlaveIndex) == ImageAction::Type::PDFToImage) {
                shouldAddToCurrentModule = (suffix == "pdf");
            } else {
                shouldAddToCurrentModule = sImageActionExtensions.contains(suffix);
            }
            break;
        }
        }
        
        if (shouldAddToCurrentModule) {
            currentModuleFiles.append(filePath);
        }
    }
    
    // 将文件添加到当前模块
    if (!currentModuleFiles.isEmpty()) {
        QMetaObject::invokeMethod(this, [this, masterType, currentSlaveIndex, currentModuleFiles]() {
            _distributeFilesToCurrentModule(masterType, currentSlaveIndex, currentModuleFiles);
        }, Qt::QueuedConnection);
    }
}

void PWConverterWidget::_distributeToOtherSlaveModules(WizConverter::Module::Enums::MasterModule::Type masterType, int currentSlaveIndex, const QStringList& filePaths) 
{
    
    // 获取需要分发的文件分类
    QHash<int, QStringList> moduleFileMap = _categorizeFilesForDistribution(masterType, currentSlaveIndex, filePaths);
    
    if (moduleFileMap.isEmpty()) return;
    
    // 创建异步任务处理分发
    class AsyncDistributionTask : public QRunnable {
    public:
        AsyncDistributionTask(PWConverterWidget* widget, MasterModule::Type masterType, QHash<int, QStringList> moduleFileMap)
            : _widget(widget), _masterType(masterType), _moduleFileMap(std::move(moduleFileMap)) {
            setAutoDelete(true);
        }
        
        void run() override {
            if (!_widget) return;
            
            // 获取信号量
            _widget->_distributionSemaphore.acquire();
            
            try {
                std::vector<std::future<void>> futures;
                
                // 为每个目标模块创建异步任务
                for (auto it = _moduleFileMap.begin(); it != _moduleFileMap.end(); ++it) {
                    int slaveIndex = it.key();
                    const QStringList& files = it.value();
                    
                    if (files.isEmpty()) continue;
                    
                    // 异步创建子模块并分发文件
                    auto future = _widget->_createSlaveModuleAsync(_masterType, slaveIndex);
                    futures.push_back(std::move(future));
                    
                    // 等待模块创建完成后分发文件
                    std::thread([widget = _widget, masterType = _masterType, slaveIndex, files, future = std::move(futures.back())]() mutable {
                        try {
                            future.wait(); // 等待模块创建完成
                            
                            // 在主线程中分发文件
                            QMetaObject::invokeMethod(widget, [widget, masterType, slaveIndex, files]() {
                                widget->_distributeFilesToCurrentModule(masterType, slaveIndex, files);
                            }, Qt::QueuedConnection);
                        }
                        catch (...) {
                            // 异常处理
                        }
                    }).detach();
                }
            }
            catch (...) {
                // 异常处理
            }
            
            // 释放信号量
            _widget->_distributionSemaphore.release();
        }
        
    private:
        PWConverterWidget* _widget;
        MasterModule::Type _masterType;
        QHash<int, QStringList> _moduleFileMap;
    };
    
    AsyncDistributionTask* task = new AsyncDistributionTask(this, masterType, std::move(moduleFileMap));
    QThreadPool::globalInstance()->start(task);
}

QHash<int, QStringList> PWConverterWidget::_categorizeFilesForDistribution(WizConverter::Module::Enums::MasterModule::Type masterType, int currentSlaveIndex, const QStringList& filePaths) {
    
    QHash<int, QStringList> moduleFileMap;
    
    for (const QString& filePath : filePaths) {
        QFileInfo fileInfo(filePath);
        const QString& suffix = fileInfo.suffix().toLower();
        const QString& fileType = Scope::Utils::GetFileTypeByExtension(suffix);
        
        switch (masterType) {
        case MasterModule::Type::PDFToWord: {
            // PDFToWord：PDF文件分发给所有其他8个子模块
            if (suffix == "pdf") {
                const auto& enumArray = EnumTraits<PDFToWord>::Map;
                for (const auto& enumItem : enumArray) {
                    int slaveIndex = static_cast<int>(enumItem.Value);
                    if (slaveIndex != currentSlaveIndex) {
                        moduleFileMap[slaveIndex].append(filePath);
                    }
                }
            }
            break;
        }
        
        case MasterModule::Type::WordToPDF: {
            // WordToPDF：根据文件类型分发到对应的子模块
            int targetSlaveIndex = -1;
            if (fileType == "word") targetSlaveIndex = static_cast<int>(WordToPDF::Type::WordToPDF);
            else if (fileType == "excel") targetSlaveIndex = static_cast<int>(WordToPDF::Type::ExcelToPDF);
            else if (fileType == "powerpoint") targetSlaveIndex = static_cast<int>(WordToPDF::Type::PowerpointToPDF);
            else if (fileType == "image") targetSlaveIndex = static_cast<int>(WordToPDF::Type::ImageToPDF);
            else if (fileType == "text") targetSlaveIndex = static_cast<int>(WordToPDF::Type::TxtToPDF);
            else if (fileType == "cad") targetSlaveIndex = static_cast<int>(WordToPDF::Type::CadToPDF);
            else if (fileType == "epub") targetSlaveIndex = static_cast<int>(WordToPDF::Type::EpubToPDF);
            else if (fileType == "html") targetSlaveIndex = static_cast<int>(WordToPDF::Type::HtmlToPDF);
            else if (fileType == "markdown") targetSlaveIndex = static_cast<int>(WordToPDF::Type::MarkdownToPDF);
            
            if (targetSlaveIndex >= 0 && targetSlaveIndex != currentSlaveIndex) {
                moduleFileMap[targetSlaveIndex].append(filePath);
            }
            break;
        }
        
        case MasterModule::Type::PDFAction: {
            if (suffix == "pdf") {
                // PDF文件分发给所有其他8个子模块
                const auto& enumArray = EnumTraits<PDFAction>::Map;
                for (const auto& enumItem : enumArray) {
                    int slaveIndex = static_cast<int>(enumItem.Value);
                    if (slaveIndex != currentSlaveIndex) {
                        moduleFileMap[slaveIndex].append(filePath);
                    }
                }
            } else if (sTranslateExtensions.contains(suffix)) {
                // 翻译文件只分发给DocumentTranslate模块
                int translateIndex = static_cast<int>(PDFAction::Type::DocumentTranslate);
                if (currentSlaveIndex != translateIndex) {
                    moduleFileMap[translateIndex].append(filePath);
                }
            }
            break;
        }
        
        case MasterModule::Type::ImageAction: {
            if (sImageActionExtensions.contains(suffix)) {
                // 图片文件分发给除PDFToImage外的其他7个子模块
                const auto& enumArray = EnumTraits<ImageAction>::Map;
                for (const auto& enumItem : enumArray) {
                    int slaveIndex = static_cast<int>(enumItem.Value);
                    if (slaveIndex != currentSlaveIndex && 
                        enumItem.Value != ImageAction::Type::PDFToImage) {
                        moduleFileMap[slaveIndex].append(filePath);
                    }
                }
            } else if (suffix == "pdf") {
                // PDF文件只分发给PDFToImage模块
                int pdfToImageIndex = static_cast<int>(ImageAction::Type::PDFToImage);
                if (currentSlaveIndex != pdfToImageIndex) {
                    moduleFileMap[pdfToImageIndex].append(filePath);
                }
            }
            break;
        }
        }
    }
    
    return moduleFileMap;
}

std::future<void> PWConverterWidget::_createSlaveModuleAsync(WizConverter::Module::Enums::MasterModule::Type masterType, int slaveIndex) {
    return std::async(std::launch::async, [this, masterType, slaveIndex]() {
        // 检查模块是否已经创建
        {
            std::shared_lock<std::shared_mutex> lock(_moduleCreationMutex);
            if (_pSlaveMetadataMap[masterType].CreatedModules.value(slaveIndex, false)) {
                return; // 已经创建
            }
        }
        
        // 创建latch用于同步
        auto latch = std::make_shared<std::latch>(1);
        
        {
            std::unique_lock<std::shared_mutex> lock(_moduleCreationMutex);
            if (!_pSlaveMetadataMap[masterType].CreatedModules.value(slaveIndex, false)) {
                _moduleCreationLatches[slaveIndex] = latch;
            } else {
                return; // 已经创建
            }
        }
        
        // 在主线程中触发模块创建
        QMetaObject::invokeMethod(this, [this, masterType, slaveIndex, latch]() {
            auto& slaveMetadata = _pSlaveMetadataMap[masterType];
            if (slaveMetadata.ButtonGroup) {
                QAbstractButton* button = slaveMetadata.ButtonGroup->button(slaveIndex);
                if (button && !_pSlaveMetadataMap[masterType].CreatedModules.value(slaveIndex, false)) {
                    // 连接信号，在模块创建完成后释放latch
                    QObject::connect(button, &QAbstractButton::pressed, this, [this, masterType, slaveIndex, latch]() {
                        // 标记模块已创建
                        {
                            std::unique_lock<std::shared_mutex> lock(_moduleCreationMutex);
                            _pSlaveMetadataMap[masterType].CreatedModules[slaveIndex] = true;
                        }
                        // 释放latch
                        latch->count_down();
                    }, Qt::SingleShotConnection);
                    
                    button->click(); // 触发延迟构造
                } else {
                    latch->count_down(); // 如果按钮不存在或已创建，直接释放
                }
            } else {
                latch->count_down(); // 如果ButtonGroup不存在，直接释放
            }
        }, Qt::QueuedConnection);
        
        // 等待模块创建完成
        latch->wait();
        
        // 清理latch
        {
            std::unique_lock<std::shared_mutex> lock(_moduleCreationMutex);
            _moduleCreationLatches.erase(slaveIndex);
        }
    });
}
