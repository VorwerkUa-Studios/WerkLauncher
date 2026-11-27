#include "InstanceJavaPage.h"

#include <QFileDialog>
#include <QLabel>
#include <QQuickWidget>
#include <QStackedLayout>
#include <QUrl>
#include <QVariant>

#include "Application.h"
#include "FileSystem.h"
#include "JavaCommon.h"
#include "java/JavaInstall.h"
#include "java/JavaInstallList.h"
#include "sys.h"
#include "ui/dialogs/VersionSelectDialog.h"

InstanceJavaPage::InstanceJavaPage(BaseInstance *inst, QWidget *parent)
    : QWidget(parent), m_instance(inst), m_settings(inst->settings())
{
    m_overrideMemory = m_settings->get("OverrideMemory").toBool();
    m_overrideJavaLocation =
        m_settings->get("OverrideJavaLocation").toBool() ||
        m_settings->get("OverrideJava").toBool();
    m_overrideJvmArgs =
        m_settings->get("OverrideJavaArgs").toBool() ||
        m_settings->get("OverrideJava").toBool();

    int min = m_settings->get("MinMemAlloc").toInt();
    int max = m_settings->get("MaxMemAlloc").toInt();
    m_permGen = m_settings->get("PermGen").toInt();
    m_minMemory = qBound(128, qMin(min, max), 65536);
    m_maxMemory = qBound(128, qMax(min, max), 65536);
    m_javaPath = m_settings->get("JavaPath").toString();
    m_jvmArgs = m_settings->get("JvmArgs").toString();

    const auto sysMiB = Sys::getSystemRam() / Sys::mebibyte;
    m_memoryMaximum = sysMiB > 0 ? static_cast<int>(sysMiB) : 65536;

    auto *stack = new QStackedLayout(this);
    stack->setContentsMargins(0, 0, 0, 0);

    auto *qmlWidget = new QQuickWidget(this);
    qmlWidget->setObjectName(QStringLiteral("instanceJavaPageQml"));
    qmlWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    qmlWidget->setInitialProperties(
        {{QStringLiteral("pageController"),
          QVariant::fromValue(static_cast<QObject *>(this))}});
    stack->addWidget(qmlWidget);
    stack->setCurrentWidget(qmlWidget);

    auto *errorLabel =
        new QLabel(tr("The instance Java settings page could not be loaded."),
                   this);
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
        qCritical() << "Instance Java settings QML failed to load:"
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
    qmlWidget->setSource(QUrl(QStringLiteral("qrc:/qml/InstanceJavaPage.qml")));
    if (qmlWidget->status() == QQuickWidget::Error ||
        (qmlWidget->status() == QQuickWidget::Ready && !qmlWidget->rootObject())) {
        showLoadError();
    }
}

InstanceJavaPage::~InstanceJavaPage() = default;

QIcon InstanceJavaPage::icon() const
{
    return APPLICATION->getThemedIcon("java");
}

bool InstanceJavaPage::apply()
{
    SettingsObject::Lock lock(m_settings);
    m_settings->set("OverrideMemory", m_overrideMemory);
    m_settings->set("MinMemAlloc", m_minMemory);
    m_settings->set("MaxMemAlloc", m_maxMemory);
    m_settings->set("PermGen", m_permGen);

    m_settings->set("OverrideJavaLocation", m_overrideJavaLocation);
    m_settings->set("JavaPath", m_javaPath);

    m_settings->set("OverrideJavaArgs", m_overrideJvmArgs);
    m_settings->set("JvmArgs", m_jvmArgs.replace('\n', ' '));
    JavaCommon::checkJVMArgs(m_jvmArgs, this);
    return true;
}

bool InstanceJavaPage::shouldDisplay() const
{
    return !m_instance->isRunning();
}

void InstanceJavaPage::setOverrideMemory(bool value)
{
    if (m_overrideMemory != value) {
        m_overrideMemory = value;
        emit overrideChanged();
    }
}

void InstanceJavaPage::setOverrideJavaLocation(bool value)
{
    if (m_overrideJavaLocation != value) {
        m_overrideJavaLocation = value;
        emit overrideChanged();
    }
}

void InstanceJavaPage::setOverrideJvmArgs(bool value)
{
    if (m_overrideJvmArgs != value) {
        m_overrideJvmArgs = value;
        emit overrideChanged();
    }
}

void InstanceJavaPage::setMinMemory(int value)
{
    value = qBound(128, value, m_memoryMaximum);
    if (m_minMemory != value) {
        m_minMemory = value;
        if (m_minMemory > m_maxMemory) {
            m_maxMemory = m_minMemory;
            emit memoryChanged();
        }
        emit memoryChanged();
    }
}

void InstanceJavaPage::setMaxMemory(int value)
{
    value = qBound(128, value, m_memoryMaximum);
    if (m_maxMemory != value) {
        m_maxMemory = value;
        if (m_maxMemory < m_minMemory) {
            m_minMemory = m_maxMemory;
        }
        emit memoryChanged();
    }
}

void InstanceJavaPage::setPermGen(int value)
{
    value = qBound(64, value, m_memoryMaximum);
    if (m_permGen != value) {
        m_permGen = value;
        emit memoryChanged();
    }
}

void InstanceJavaPage::setJavaPath(const QString &path)
{
    if (m_javaPath != path) {
        m_javaPath = path;
        emit javaPathChanged();
    }
}

void InstanceJavaPage::setJvmArgs(const QString &args)
{
    if (m_jvmArgs != args) {
        m_jvmArgs = args;
        emit jvmArgsChanged();
    }
}

void InstanceJavaPage::browseJava()
{
    const QString rawPath = QFileDialog::getOpenFileName(
        this, tr("Find Java executable"), m_javaPath);
    if (rawPath.isEmpty()) {
        return;
    }
    const QString cookedPath = FS::NormalizePath(rawPath);
    QFileInfo info(cookedPath);
    if (info.exists() && info.isExecutable()) {
        setJavaPath(cookedPath);
    }
}

void InstanceJavaPage::detectJava()
{
    VersionSelectDialog dialog(APPLICATION->javalist().get(),
                               tr("Select a Java version"), this, true);
    dialog.setResizeOn(2);
    if (dialog.exec() == QDialog::Accepted && dialog.selectedVersion()) {
        if (const auto java = std::dynamic_pointer_cast<JavaInstall>(
                dialog.selectedVersion())) {
            setJavaPath(java->path);
        }
    }
}

void InstanceJavaPage::testJava()
{
    if (m_checker) {
        return;
    }
    m_checker.reset(new JavaCommon::TestCheck(
        this, m_javaPath, m_jvmArgs, m_minMemory, m_maxMemory, m_permGen));
    connect(m_checker.get(), &JavaCommon::TestCheck::finished, this,
            [this]() { m_checker.reset(); });
    m_checker->run();
}
