#include "LogPage.h"

#include <QApplication>
#include <QDateTime>
#include <QDebug>
#include <QHash>
#include <QLabel>
#include <QMessageBox>
#include <QPalette>
#include <QQuickWidget>
#include <QStackedLayout>
#include <QStringList>
#include <QUrl>
#include <QVariant>

#include "Application.h"
#include "BuildConfig.h"
#include "launch/LogModel.h"
#include "settings/Setting.h"
#include "ui/ColorCache.h"
#include "ui/GuiUtil.h"
#include "ui/dialogs/CustomMessageBox.h"

class LogFormatProxyModel : public QIdentityProxyModel {
public:
    explicit LogFormatProxyModel(QObject *parent = nullptr)
        : QIdentityProxyModel(parent)
    {
    }

    QVariant data(const QModelIndex &index, int role) const override
    {
        if (role == Qt::FontRole) {
            return m_font;
        }
        if (role == Qt::ForegroundRole || role == Qt::BackgroundRole) {
            const auto level = static_cast<MessageLevel::Enum>(
                QIdentityProxyModel::data(index, LogModel::LevelRole).toInt());
            return role == Qt::ForegroundRole ? m_colors->getFront(level)
                                               : m_colors->getBack(level);
        }
        return QIdentityProxyModel::data(index, role);
    }

    QHash<int, QByteArray> roleNames() const override
    {
        auto roles = QIdentityProxyModel::roleNames();
        roles.insert(Qt::DisplayRole, QByteArrayLiteral("display"));
        roles.insert(Qt::FontRole, QByteArrayLiteral("font"));
        roles.insert(Qt::ForegroundRole, QByteArrayLiteral("foreground"));
        roles.insert(Qt::BackgroundRole, QByteArrayLiteral("background"));
        return roles;
    }

    void setFont(const QFont &font) { m_font = font; }
    void setColors(LogColorCache *colors) { m_colors.reset(colors); }

private:
    QFont m_font;
    std::unique_ptr<LogColorCache> m_colors;
};

LogPage::LogPage(InstancePtr instance, QWidget *parent)
    : QWidget(parent), m_instance(std::move(instance))
{
    m_proxy = new LogFormatProxyModel(this);
    const auto palette = QApplication::palette();
    m_proxy->setColors(new LogColorCache(palette.color(QPalette::Text),
                                         palette.color(QPalette::Base)));

    const QString fontFamily =
        APPLICATION->settings()->get("ConsoleFont").toString();
    bool conversionOk = false;
    int fontSize =
        APPLICATION->settings()->get("ConsoleFontSize").toInt(&conversionOk);
    if (!conversionOk) {
        fontSize = 11;
    }
    m_proxy->setFont(QFont(fontFamily, fontSize));

    if (const auto launchTask = m_instance->getLaunchTask()) {
        setInstanceLaunchTaskChanged(launchTask, true);
    }
    connect(m_instance.get(), &BaseInstance::launchTaskChanged, this,
            &LogPage::onInstanceLaunchTaskChanged);

    auto *stack = new QStackedLayout(this);
    stack->setContentsMargins(0, 0, 0, 0);

    auto *qmlWidget = new QQuickWidget(this);
    qmlWidget->setObjectName(QStringLiteral("logPageQml"));
    qmlWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    qmlWidget->setInitialProperties(
        {{QStringLiteral("pageController"),
          QVariant::fromValue(static_cast<QObject *>(this))}});
    stack->addWidget(qmlWidget);
    stack->setCurrentWidget(qmlWidget);

    auto *errorLabel = new QLabel(tr("The Minecraft log page could not be loaded."), this);
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
        qCritical() << "Minecraft log page QML failed to load:"
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
    qmlWidget->setSource(QUrl(QStringLiteral("qrc:/qml/LogPage.qml")));
    if (qmlWidget->status() == QQuickWidget::Error ||
        (qmlWidget->status() == QQuickWidget::Ready && !qmlWidget->rootObject())) {
        showLoadError();
    }
}

LogPage::~LogPage() = default;

QAbstractItemModel *LogPage::logModel() const
{
    return m_proxy;
}

bool LogPage::hasLogModel() const
{
    return m_model != nullptr;
}

