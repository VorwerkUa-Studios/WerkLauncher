#pragma once

#include <QDialog>
#include <QStringList>
#include "BaseInstance.h"

class CopyInstanceDialog : public QDialog {
    Q_OBJECT
    Q_PROPERTY(QString instName READ instName WRITE setInstName NOTIFY instNameChanged)
    Q_PROPERTY(QString instGroup READ instGroup WRITE setInstGroup NOTIFY instGroupChanged)
    Q_PROPERTY(QString iconKey READ iconKey WRITE setIconKey NOTIFY iconKeyChanged)
    Q_PROPERTY(QStringList availableGroups READ availableGroups CONSTANT)
    Q_PROPERTY(bool shouldCopySaves READ shouldCopySaves WRITE setShouldCopySaves NOTIFY shouldCopySavesChanged)
    Q_PROPERTY(bool shouldKeepPlaytime READ shouldKeepPlaytime WRITE setShouldKeepPlaytime NOTIFY shouldKeepPlaytimeChanged)
    Q_PROPERTY(bool canAccept READ canAccept NOTIFY canAcceptChanged)

public:
    explicit CopyInstanceDialog(InstancePtr original, QWidget *parent = nullptr);
    ~CopyInstanceDialog() override;

    QString instName() const { return m_instName; }
    void setInstName(const QString &value);

    QString instGroup() const { return m_instGroup; }
    void setInstGroup(const QString &value);

    QString iconKey() const { return m_iconKey; }
    void setIconKey(const QString &value);

    QStringList availableGroups() const;

    bool shouldCopySaves() const { return m_copySaves; }
    void setShouldCopySaves(bool value);

    bool shouldKeepPlaytime() const { return m_keepPlaytime; }
    void setShouldKeepPlaytime(bool value);

    bool canAccept() const { return !m_instName.trimmed().isEmpty(); }

    Q_INVOKABLE void pickIcon();
    Q_INVOKABLE void acceptDialog();
    Q_INVOKABLE void rejectDialog();

signals:
    void instNameChanged();
    void instGroupChanged();
    void iconKeyChanged();
    void shouldCopySavesChanged();
    void shouldKeepPlaytimeChanged();
    void canAcceptChanged();

private:
    InstancePtr m_original;
    QString m_instName;
    QString m_instGroup;
    QString m_iconKey;
    bool m_copySaves = true;
    bool m_keepPlaytime = true;
};
