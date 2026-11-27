#pragma once

#include <QDialog>
#include <QVariantList>
#include "QObjectPtr.h"
#include "minecraft/auth/AccountList.h"

class ProfileSelectDialog : public QDialog
{
    Q_OBJECT
    Q_PROPERTY(QString message READ message CONSTANT)
    Q_PROPERTY(QVariantList accountsList READ accountsList CONSTANT)
    Q_PROPERTY(int selectedIndex READ selectedIndex WRITE setSelectedIndex NOTIFY selectedIndexChanged)
    Q_PROPERTY(bool showGlobalDefault READ showGlobalDefault CONSTANT)
    Q_PROPERTY(bool showInstanceDefault READ showInstanceDefault CONSTANT)
    Q_PROPERTY(bool useAsGlobalDefault READ useAsGlobalDefault WRITE setUseAsGlobalDefault NOTIFY useAsGlobalDefaultChanged)
    Q_PROPERTY(bool useAsInstDefault READ useAsInstDefault WRITE setUseAsInstDefault NOTIFY useAsInstDefaultChanged)
    Q_PROPERTY(bool canAccept READ canAccept NOTIFY selectedIndexChanged)

public:
    enum Flags
    {
        NoFlags = 0,
        GlobalDefaultCheckbox = 1,
        InstanceDefaultCheckbox = 2,
    };

    explicit ProfileSelectDialog(const QString &message = "", int flags = 0, QWidget *parent = nullptr);
    ~ProfileSelectDialog() override;

    MinecraftAccountPtr selectedAccount() const;

    QString message() const { return m_message; }
    QVariantList accountsList() const;
    int selectedIndex() const { return m_selectedIndex; }
    void setSelectedIndex(int index);

    bool showGlobalDefault() const { return m_flags & GlobalDefaultCheckbox; }
    bool showInstanceDefault() const { return m_flags & InstanceDefaultCheckbox; }

    bool useAsGlobalDefault() const { return m_useAsGlobalDefault; }
    void setUseAsGlobalDefault(bool value);

    bool useAsInstDefault() const { return m_useAsInstDefault; }
    void setUseAsInstDefault(bool value);

    bool canAccept() const { return m_selectedIndex >= 0 && m_selectedIndex < m_accountsList.size(); }

    Q_INVOKABLE void acceptDialog();
    Q_INVOKABLE void rejectDialog();

signals:
    void selectedIndexChanged();
    void useAsGlobalDefaultChanged();
    void useAsInstDefaultChanged();

private:
    QString m_message;
    int m_flags = 0;
    shared_qobject_ptr<AccountList> m_accounts;
    QList<MinecraftAccountPtr> m_accountsList;
    int m_selectedIndex = 0;
    bool m_useAsGlobalDefault = false;
    bool m_useAsInstDefault = false;
    MinecraftAccountPtr m_selected;
};
