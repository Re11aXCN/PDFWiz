#include "PWFileTableWidget.h"
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QDir>
#include <QResizeEvent>
#include <QFileInfo>
#include <QTableWidgetItem>

#include "Component/PWTableMaskWidget.h"
#include "Component/PWTableView.h"
PWFileTableWidget::PWFileTableWidget(QWidget* parent)
    : QWidget(parent)
{
    _pLayer = new QStackedWidget(this);
    _pTableMask = new PWTableMaskWidget(this);
    _pLayer->insertWidget(0, _pTableMask);
    _pLayer->insertWidget(1, new PWTableView(this));
    _pLayer->setCurrentIndex(1);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(_pLayer);

    QObject::connect(_pTableMask, &PWTableMaskWidget::pressed, this, &PWFileTableWidget::_openFileExplorer);
}

PWFileTableWidget::~PWFileTableWidget()
{
}

void PWFileTableWidget::setMask(const QPixmap& mask) {
    _pTableMask->setMask(mask);
}

void PWFileTableWidget::resizeEvent(QResizeEvent* event)
{
    _pLayer->setFixedSize(event->size());
    QWidget::resizeEvent(event);
}

void PWFileTableWidget::_openFileExplorer()
{
    QStringList fileAbsolutePaths = QFileDialog::getOpenFileNames(this,
        "选择文件",
        QDir::homePath(),
        _pFileFilter);

    /*auto* tableWidget = _pTabWidgetInfos[_pIndexPair.first].at(_pIndexPair.second).TableWidget;
    QString fileState = _pTabWidgetInfos[_pIndexPair.first].at(_pIndexPair.second).FileState;
    qobject_cast<PWTableViewModel*>(tableWidget->model())->processDocumentData(fileAbsolutePaths, fileState);
    if (_pIndexPair.first != 1)
        Q_EMIT requestCreateAllTabBarWidgets(_pIndexPair.first, _pIndexPair.second);*/
}

void PWFileTableWidget::addFiles(const QStringList& filePaths)
{
    // 实现添加文件到表格的逻辑
    // 这里可以根据实际需求实现具体的添加逻辑
    if (filePaths.isEmpty()) return;
    
    // 切换到表格视图
    _pLayer->setCurrentIndex(1);
    
    // 获取表格控件
    auto* tableWidget = qobject_cast<QTableWidget*>(_pLayer->widget(1));
    if (!tableWidget) return;
    
    // 添加文件到表格
    int currentRow = tableWidget->rowCount();
    for (const QString& filePath : filePaths) {
        tableWidget->insertRow(currentRow);
        QFileInfo fileInfo(filePath);
        tableWidget->setItem(currentRow, 0, new QTableWidgetItem(fileInfo.fileName()));
        tableWidget->setItem(currentRow, 1, new QTableWidgetItem(filePath));
        currentRow++;
    }
}

void PWFileTableWidget::removeAll()
{
    // 实现清空表格的逻辑
    auto* tableWidget = qobject_cast<QTableWidget*>(_pLayer->widget(1));
    if (tableWidget) {
        tableWidget->setRowCount(0);
    }
    
    // 如果表格为空，切换回遮罩视图
    if (!tableWidget || tableWidget->rowCount() == 0) {
        _pLayer->setCurrentIndex(0);
    }
}