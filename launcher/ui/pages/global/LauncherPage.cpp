#include "LauncherPage.h"

#include <QColorDialog>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QLabel>
#include <QMessageBox>
#include <QPointer>
#include <QQuickWidget>
#include <QStackedLayout>
#include <QUrl>
#include <QVBoxLayout>

#include "Application.h"
#include "BuildConfig.h"
#include "DesktopServices.h"
#include "FileSystem.h"
#include "settings/SettingsObject.h"
#include "ui/themes/ITheme.h"
#include "ui/widgets/CustomTitleBar.h"

LauncherPage::LauncherPage(QWidget *parent)
    : QWidget(parent)
{
    loadSettings();

    m_themeTimer.setSingleShot(true);
    connect(&m_themeTimer, &QTimer::timeout, this, &LauncherPage::applyPendingTheme);

    auto *stack = new QStackedLayout(this);
    stack->setContentsMargins(0, 0, 0, 0);

    auto *qmlWidget = new QQuickWidget(this);
    qmlWidget->setObjectName(QStringLiteral("launcherPageQml"));
    qmlWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    qmlWidget->setInitialProperties(
        {{QStringLiteral("pageController"),
          QVariant::fromValue(static_cast<QObject *>(this))}});
    stack->addWidget(qmlWidget);
    stack->setCurrentWidget(qmlWidget);

    auto *errorLabel =
        new QLabel(tr("The launcher settings page could not be loaded."), this);
    errorLabel->setAlignment(Qt::AlignCenter);
    errorLabel->setWordWrap(true);
    stack->addWidget(errorLabel);

    const auto showLoadError = [qmlWidget, stack, errorLabel]() {
        QStringList details;
        for (const auto &error : qmlWidget->errors()) {
            details.append(error.toString());
        }
        if (details.isEmpty()) {
            details.append(QObject::tr("No QML diagnostic was provided."));
        }
        qCritical() << "Launcher settings QML failed to load:"
                    << details.join('\n');
        errorLabel->setToolTip(details.join('\n'));
        stack->setCurrentWidget(errorLabel);
    };
    connect(qmlWidget, &QQuickWidget::statusChanged, this,
            [qmlWidget, showLoadError](QQuickWidget::Status status) {
                if (status == QQuickWidget::Error ||
                    (status == QQuickWidget::Ready && !qmlWidget->rootObject())) {
                    showLoadError();
                }
            });
    qmlWidget->setSource(QUrl(QStringLiteral("qrc:/qml/LauncherPage.qml")));
    if (qmlWidget->status() == QQuickWidget::Error ||
        (qmlWidget->status() == QQuickWidget::Ready && !qmlWidget->rootObject())) {
        showLoadError();
    }
}

LauncherPage::~LauncherPage() = default;

bool LauncherPage::updaterEnabled() const {
    return BuildConfig.UPDATER_ENABLED;
}

bool LauncherPage::apply() {
    applySettings();
    return true;
}

void LauncherPage::loadSettings() {
    auto s = APPLICATION->settings();

    m_autoUpdate = s->get("AutoUpdate").toBool();
    m_minimizeToTray = s->get("MinimizeToTray").toBool();

    m_iconTheme = s->get("IconTheme").toString();
    if (m_iconTheme.isEmpty()) {
        m_iconTheme = "flat";
    }

    m_applicationTheme = s->get("ApplicationTheme").toString();
    if (m_applicationTheme.isEmpty()) {
        m_applicationTheme = APPLICATION->currentTheme();
    }

    m_instanceDir = s->get("InstanceDir").toString();
    m_iconsDir = s->get("IconsDir").toString();
    m_skinsDir = s->get("SkinsDir").toString();

    m_instSortMode = s->get("InstSortMode").toString();
    if (m_instSortMode.isEmpty()) {
        m_instSortMode = "Name";
    }

    m_customBackgroundImage = s->get("CustomBackgroundImage").toString();
    m_customBackgroundOpacity = qRound(s->get("CustomBackgroundOpacity").toDouble() * 100.0);
    if (m_customBackgroundOpacity <= 0) {
        m_customBackgroundOpacity = 100;
    }
    m_customBackgroundBlur = s->get("CustomBackgroundBlur").toBool();

    m_showConsole = s->get("ShowConsole").toBool();
    m_autoCloseConsole = s->get("AutoCloseConsole").toBool();
    m_showConsoleOnError = s->get("ShowConsoleOnError").toBool();
    m_consoleFontSize = s->get("ConsoleFontSize").toInt();
    if (m_consoleFontSize <= 0) {
        m_consoleFontSize = 11;
    }
    m_consoleMaxLines = s->get("ConsoleMaxLines").toInt();
    if (m_consoleMaxLines <= 0) {
        m_consoleMaxLines = 10000;
    }
    m_consoleOverflowStop = s->get("ConsoleOverflowStop").toBool();
}

