#include "ProgressDialog.h"

#include <QDebug>
#include <QKeyEvent>
#include <QLabel>
#include <QQuickWidget>
#include <QUrl>
#include <QVBoxLayout>

#include "tasks/Task.h"
#include "ui/GuiUtil.h"
#include "ui/widgets/CustomTitleBar.h"

ProgressDialog::ProgressDialog(QWidget *parent)
    : QDialog(parent) {
  setWindowFlags(Qt::FramelessWindowHint | windowFlags());
  setWindowTitle(tr("Progress"));
  resize(480, 160);

  auto *mainLayout = new QVBoxLayout(this);
  mainLayout->setContentsMargins(0, 0, 0, 0);
  mainLayout->setSpacing(0);

  auto *titleBar = new CustomTitleBar(this);
  titleBar->setTitle(tr("Progress"));
  mainLayout->addWidget(titleBar);

  auto *qmlWidget = new QQuickWidget(this);
  qmlWidget->setObjectName(QStringLiteral("progressDialogQml"));
  qmlWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
  qmlWidget->setInitialProperties(
      {{QStringLiteral("dialogController"),
        QVariant::fromValue(static_cast<QObject *>(this))}});

  connect(qmlWidget, &QQuickWidget::statusChanged, this,
          [qmlWidget](QQuickWidget::Status status) {
            if (status == QQuickWidget::Error) {
              for (const auto &err : qmlWidget->errors()) {
                qCritical() << "ProgressDialog QML error:" << err.toString();
              }
            }
          });

  qmlWidget->setSource(QUrl(QStringLiteral("qrc:/qml/ProgressDialog.qml")));
  mainLayout->addWidget(qmlWidget);

  setSkipButton(false);
  changeProgress(0, 100);
  GuiUtil::fitWindowToAvailableScreen(this, size());
}

ProgressDialog::~ProgressDialog() = default;

void ProgressDialog::updateSize() {
  setFixedWidth(480);
}

void ProgressDialog::setSkipButton(bool present, QString label) {
  m_skipButtonVisible = present;
  m_skipButtonLabel = label;
  emit skipButtonChanged();
}

void ProgressDialog::abortTask() {
  if (task && task->isRunning()) {
    task->abort();
  }
}

int ProgressDialog::execWithTask(Task *t) {
  this->task = t;
  QDialog::DialogCode result;

  if (!task) {
    qDebug() << "Programmer error: progress dialog created with null task.";
    return Accepted;
  }

  if (handleImmediateResult(result)) {
    return result;
  }

  connect(task, SIGNAL(started()), SLOT(onTaskStarted()));
  connect(task, SIGNAL(failed(QString)), SLOT(onTaskFailed(QString)));
  connect(task, SIGNAL(succeeded()), SLOT(onTaskSucceeded()));
  connect(task, SIGNAL(status(QString)), SLOT(changeStatus(const QString &)));
  connect(task, SIGNAL(progress(qint64, qint64)), SLOT(changeProgress(qint64, qint64)));

  if (!task->isRunning()) {
    task->start();
  }
  if (task->isRunning()) {
    changeProgress(task->getProgress(), task->getTotalProgress());
    changeStatus(task->getStatus());
    return QDialog::exec();
  } else if (handleImmediateResult(result)) {
    return result;
  } else {
    return QDialog::Rejected;
  }
}

int ProgressDialog::execWithTask(std::unique_ptr<Task> &&t) {
  connect(this, &ProgressDialog::destroyed, t.get(), &Task::deleteLater);
  return execWithTask(t.release());
}

int ProgressDialog::execWithTask(std::unique_ptr<Task> &t) {
  connect(this, &ProgressDialog::destroyed, t.get(), &Task::deleteLater);
  return execWithTask(t.release());
}

bool ProgressDialog::handleImmediateResult(QDialog::DialogCode &result) {
  if (task->isFinished()) {
    if (task->wasSuccessful()) {
      result = QDialog::Accepted;
    } else {
      result = QDialog::Rejected;
    }
    return true;
  }
  return false;
}

Task *ProgressDialog::getTask() {
  return task;
}

void ProgressDialog::onTaskStarted() {}

void ProgressDialog::onTaskFailed(QString failure) {
  reject();
}

void ProgressDialog::onTaskSucceeded() {
  accept();
}

void ProgressDialog::changeStatus(const QString &status) {
  m_statusText = status;
  emit statusTextChanged();
}

void ProgressDialog::changeProgress(qint64 current, qint64 total) {
  m_progressValue = static_cast<int>(current);
  m_progressMaximum = static_cast<int>(total);
  emit progressChanged();
}

void ProgressDialog::keyPressEvent(QKeyEvent *e) {
  if (m_skipButtonVisible) {
    if (e->key() == Qt::Key_Escape) {
      abortTask();
      return;
    }
  }
  QDialog::keyPressEvent(e);
}

void ProgressDialog::closeEvent(QCloseEvent *e) {
  if (task && task->isRunning()) {
    if (task->canAbort()) {
      task->abort();
    }
    e->ignore();
  } else {
    QDialog::closeEvent(e);
  }
}
