#pragma once

#include <QWidget>
#include <NXProperty.h>
#include "PWDef.h"
class QTableWidget;
class QStackedWidget;
class PWTableMaskWidget;
class PWFileTableWidget : public QWidget
{
    Q_OBJECT
    Q_PRIVATE_CREATE(WizConverter::Module::Enums::MasterModule, MasterModule)
    Q_PRIVATE_CREATE(WizConverter::Module::Enums::SlaveModule, SlaveModule)
    Q_PRIVATE_CREATE_D(QStackedWidget*, Layer)
    Q_PRIVATE_CREATE_D(PWTableMaskWidget*, TableMask)
    Q_PRIVATE_CREATE_D(QTableWidget*, TableView)
    Q_PRIVATE_CREATE_EX(const QString&, QString, FileFilter)
    Q_PRIVATE_CREATE_EX(const QString&, QString, FileState)
public:
    explicit PWFileTableWidget(QWidget *parent = nullptr);
    ~PWFileTableWidget();

    void setMask(const QPixmap& pixmap);
    void addFiles(const QStringList& filePaths);
    void removeAll();
protected:
    void resizeEvent(QResizeEvent* event) override;
private Q_SLOTS:
    void _openFileExplorer();

};