#include "NotificationDialog.h"

#include <QLabel>
#include <QQuickWidget>
#include <QTimerEvent>
#include <QUrl>
#include <QVBoxLayout>

#include "ui/GuiUtil.h"
#include "ui/widgets/CustomTitleBar.h"

NotificationDialog::NotificationDialog(
    const NotificationChecker::NotificationEntry &entry, QWidget *parent)
    : QDialog(parent) {
  setWindowFlags(Qt::FramelessWindowHint | windowFlags());
  setWindowTitle(tr("Notification"));
  resize(450, 240);

  m_message = entry.message;
  switch (entry.type) {
  case NotificationChecker::NotificationEntry::Critical:
    m_type = "critical";
    break;
  case NotificationChecker::NotificationEntry::Warning:
    m_type = "warning";
    break;
  default:
  case NotificationChecker::NotificationEntry::Information:
    m_type = "info";
    break;
  }

  auto *mainLayout = new QVBoxLayout(this);
  mainLayout->setContentsMargins(0, 0, 0, 0);
  mainLayout->setSpacing(0);

  auto *titleBar = new CustomTitleBar(this);
  titleBar->setTitle(tr("Notification"));
  mainLayout->addWidget(titleBar);

  auto *qmlWidget = new QQuickWidget(this);
  qmlWidget->setObjectName(QStringLiteral("notificationDialogQml"));
  qmlWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
  qmlWidget->setInitialProperties(
      {{QStringLiteral("dialogController"),
        QVariant::fromValue(static_cast<QObject *>(this))}});

  connect(qmlWidget, &QQuickWidget::statusChanged, this,
          [qmlWidget](QQuickWidget::Status status) {
            if (status == QQuickWidget::Error) {
              for (const auto &err : qmlWidget->errors()) {
                qCritical() << "NotificationDialog QML error:" << err.toString();
              }
            }
          });

  qmlWidget->setSource(QUrl(QStringLiteral("qrc:/qml/NotificationDialog.qml")));
  mainLayout->addWidget(qmlWidget);

  startTimer(1000);
  GuiUtil::fitWindowToAvailableScreen(this, size());
}

NotificationDialog::~NotificationDialog() = default;

void NotificationDialog::timerEvent(QTimerEvent *event) {
  bool changed = false;
  if (m_dontShowAgainTime > 0) {
    m_dontShowAgainTime--;
    changed = true;
  }
  if (m_closeTime > 0) {
    m_closeTime--;
    changed = true;
  }

  if (changed) {
    emit countdownChanged();
  }

  if (m_closeTime == 0 && m_dontShowAgainTime == 0) {
    killTimer(event->timerId());
  }
}

void NotificationDialog::dismiss() {
  if (m_closeTime == 0) {
    done(Normal);
  }
}

void NotificationDialog::dontShowAgain() {
  if (m_dontShowAgainTime == 0) {
    done(DontShowAgain);
  }
}
