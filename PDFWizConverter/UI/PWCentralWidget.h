#pragma once

#include <QWidget>
#include <NXProperty.h>
#include "PWDef.h"
class QStackedWidget;
class PWTableView;
class PWTableViewModel;
class PWTableMaskWidget;
class PWCentralWidget : public QWidget
{
    Q_OBJECT
    Q_PRIVATE_CREATE_D(WizConverter::Enums::ModuleType, ModuleType)
    Q_PRIVATE_CREATE_D(QStackedWidget*, Layer)
    Q_PRIVATE_CREATE_D(PWTableMaskWidget*, TableMask)
    Q_PRIVATE_CREATE_D(PWTableView*, TableView)
    Q_PRIVATE_CREATE_D(PWTableViewModel*, TableViewModel)
    Q_PRIVATE_CREATE_EX(const QString&, QString, FileFilter)
    Q_PRIVATE_CREATE_EX(const QString&, QString, FileBereadyState)

    Q_PRIVATE_CREATE_Q_H(WizConverter::Enums::ModuleType, ModuleType)
public:
    explicit PWCentralWidget(QWidget *parent = nullptr);
    ~PWCentralWidget();

    void setMask(const QPixmap& pixmap);
    void addFiles(const QStringList& filePaths);
    void removeAll();
    void removeAllSelected();
    void removeAllNotSelected();

Q_SIGNALS:
    void switchToTableView(int index);
protected:
    void resizeEvent(QResizeEvent* event) override;
private Q_SLOTS:
    void _openFileExplorer();

};