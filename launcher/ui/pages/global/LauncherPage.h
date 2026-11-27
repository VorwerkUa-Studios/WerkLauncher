#pragma once

#include <QWidget>
#include <QVariantList>
#include <QTimer>
#include "ui/pages/BasePage.h"
#include <Application.h>

class LauncherPage : public QWidget, public BasePage {
    Q_OBJECT

    // Updates & System
    Q_PROPERTY(bool autoUpdate READ autoUpdate WRITE setAutoUpdate NOTIFY autoUpdateChanged)
    Q_PROPERTY(bool updaterEnabled READ updaterEnabled CONSTANT)
    Q_PROPERTY(bool minimizeToTray READ minimizeToTray WRITE setMinimizeToTray NOTIFY minimizeToTrayChanged)
    Q_PROPERTY(QString instSortMode READ instSortMode WRITE setInstSortMode NOTIFY instSortModeChanged)

    // Folders
    Q_PROPERTY(QString instanceDir READ instanceDir WRITE setInstanceDir NOTIFY instanceDirChanged)
    Q_PROPERTY(QString iconsDir READ iconsDir WRITE setIconsDir NOTIFY iconsDirChanged)
    Q_PROPERTY(QString skinsDir READ skinsDir WRITE setSkinsDir NOTIFY skinsDirChanged)

    // Appearance
    Q_PROPERTY(QString iconTheme READ iconTheme WRITE setIconTheme NOTIFY iconThemeChanged)
    Q_PROPERTY(QString applicationTheme READ applicationTheme WRITE setApplicationTheme NOTIFY applicationThemeChanged)
    Q_PROPERTY(QVariantList availableThemes READ availableThemes NOTIFY availableThemesChanged)
    Q_PROPERTY(QString customAccentColor READ customAccentColor NOTIFY customAccentColorChanged)
    Q_PROPERTY(QString customBackgroundImage READ customBackgroundImage NOTIFY customBackgroundImageChanged)
    Q_PROPERTY(int customBackgroundOpacity READ customBackgroundOpacity WRITE setCustomBackgroundOpacity NOTIFY customBackgroundOpacityChanged)
    Q_PROPERTY(bool customBackgroundBlur READ customBackgroundBlur WRITE setCustomBackgroundBlur NOTIFY customBackgroundBlurChanged)

    // Console
    Q_PROPERTY(bool showConsole READ showConsole WRITE setShowConsole NOTIFY showConsoleChanged)
    Q_PROPERTY(bool autoCloseConsole READ autoCloseConsole WRITE setAutoCloseConsole NOTIFY autoCloseConsoleChanged)
    Q_PROPERTY(bool showConsoleOnError READ showConsoleOnError WRITE setShowConsoleOnError NOTIFY showConsoleOnErrorChanged)
    Q_PROPERTY(int consoleFontSize READ consoleFontSize WRITE setConsoleFontSize NOTIFY consoleFontSizeChanged)
    Q_PROPERTY(int consoleMaxLines READ consoleMaxLines WRITE setConsoleMaxLines NOTIFY consoleMaxLinesChanged)
    Q_PROPERTY(bool consoleOverflowStop READ consoleOverflowStop WRITE setConsoleOverflowStop NOTIFY consoleOverflowStopChanged)

public:
    explicit LauncherPage(QWidget *parent = nullptr);
    ~LauncherPage() override;

    QString displayName() const override { return tr("Launcher"); }
    QIcon icon() const override { return APPLICATION->getThemedIcon("launcher"); }
    QString id() const override { return "launcher-settings"; }
    QString helpPage() const override { return "Launcher-settings"; }
    bool apply() override;

    bool autoUpdate() const { return m_autoUpdate; }
    void setAutoUpdate(bool value);

    bool updaterEnabled() const;

    bool minimizeToTray() const { return m_minimizeToTray; }
    void setMinimizeToTray(bool value);

    QString instSortMode() const { return m_instSortMode; }
    void setInstSortMode(const QString &value);

    QString instanceDir() const { return m_instanceDir; }
    void setInstanceDir(const QString &value);

    QString iconsDir() const { return m_iconsDir; }
    void setIconsDir(const QString &value);

    QString skinsDir() const { return m_skinsDir; }
    void setSkinsDir(const QString &value);

    QString iconTheme() const { return m_iconTheme; }
    void setIconTheme(const QString &value);

    QString applicationTheme() const { return m_applicationTheme; }
    void setApplicationTheme(const QString &value);

    QVariantList availableThemes() const;

    QString customAccentColor() const;
    QString customBackgroundImage() const { return m_customBackgroundImage; }
    int customBackgroundOpacity() const { return m_customBackgroundOpacity; }
    void setCustomBackgroundOpacity(int value);
    bool customBackgroundBlur() const { return m_customBackgroundBlur; }
    void setCustomBackgroundBlur(bool value);

    bool showConsole() const { return m_showConsole; }
    void setShowConsole(bool value);

    bool autoCloseConsole() const { return m_autoCloseConsole; }
    void setAutoCloseConsole(bool value);

    bool showConsoleOnError() const { return m_showConsoleOnError; }
    void setShowConsoleOnError(bool value);

    int consoleFontSize() const { return m_consoleFontSize; }
    void setConsoleFontSize(int value);

    int consoleMaxLines() const { return m_consoleMaxLines; }
    void setConsoleMaxLines(int value);

    bool consoleOverflowStop() const { return m_consoleOverflowStop; }
    void setConsoleOverflowStop(bool value);

    Q_INVOKABLE void browseInstanceDir();
    Q_INVOKABLE void openInstanceDir();
    Q_INVOKABLE void browseIconsDir();
    Q_INVOKABLE void openIconsDir();
    Q_INVOKABLE void browseSkinsDir();
    Q_INVOKABLE void openSkinsDir();

    Q_INVOKABLE void pickAccentColor();
    Q_INVOKABLE void chooseBackgroundImage();
    Q_INVOKABLE void clearBackgroundImage();
    Q_INVOKABLE void resetNotifications();

signals:
    void autoUpdateChanged();
    void minimizeToTrayChanged();
    void instSortModeChanged();
    void instanceDirChanged();
    void iconsDirChanged();
    void skinsDirChanged();
    void iconThemeChanged();
    void applicationThemeChanged();
    void availableThemesChanged();
    void customAccentColorChanged();
    void customBackgroundImageChanged();
    void customBackgroundOpacityChanged();
    void customBackgroundBlurChanged();
    void showConsoleChanged();
    void autoCloseConsoleChanged();
    void showConsoleOnErrorChanged();
    void consoleFontSizeChanged();
    void consoleMaxLinesChanged();
    void consoleOverflowStopChanged();

private slots:
    void applyPendingTheme();

private:
    void loadSettings();
    void applySettings();

private:
    bool m_autoUpdate = false;
    bool m_minimizeToTray = false;
    QString m_instSortMode;
    QString m_instanceDir;
    QString m_iconsDir;
    QString m_skinsDir;
    QString m_iconTheme;
    QString m_applicationTheme;
    QString m_customBackgroundImage;
    int m_customBackgroundOpacity = 100;
    bool m_customBackgroundBlur = false;
    bool m_resetNotifications = false;

    bool m_showConsole = false;
    bool m_autoCloseConsole = false;
    bool m_showConsoleOnError = false;
    int m_consoleFontSize = 11;
    int m_consoleMaxLines = 10000;
    bool m_consoleOverflowStop = true;

    QTimer m_themeTimer;
    QString m_pendingTheme;
};
