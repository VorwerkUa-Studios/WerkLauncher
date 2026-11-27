#include "MinecraftPage.h"

#include <QLabel>
#include <QQuickWidget>
#include <QStackedLayout>
#include <QUrl>
#include <QVariant>

#include "Application.h"
#include "settings/SettingsObject.h"

MinecraftPage::MinecraftPage(QWidget *parent)
    : QWidget(parent)
{
    loadSettings();

    auto *stack = new QStackedLayout(this);
    stack->setContentsMargins(0, 0, 0, 0);

    auto *qmlWidget = new QQuickWidget(this);
    qmlWidget->setObjectName(QStringLiteral("minecraftPageQml"));
    qmlWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    qmlWidget->setInitialProperties(
        {{QStringLiteral("pageController"),
          QVariant::fromValue(static_cast<QObject *>(this))}});
    stack->addWidget(qmlWidget);
    stack->setCurrentWidget(qmlWidget);

    auto *errorLabel =
        new QLabel(tr("The Minecraft settings page could not be loaded."), this);
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
        qCritical() << "Minecraft settings QML failed to load:"
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
    qmlWidget->setSource(QUrl(QStringLiteral("qrc:/qml/MinecraftPage.qml")));
    if (qmlWidget->status() == QQuickWidget::Error ||
        (qmlWidget->status() == QQuickWidget::Ready && !qmlWidget->rootObject())) {
        showLoadError();
    }
}

MinecraftPage::~MinecraftPage() = default;

bool MinecraftPage::apply()
{
    applySettings();
    return true;
}

void MinecraftPage::loadSettings()
{
    auto settings = APPLICATION->settings();
    m_launchMaximized = settings->get("LaunchMaximized").toBool();
    m_windowWidth = qBound(1, settings->get("MinecraftWinWidth").toInt(), 65536);
    m_windowHeight = qBound(1, settings->get("MinecraftWinHeight").toInt(), 65536);
    m_useNativeOpenAL = settings->get("UseNativeOpenAL").toBool();
    m_useNativeGLFW = settings->get("UseNativeGLFW").toBool();
    m_showGameTime = settings->get("ShowGameTime").toBool();
    m_showGlobalGameTime = settings->get("ShowGlobalGameTime").toBool();
    m_recordGameTime = settings->get("RecordGameTime").toBool();
    m_showGameTimeHours = settings->get("ShowGameTimeHours").toBool();
}

void MinecraftPage::applySettings()
{
    auto settings = APPLICATION->settings();
    settings->set("LaunchMaximized", m_launchMaximized);
    settings->set("MinecraftWinWidth", m_windowWidth);
    settings->set("MinecraftWinHeight", m_windowHeight);
    settings->set("UseNativeOpenAL", m_useNativeOpenAL);
    settings->set("UseNativeGLFW", m_useNativeGLFW);
    settings->set("ShowGameTime", m_showGameTime);
    settings->set("ShowGlobalGameTime", m_showGlobalGameTime);
    settings->set("RecordGameTime", m_recordGameTime);
    settings->set("ShowGameTimeHours", m_showGameTimeHours);
}

void MinecraftPage::setLaunchMaximized(bool value)
{
    if (m_launchMaximized != value) {
        m_launchMaximized = value;
        emit launchMaximizedChanged();
    }
}
void MinecraftPage::setWindowWidth(int value)
{
    value = qBound(1, value, 65536);
    if (m_windowWidth != value) {
        m_windowWidth = value;
        emit windowSizeChanged();
    }
}
void MinecraftPage::setWindowHeight(int value)
{
    value = qBound(1, value, 65536);
    if (m_windowHeight != value) {
        m_windowHeight = value;
        emit windowSizeChanged();
    }
}
void MinecraftPage::setUseNativeOpenAL(bool value)
{
    if (m_useNativeOpenAL != value) {
        m_useNativeOpenAL = value;
        emit settingsChanged();
    }
}
void MinecraftPage::setUseNativeGLFW(bool value)
{
    if (m_useNativeGLFW != value) {
        m_useNativeGLFW = value;
        emit settingsChanged();
    }
}
void MinecraftPage::setShowGameTime(bool value)
{
    if (m_showGameTime != value) {
        m_showGameTime = value;
        emit settingsChanged();
    }
}
void MinecraftPage::setShowGlobalGameTime(bool value)
{
    if (m_showGlobalGameTime != value) {
        m_showGlobalGameTime = value;
        emit settingsChanged();
    }
}
void MinecraftPage::setRecordGameTime(bool value)
{
    if (m_recordGameTime != value) {
        m_recordGameTime = value;
        emit settingsChanged();
    }
}
void MinecraftPage::setShowGameTimeHours(bool value)
{
    if (m_showGameTimeHours != value) {
        m_showGameTimeHours = value;
        emit settingsChanged();
    }
}