void LauncherPage::applySettings() {
    auto s = APPLICATION->settings();

    if (m_resetNotifications) {
        s->set("ShownNotifications", QString());
        m_resetNotifications = false;
    }

    s->set("AutoUpdate", m_autoUpdate);
    s->set("MinimizeToTray", m_minimizeToTray);

    auto origIconTheme = s->get("IconTheme").toString();
    if (origIconTheme != m_iconTheme) {
        s->set("IconTheme", m_iconTheme);
        APPLICATION->setIconTheme(m_iconTheme);
    }

    auto origAppTheme = s->get("ApplicationTheme").toString();
    if (origAppTheme != m_applicationTheme) {
        s->set("ApplicationTheme", m_applicationTheme);
        if (APPLICATION->currentTheme() != m_applicationTheme) {
            APPLICATION->setApplicationTheme(m_applicationTheme, false);
        }
    }

    s->set("InstanceDir", m_instanceDir);
    s->set("IconsDir", m_iconsDir);
    s->set("SkinsDir", m_skinsDir);
    s->set("InstSortMode", m_instSortMode);

    s->set("CustomBackgroundImage", m_customBackgroundImage);
    s->set("CustomBackgroundOpacity", m_customBackgroundOpacity / 100.0);
    s->set("CustomBackgroundBlur", m_customBackgroundBlur);

    s->set("ShowConsole", m_showConsole);
    s->set("AutoCloseConsole", m_autoCloseConsole);
    s->set("ShowConsoleOnError", m_showConsoleOnError);
    s->set("ConsoleFontSize", m_consoleFontSize);
    s->set("ConsoleMaxLines", m_consoleMaxLines);
    s->set("ConsoleOverflowStop", m_consoleOverflowStop);
}

void LauncherPage::setAutoUpdate(bool value) {
    if (m_autoUpdate != value) {
        m_autoUpdate = value;
        emit autoUpdateChanged();
    }
}

void LauncherPage::setMinimizeToTray(bool value) {
    if (m_minimizeToTray != value) {
        m_minimizeToTray = value;
        emit minimizeToTrayChanged();
    }
}

void LauncherPage::setInstSortMode(const QString &value) {
    if (m_instSortMode != value) {
        m_instSortMode = value;
        emit instSortModeChanged();
    }
}

void LauncherPage::setInstanceDir(const QString &value) {
    if (m_instanceDir != value) {
        m_instanceDir = value;
        emit instanceDirChanged();
    }
}

void LauncherPage::setIconsDir(const QString &value) {
    if (m_iconsDir != value) {
        m_iconsDir = value;
        emit iconsDirChanged();
    }
}

void LauncherPage::setSkinsDir(const QString &value) {
    if (m_skinsDir != value) {
        m_skinsDir = value;
        emit skinsDirChanged();
    }
}

void LauncherPage::setIconTheme(const QString &value) {
    if (m_iconTheme != value) {
        m_iconTheme = value;
        APPLICATION->setIconTheme(value);
        emit iconThemeChanged();
    }
}

void LauncherPage::setApplicationTheme(const QString &value) {
    if (m_applicationTheme != value) {
        m_applicationTheme = value;
        m_pendingTheme = value;
        m_themeTimer.start(200);
        emit applicationThemeChanged();
    }
}

void LauncherPage::applyPendingTheme() {
    if (!m_pendingTheme.isEmpty()) {
        APPLICATION->setApplicationTheme(m_pendingTheme, false);
    }
}

QVariantList LauncherPage::availableThemes() const {
    QVariantList list;
    auto themes = APPLICATION->getValidApplicationThemes();
    for (const auto &theme : themes) {
        QVariantMap map;
        map[QStringLiteral("id")] = theme->id();
        map[QStringLiteral("name")] = theme->name();
        list.append(map);
    }
    return list;
}

QString LauncherPage::customAccentColor() const {
    return APPLICATION->settings()->get("CustomAccentColor").toString();
}

void LauncherPage::setCustomBackgroundOpacity(int value) {
    value = qBound(0, value, 100);
    if (m_customBackgroundOpacity != value) {
        m_customBackgroundOpacity = value;
        APPLICATION->settings()->set("CustomBackgroundOpacity", value / 100.0);
        emit customBackgroundOpacityChanged();
    }
}

void LauncherPage::setCustomBackgroundBlur(bool value) {
    if (m_customBackgroundBlur != value) {
        m_customBackgroundBlur = value;
        APPLICATION->settings()->set("CustomBackgroundBlur", value);
        emit customBackgroundBlurChanged();
    }
}

void LauncherPage::setShowConsole(bool value) {
    if (m_showConsole != value) {
        m_showConsole = value;
        emit showConsoleChanged();
    }
}

void LauncherPage::setAutoCloseConsole(bool value) {
    if (m_autoCloseConsole != value) {
        m_autoCloseConsole = value;
        emit autoCloseConsoleChanged();
    }
}

