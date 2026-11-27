#include "JavaPage.h"

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

JavaPage::JavaPage(QWidget *parent)
    : QWidget(parent)
{
    const auto sysMiB = Sys::getSystemRam() / Sys::mebibyte;
    m_memoryMaximum = sysMiB > 0 ? static_cast<int>(sysMiB) : 65536;
    loadSettings();

    auto *stack = new QStackedLayout(this);
    stack->setContentsMargins(0, 0, 0, 0);

    auto *qmlWidget = new QQuickWidget(this);
    qmlWidget->setObjectName(QStringLiteral("javaPageQml"));
    qmlWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    qmlWidget->setInitialProperties(
        {{QStringLiteral("pageController"),
          QVariant::fromValue(static_cast<QObject *>(this))}});
    stack->addWidget(qmlWidget);
    stack->setCurrentWidget(qmlWidget);

    auto *errorLabel =
        new QLabel(tr("The Java settings page could not be loaded."), this);
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
        qCritical() << "Java settings QML failed to load:" << details.join('\n');
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
    qmlWidget->setSource(QUrl(QStringLiteral("qrc:/qml/JavaPage.qml")));
    if (qmlWidget->status() == QQuickWidget::Error ||
        (qmlWidget->status() == QQuickWidget::Ready && !qmlWidget->rootObject())) {
        showLoadError();
    }
}

JavaPage::~JavaPage() = default;

void JavaPage::setMinMemory(int value)
{
    value = qBound(128, value, m_memoryMaximum);
    if (m_minMemory == value) {
        return;
    }
    m_minMemory = value;
    if (m_minMemory > m_maxMemory) {
        m_maxMemory = m_minMemory;
        emit maxMemoryChanged();
    }
    emit minMemoryChanged();
}

void JavaPage::setMaxMemory(int value)
{
    value = qBound(128, value, m_memoryMaximum);
    if (m_maxMemory == value) {
        return;
    }
    m_maxMemory = value;
    if (m_maxMemory < m_minMemory) {
        m_minMemory = m_maxMemory;
        emit minMemoryChanged();
    }
    emit maxMemoryChanged();
}

void JavaPage::setPermGen(int value)
{
    value = qBound(64, value, m_memoryMaximum);
    if (m_permGen != value) {
        m_permGen = value;
        emit permGenChanged();
    }
}

void JavaPage::setJavaPath(const QString &path)
{
    if (m_javaPath != path) {
        m_javaPath = path;
        emit javaPathChanged();
    }
}

void JavaPage::setJvmArgs(const QString &args)
{
    if (m_jvmArgs != args) {
        m_jvmArgs = args;
        emit jvmArgsChanged();
    }
}

void JavaPage::loadSettings()
{
    auto settings = APPLICATION->settings();
    int min = settings->get("MinMemAlloc").toInt();
    int max = settings->get("MaxMemAlloc").toInt();
    m_minMemory = qBound(128, qMin(min, max), m_memoryMaximum);
    m_maxMemory = qBound(128, qMax(min, max), m_memoryMaximum);
    m_permGen = qBound(64, settings->get("PermGen").toInt(), m_memoryMaximum);
    m_javaPath = settings->get("JavaPath").toString();
    m_jvmArgs = settings->get("JvmArgs").toString();
}

bool JavaPage::apply()
{
    applySettings();
    return true;
}

void JavaPage::applySettings()
{
    auto settings = APPLICATION->settings();
    settings->set("MinMemAlloc", m_minMemory);
    settings->set("MaxMemAlloc", m_maxMemory);
    settings->set("PermGen", m_permGen);
    settings->set("JavaPath", m_javaPath);
    settings->set("JvmArgs", m_jvmArgs);
    JavaCommon::checkJVMArgs(m_jvmArgs, this);
}

void JavaPage::browseJava()
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

void JavaPage::detectJava()
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

void JavaPage::testJava()
{
    if (checker) {
        return;
    }
    checker.reset(new JavaCommon::TestCheck(
        this, m_javaPath, m_jvmArgs, m_minMemory, m_maxMemory, m_permGen));
    connect(checker.get(), &JavaCommon::TestCheck::finished, this,
            [this]() { checker.reset(); });
    checker->run();
}