bool LogPage::apply()
{
    return true;
}

bool LogPage::shouldDisplay() const
{
    return m_instance->isRunning() || m_proxy->rowCount() > 0;
}

void LogPage::setTracking(bool tracking)
{
    if (m_tracking == tracking) {
        return;
    }
    m_tracking = tracking;
    if (m_model) {
        m_model->suspend(!m_tracking);
    }
    emit controlsChanged();
}

void LogPage::setWrapLines(bool wrap)
{
    if (m_wrapLines == wrap) {
        return;
    }
    m_wrapLines = wrap;
    if (m_model) {
        m_model->setLineWrap(m_wrapLines);
    }
    emit controlsChanged();
}

void LogPage::setInstanceLaunchTaskChanged(shared_qobject_ptr<LaunchTask> proc,
                                           bool initial)
{
    m_process = std::move(proc);
    if (m_process) {
        m_model = m_process->getLogModel();
        m_proxy->setSourceModel(m_model.get());
        if (initial) {
            m_wrapLines = m_model->wrapLines();
            m_tracking = !m_model->suspended();
        } else {
            m_model->setLineWrap(m_wrapLines);
            m_model->suspend(!m_tracking);
        }
    } else {
        m_proxy->setSourceModel(nullptr);
        m_model.reset();
    }
    if (m_foundRow != -1) {
        m_foundRow = -1;
        emit foundRowChanged();
    }
    emit controlsChanged();
}

void LogPage::onInstanceLaunchTaskChanged(shared_qobject_ptr<LaunchTask> proc)
{
    setInstanceLaunchTaskChanged(std::move(proc), false);
}

void LogPage::copyLog()
{
    if (!m_model) {
        return;
    }
    m_model->append(
        MessageLevel::Launcher,
        QStringLiteral("Clipboard copy at: %1")
            .arg(QDateTime::currentDateTime().toString(Qt::RFC2822Date)));
    GuiUtil::setClipboardText(m_model->toPlainText());
}

void LogPage::uploadLog()
{
    if (!m_model) {
        return;
    }
    const auto response = CustomMessageBox::selectable(
                              this, tr("Log upload"),
                              tr("Are you sure you want to upload this log file?"),
                              QMessageBox::Warning,
                              QMessageBox::Yes | QMessageBox::No,
                              QMessageBox::No)
                              ->exec();
    if (response != QMessageBox::Yes) {
        return;
    }

    m_model->append(
        MessageLevel::Launcher,
        QStringLiteral("%2: Log upload triggered at: %1")
            .arg(QDateTime::currentDateTime().toString(Qt::RFC2822Date),
                 BuildConfig.LAUNCHER_NAME));
    const auto url = GuiUtil::uploadPaste(m_model->toPlainText(), this);
    if (!url.isEmpty()) {
        m_model->append(
            MessageLevel::Launcher,
            QStringLiteral("%2: Log uploaded to: %1")
                .arg(url, BuildConfig.LAUNCHER_NAME));
    } else {
        m_model->append(
            MessageLevel::Error,
            QStringLiteral("%1: Log upload failed!")
                .arg(BuildConfig.LAUNCHER_NAME));
    }
}

void LogPage::clearLog()
{
    if (!m_model) {
        return;
    }
    m_model->clear();
    m_foundRow = -1;
    emit foundRowChanged();
    if (m_container) {
        m_container->refreshContainer();
    }
}

int LogPage::findNext(const QString &text, bool reverse)
{
    const int count = m_proxy->rowCount();
    const QString needle = text.trimmed();
    int result = -1;
    if (count > 0 && !needle.isEmpty()) {
        for (int offset = 0; offset < count; ++offset) {
            int row = 0;
            if (m_foundRow < 0) {
                row = reverse ? count - 1 - offset : offset;
            } else {
                row = (m_foundRow + (reverse ? -offset - 1 : offset + 1) + count) % count;
            }
            if (m_proxy->data(m_proxy->index(row, 0), Qt::DisplayRole)
                    .toString()
                    .contains(needle, Qt::CaseInsensitive)) {
                result = row;
                break;
            }
        }
    }
    if (m_foundRow != result) {
        m_foundRow = result;
        emit foundRowChanged();
    }
    return result;
}
