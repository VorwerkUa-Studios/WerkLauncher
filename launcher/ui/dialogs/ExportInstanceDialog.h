#pragma once

#include <QDialog>
#include <QModelIndex>
#include <memory>

class BaseInstance;
class PackIgnoreProxy;
class QTreeView;
class QDialogButtonBox;
typedef std::shared_ptr<BaseInstance> InstancePtr;

class ExportInstanceDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ExportInstanceDialog(InstancePtr instance, QWidget *parent = 0);
    ~ExportInstanceDialog();

    virtual void done(int result);

private:
    bool doExport();
    void loadPackIgnore();
    void savePackIgnore();
    QString ignoreFileName();

private:
    QTreeView *m_treeView = nullptr;
    QDialogButtonBox *m_buttonBox = nullptr;
    InstancePtr m_instance;
    PackIgnoreProxy * proxyModel = nullptr;

private slots:
    void rowsInserted(QModelIndex parent, int top, int bottom);
};
