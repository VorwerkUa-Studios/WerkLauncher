#include "LegacyUpgradePage.h"

#include <QLabel>
#include <QQuickWidget>
#include <QStackedLayout>

#include "BaseInstance.h"
#include "InstanceList.h"
#include "minecraft/legacy/LegacyInstance.h"
#include "minecraft/legacy/LegacyUpgradeTask.h"
#include "Application.h"

#include "ui/dialogs/CustomMessageBox.h"
#include "ui/dialogs/ProgressDialog.h"

LegacyUpgradePage::LegacyUpgradePage(InstancePtr inst, QWidget *parent)
    : QWidget(parent), m_inst(std::move(inst))
{
    connect(m_inst.get(), &BaseInstance::runningStatusChanged, this,
            &LegacyUpgradePage::updateRunningStatus);

    auto *stack = new QStackedLayout(this);
    stack->setContentsMargins(0, 0, 0, 0);

    auto *qmlWidget = new QQuickWidget(this);
    qmlWidget->setObjectName(QStringLiteral("legacyUpgradePageQml"));
    qmlWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    qmlWidget->setInitialProperties(
        {{QStringLiteral("pageController"),
          QVariant::fromValue(static_cast<QObject *>(this))}});
    stack->addWidget(qmlWidget);
    stack->setCurrentWidget(qmlWidget);

    auto *errorLabel = new QLabel(tr("The legacy upgrade page could not be loaded."), this);
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
        qCritical() << "Legacy upgrade page QML failed to load:" << details.join('\n');
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
    qmlWidget->setSource(QUrl(QStringLiteral("qrc:/qml/LegacyUpgradePage.qml")));
    if (qmlWidget->status() == QQuickWidget::Error ||
        (qmlWidget->status() == QQuickWidget::Ready && !qmlWidget->rootObject())) {
        showLoadError();
    }
}

LegacyUpgradePage::~LegacyUpgradePage() = default;

bool LegacyUpgradePage::upgradeEnabled() const
{
    return !m_inst->isRunning();
}

void LegacyUpgradePage::updateRunningStatus(bool)
{
    emit upgradeEnabledChanged();
}

void LegacyUpgradePage::runModalTask(Task *task)
{
    connect(task, &Task::failed, [this](QString reason)
        {
            CustomMessageBox::selectable(this, tr("Error"), reason, QMessageBox::Warning)->show();
        });
    ProgressDialog loadDialog(this);
    loadDialog.setSkipButton(true, tr("Abort"));
    if(loadDialog.execWithTask(task) == QDialog::Accepted)
    {
        m_container->requestClose();
    }
}

void LegacyUpgradePage::upgrade()
{
    if (!upgradeEnabled()) {
        return;
    }
    QString newName = tr("%1 (Migrated)").arg(m_inst->name());
    auto upgradeTask = new LegacyUpgradeTask(m_inst);
    upgradeTask->setName(newName);
    upgradeTask->setGroup(APPLICATION->instances()->getInstanceGroup(m_inst->id()));
    upgradeTask->setIcon(m_inst->iconKey());
    unique_qobject_ptr<Task> task(APPLICATION->instances()->wrapInstanceTask(upgradeTask));
    runModalTask(task.get());
}

bool LegacyUpgradePage::shouldDisplay() const
{
    return !m_inst->isRunning();
}