void LauncherPage::setShowConsoleOnError(bool value) {
    if (m_showConsoleOnError != value) {
        m_showConsoleOnError = value;
        emit showConsoleOnErrorChanged();
    }
}

void LauncherPage::setConsoleFontSize(int value) {
    value = qBound(6, value, 36);
    if (m_consoleFontSize != value) {
        m_consoleFontSize = value;
        emit consoleFontSizeChanged();
    }
}

void LauncherPage::setConsoleMaxLines(int value) {
    value = qBound(100, value, 1000000);
    if (m_consoleMaxLines != value) {
        m_consoleMaxLines = value;
        emit consoleMaxLinesChanged();
    }
}

void LauncherPage::setConsoleOverflowStop(bool value) {
    if (m_consoleOverflowStop != value) {
        m_consoleOverflowStop = value;
        emit consoleOverflowStopChanged();
    }
}

void LauncherPage::browseInstanceDir() {
    QString raw_dir = QFileDialog::getExistingDirectory(
        this, tr("Instance Folder"), m_instanceDir);

    if (!raw_dir.isEmpty() && QDir(raw_dir).exists()) {
        QString cooked_dir = FS::NormalizePath(raw_dir);
        if (FS::checkProblemticPathJava(QDir(cooked_dir))) {
            auto result = QMessageBox::warning(
                this, tr("Warning"),
                tr("You're trying to specify an instance folder which's path contains at least one '!'. "
                   "Java is known to cause problems if that is the case, your instances (probably) won't start!\n\n"
                   "Do you really want to use this path?"),
                QMessageBox::Yes | QMessageBox::No);
            if (result == QMessageBox::Yes) {
                setInstanceDir(cooked_dir);
            }
        } else {
            setInstanceDir(cooked_dir);
        }
    }
}

void LauncherPage::openInstanceDir() {
    DesktopServices::openDirectory(m_instanceDir);
}

void LauncherPage::browseIconsDir() {
    QString raw_dir = QFileDialog::getExistingDirectory(
        this, tr("Icons Folder"), m_iconsDir);
    if (!raw_dir.isEmpty() && QDir(raw_dir).exists()) {
        setIconsDir(FS::NormalizePath(raw_dir));
    }
}

void LauncherPage::openIconsDir() {
    DesktopServices::openDirectory(m_iconsDir);
}

void LauncherPage::browseSkinsDir() {
    QString raw_dir = QFileDialog::getExistingDirectory(
        this, tr("Skins Folder"), m_skinsDir);
    if (!raw_dir.isEmpty() && QDir(raw_dir).exists()) {
        setSkinsDir(FS::NormalizePath(raw_dir));
    }
}

void LauncherPage::openSkinsDir() {
    DesktopServices::openDirectory(m_skinsDir);
}

void LauncherPage::pickAccentColor() {
    auto s = APPLICATION->settings();
    QString currentColorStr = s->get("CustomAccentColor").toString();
    QColor currentColor(currentColorStr);

    QColorDialog colorDlg(currentColor, this);
    colorDlg.setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    colorDlg.setOptions(QColorDialog::DontUseNativeDialog);

    auto titleBar = new CustomTitleBar(&colorDlg);
    if (auto layout = qobject_cast<QVBoxLayout*>(colorDlg.layout())) {
        layout->insertWidget(0, titleBar);
        layout->setContentsMargins(0, 0, 0, 0);
    }

    if (colorDlg.exec() != QDialog::Accepted) {
        return;
    }

    QColor newColor = colorDlg.currentColor();
    if (!newColor.isValid() || newColor.name() == currentColor.name())
        return;

    s->set("CustomAccentColor", newColor.name());
    emit customAccentColorChanged();

    QPointer<LauncherPage> safeThis(this);
    QTimer::singleShot(100, this, [safeThis]() {
        if (!safeThis)
            return;
        APPLICATION->setApplicationTheme(safeThis->m_applicationTheme, false);
    });
}

void LauncherPage::chooseBackgroundImage() {
    QString path = QFileDialog::getOpenFileName(
        this, tr("Select background image"), QString(),
        tr("Images (*.png *.jpg *.jpeg *.bmp *.webp)"));
    if (path.isEmpty())
        return;

    auto s = APPLICATION->settings();
    s->set("CustomBackgroundImage", path);
    m_customBackgroundImage = path;
    emit customBackgroundImageChanged();

    APPLICATION->setApplicationTheme(m_applicationTheme, false);
}

void LauncherPage::clearBackgroundImage() {
    auto s = APPLICATION->settings();
    s->set("CustomBackgroundImage", "");
    m_customBackgroundImage = "";
    emit customBackgroundImageChanged();

    APPLICATION->setApplicationTheme(m_applicationTheme, false);
}

void LauncherPage::resetNotifications() {
    m_resetNotifications = true;
    QMessageBox::information(
        this, tr("Notifications"),
        tr("Hidden notifications have been reset."));
}
