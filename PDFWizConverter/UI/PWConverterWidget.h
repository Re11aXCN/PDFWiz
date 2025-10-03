#pragma once

#include <QThreadPool>
#include <QMutex>
#include <QRunnable>
#include <QMetaObject>
#include <NXWidget.h>
#include "PWDef.h"
#include "Component/PWToolButton.h"

// C++20 标准库头文件
#include <mutex>
#include <condition_variable>
#include <future>
#include <thread>
#include <atomic>
#include <shared_mutex>
#include <latch>
#include <semaphore>
#include <unordered_map>
#include <memory>

class QStackedWidget;
class QButtonGroup;
class PWConverterWidget : public NXWidget {
    Q_OBJECT

public:
    explicit PWConverterWidget(QWidget* parent = nullptr);
    ~PWConverterWidget();

Q_SIGNALS:

protected:
    virtual void paintEvent(QPaintEvent* event) override;
    virtual void dragEnterEvent(QDragEnterEvent* event) override;
    virtual void dropEvent(QDropEvent* event) override;
    virtual bool eventFilter(QObject* obj, QEvent* event) override;

private Q_SLOTS:

private:
    void _initUI();
    void _initButtonComponent(const QJsonObject& buttonConfig);
    PWToolButton::MetaData _parseToolButtonMetaData(const QJsonObject& metadataObj);
    QHash<PWToolButton::MetaData::State, QColor> _parseToolButtonTextColor(const QJsonObject& textColorObj);
    
    void _handleAddFiles(WizConverter::Module::Enums::MasterModule::Type masterType);
    void _handleRemoveFiles(WizConverter::Module::Enums::MasterModule::Type masterType);
    void _distributeFilesToPDFToWord(QStackedWidget* stackedWidget,
        const QStringList& filePaths);
    void _distributeFilesToWordToPDF(QStackedWidget* stackedWidget,
        const QStringList& filePaths);
    void _distributeFilesToPDFAction(QStackedWidget* stackedWidget,
        const QStringList& filePaths);
    void _distributeFilesToImageAction(QStackedWidget* stackedWidget,
        const QStringList& filePaths);

    // 异步文件处理相关方法
    void _asyncFilterAndDistributeFiles(WizConverter::Module::Enums::MasterModule::Type masterType, const QStringList& filePaths);
    void _processCurrentModuleFiles(WizConverter::Module::Enums::MasterModule::Type masterType, int currentSlaveIndex, const QStringList& filePaths);

    void _distributeFilesToCurrentModule(WizConverter::Module::Enums::MasterModule::Type masterType, int slaveIndex, const QStringList& filePaths);
    void _distributeToOtherSlaveModules(WizConverter::Module::Enums::MasterModule::Type masterType, int currentSlaveIndex, const QStringList& filePaths);

    QHash<int, QStringList> _categorizeFilesForDistribution(WizConverter::Module::Enums::MasterModule::Type masterType, int currentSlaveIndex, const QStringList& filePaths);
    std::future<void> _createSlaveModuleAsync(WizConverter::Module::Enums::MasterModule::Type masterType, int slaveIndex);

    struct SlaveModuleMetadata {
        QButtonGroup* ButtonGroup{ nullptr };
        QStackedWidget* StackedWidget{ nullptr };
        QHash<int, bool> CreatedModules; // 记录哪些子模块已经创建
    };
    QHash<WizConverter::Module::Enums::MasterModule::Type, SlaveModuleMetadata> _pSlaveMetadataMap;
    //QHash<QString, QStackedWidget*> _pSlaveTableStackedWidget;

    QButtonGroup* _pMasterButtonGroup{ nullptr };
    QStackedWidget* _pMasterStackedWidget{ nullptr };

    // 线程安全相关成员
    QThreadPool* _pThreadPool{ nullptr };
    QMutex _pSlaveModuleMutex; // 保护子模块创建状态的互斥锁
    
    // C++20 同步机制成员变量
    std::shared_mutex _moduleCreationMutex;
    std::unordered_map<int, std::shared_ptr<std::latch>> _moduleCreationLatches;
    std::counting_semaphore<10> _distributionSemaphore{10}; // 限制并发分发任务数量
};
