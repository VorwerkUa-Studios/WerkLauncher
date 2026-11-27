#pragma once

#include "BaseInstance.h"
#include <QDialog>
#include <QStringList>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

class CreateShortcutDialog : public QDialog {
    Q_OBJECT
    Q_PROPERTY(QString shortcutPath READ shortcutPath WRITE setShortcutPath NOTIFY shortcutPathChanged)
    Q_PROPERTY(bool joinWorld READ joinWorld WRITE setJoinWorld NOTIFY joinWorldChanged)
    Q_PROPERTY(bool joinServerMode READ joinServerMode WRITE setJoinServerMode NOTIFY joinServerModeChanged)
    Q_PROPERTY(QString serverAddress READ serverAddress WRITE setServerAddress NOTIFY serverAddressChanged)
    Q_PROPERTY(QString selectedSingleplayerWorld READ selectedSingleplayerWorld WRITE setSelectedSingleplayerWorld NOTIFY selectedSingleplayerWorldChanged)
    Q_PROPERTY(QStringList singleplayerWorlds READ singleplayerWorlds CONSTANT)
    Q_PROPERTY(bool instanceSupportsQuickPlay READ instanceSupportsQuickPlay CONSTANT)

    Q_PROPERTY(bool useProfile READ useProfile WRITE setUseProfile NOTIFY useProfileChanged)
    Q_PROPERTY(QString selectedProfile READ selectedProfile WRITE setSelectedProfile NOTIFY selectedProfileChanged)
    Q_PROPERTY(QStringList availableProfiles READ availableProfiles CONSTANT)

    Q_PROPERTY(bool launchOffline READ launchOffline WRITE setLaunchOffline NOTIFY launchOfflineChanged)
    Q_PROPERTY(bool setOfflineUsername READ setOfflineUsername WRITE setSetOfflineUsername NOTIFY setOfflineUsernameChanged)
    Q_PROPERTY(QString offlineUsername READ offlineUsername WRITE setOfflineUsername NOTIFY offlineUsernameChanged)

    Q_PROPERTY(bool createScript READ createScript WRITE setCreateScript NOTIFY createScriptChanged)
    Q_PROPERTY(bool createScriptEnabled READ createScriptEnabled CONSTANT)
    Q_PROPERTY(bool canCreate READ canCreate NOTIFY canCreateChanged)

public:
    explicit CreateShortcutDialog(QWidget *parent = nullptr, InstancePtr instance = nullptr);
    ~CreateShortcutDialog() override;

    QString shortcutPath() const { return m_shortcutPath; }
    void setShortcutPath(const QString &val);

    bool joinWorld() const { return m_joinWorld; }
    void setJoinWorld(bool val);

    bool joinServerMode() const { return m_joinServerMode; }
    void setJoinServerMode(bool val);

    QString serverAddress() const { return m_serverAddress; }
    void setServerAddress(const QString &val);

    QString selectedSingleplayerWorld() const { return m_selectedSingleplayerWorld; }
    void setSelectedSingleplayerWorld(const QString &val);

    QStringList singleplayerWorlds() const { return m_singleplayerWorlds; }
    bool instanceSupportsQuickPlay() const { return m_instanceSupportsQuickPlay; }

    bool useProfile() const { return m_useProfile; }
    void setUseProfile(bool val);

    QString selectedProfile() const { return m_selectedProfile; }
    void setSelectedProfile(const QString &val);

    QStringList availableProfiles() const { return m_availableProfiles; }

    bool launchOffline() const { return m_launchOffline; }
    void setLaunchOffline(bool val);

    bool setOfflineUsername() const { return m_setOfflineUsername; }
    void setSetOfflineUsername(bool val);

    QString offlineUsername() const { return m_offlineUsername; }
    void setOfflineUsername(const QString &val);

    bool createScript() const { return m_createScript; }
    void setCreateScript(bool val);

    bool createScriptEnabled() const;
    bool canCreate() const;

    Q_INVOKABLE void browseShortcutPath();
    Q_INVOKABLE void createShortcutAction();
    Q_INVOKABLE void cancelAction();

signals:
    void shortcutPathChanged();
    void joinWorldChanged();
    void joinServerModeChanged();
    void serverAddressChanged();
    void selectedSingleplayerWorldChanged();
    void useProfileChanged();
    void selectedProfileChanged();
    void launchOfflineChanged();
    void setOfflineUsernameChanged();
    void offlineUsernameChanged();
    void createScriptChanged();
    void canCreateChanged();

private:
    InstancePtr m_instance;
    QString m_shortcutPath;
    bool m_joinWorld = false;
    bool m_joinServerMode = true;
    QString m_serverAddress;
    QString m_selectedSingleplayerWorld;
    QStringList m_singleplayerWorlds;
    bool m_instanceSupportsQuickPlay = false;

    bool m_useProfile = false;
    QString m_selectedProfile;
    QStringList m_availableProfiles;

    bool m_launchOffline = false;
    bool m_setOfflineUsername = false;
    QString m_offlineUsername;

    bool m_createScript = false;

    QString getLaunchCommand(bool escapeQuotesTwice = false);
    QString getLaunchArgs(bool escapeQuotesTwice = false);
    void createShortcut();

#ifdef Q_OS_WIN
    void createWindowsLink(LPCWSTR target, LPCWSTR workingDir, LPCWSTR args,
                           LPCWSTR filename, LPCWSTR desc, LPCWSTR iconPath);
#endif
};
