#pragma once

#include <QDialog>
#include <QStringList>

class IconPickerDialog : public QDialog
{
    Q_OBJECT
    Q_PROPERTY(QString selectedIconKey READ selectedKey WRITE setSelectedKey NOTIFY selectedKeyChanged)
    Q_PROPERTY(QStringList iconKeys READ iconKeys NOTIFY iconKeysChanged)
    Q_PROPERTY(bool canRemove READ canRemove NOTIFY selectedKeyChanged)

public:
    explicit IconPickerDialog(QWidget *parent = nullptr);
    ~IconPickerDialog() override;

    int execWithSelection(QString selection);
    QString selectedIconKey;

    QString selectedKey() const { return selectedIconKey; }
    void setSelectedKey(const QString &key);

    QStringList iconKeys() const;
    bool canRemove() const;

    Q_INVOKABLE void addNewIcon();
    Q_INVOKABLE void removeSelectedIcon();
    Q_INVOKABLE void openFolder();
    Q_INVOKABLE void chooseIcon(const QString &key);
    Q_INVOKABLE void acceptDialog();
    Q_INVOKABLE void rejectDialog();

signals:
    void selectedKeyChanged();
    void iconKeysChanged();
};
