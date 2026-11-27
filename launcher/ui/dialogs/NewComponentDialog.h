#pragma once

#include <QDialog>
#include <QString>
#include <QStringList>

class NewComponentDialog : public QDialog {
    Q_OBJECT
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(QString uid READ uid WRITE setUid NOTIFY uidChanged)
    Q_PROPERTY(QString placeholderUid READ placeholderUid NOTIFY placeholderUidChanged)
    Q_PROPERTY(bool canAccept READ canAccept NOTIFY canAcceptChanged)

public:
    explicit NewComponentDialog(const QString &initialName = QString(),
                                const QString &initialUid = QString(),
                                QWidget *parent = nullptr);
    virtual ~NewComponentDialog();
    void setBlacklist(QStringList badUids);

    QString name() const;
    void setName(const QString &name);

    QString uid() const;
    void setUid(const QString &uid);

    QString placeholderUid() const { return m_placeholderUid; }
    bool canAccept() const;

    Q_INVOKABLE void acceptDialog();
    Q_INVOKABLE void rejectDialog();

signals:
    void nameChanged();
    void uidChanged();
    void placeholderUidChanged();
    void canAcceptChanged();

private:
    void updateDialogState();

    QString m_name;
    QString m_uid;
    QString m_placeholderUid;
    QString m_originalPlaceholderText;
    QStringList m_uidBlacklist;
};
