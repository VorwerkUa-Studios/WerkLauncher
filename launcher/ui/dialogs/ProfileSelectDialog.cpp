#include "ProfileSelectDialog.h"

#include <QLabel>
#include <QQuickWidget>
#include <QUrl>
#include <QVBoxLayout>

#include "Application.h"
#include "ui/GuiUtil.h"
#include "ui/widgets/CustomTitleBar.h"

ProfileSelectDialog::ProfileSelectDialog(const QString &message, int flags,
                                         QWidget *parent)
    : QDialog(parent), m_message(message), m_flags(flags) {
  setWindowFlags(Qt::FramelessWindowHint | windowFlags());
  setWindowTitle(tr("Select Profile"));
  resize(440, 380);

  m_accounts = APPLICATION->accounts();
  for (int i = 0; i < m_accounts->count(); i++) {
    auto entry = m_accounts->at(i);
    if (entry.isAccount && entry.account) {
      m_accountsList.append(entry.account);
    }
  }

  auto *mainLayout = new QVBoxLayout(this);
  mainLayout->setContentsMargins(0, 0, 0, 0);
  mainLayout->setSpacing(0);

  auto *titleBar = new CustomTitleBar(this);
  titleBar->setTitle(tr("Select Profile"));
  mainLayout->addWidget(titleBar);

  auto *qmlWidget = new QQuickWidget(this);
  qmlWidget->setObjectName(QStringLiteral("profileSelectDialogQml"));
  qmlWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
  qmlWidget->setInitialProperties(
      {{QStringLiteral("dialogController"),
        QVariant::fromValue(static_cast<QObject *>(this))}});

  connect(qmlWidget, &QQuickWidget::statusChanged, this,
          [qmlWidget](QQuickWidget::Status status) {
            if (status == QQuickWidget::Error) {
              for (const auto &err : qmlWidget->errors()) {
                qCritical() << "ProfileSelectDialog QML error:" << err.toString();
              }
            }
          });

  qmlWidget->setSource(QUrl(QStringLiteral("qrc:/qml/ProfileSelectDialog.qml")));
  mainLayout->addWidget(qmlWidget);

  GuiUtil::fitWindowToAvailableScreen(this, size());
}

ProfileSelectDialog::~ProfileSelectDialog() = default;

QVariantList ProfileSelectDialog::accountsList() const {
  QVariantList list;
  for (const auto &account : m_accountsList) {
    QVariantMap map;
    map["name"] = account->profileName();
    map["inUse"] = account->isInUse();
    map["type"] = account->typeString();
    list.append(map);
  }
  return list;
}

void ProfileSelectDialog::setSelectedIndex(int index) {
  if (m_selectedIndex != index) {
    m_selectedIndex = index;
    emit selectedIndexChanged();
  }
}

void ProfileSelectDialog::setUseAsGlobalDefault(bool value) {
  if (m_useAsGlobalDefault != value) {
    m_useAsGlobalDefault = value;
    emit useAsGlobalDefaultChanged();
  }
}

void ProfileSelectDialog::setUseAsInstDefault(bool value) {
  if (m_useAsInstDefault != value) {
    m_useAsInstDefault = value;
    emit useAsInstDefaultChanged();
  }
}

MinecraftAccountPtr ProfileSelectDialog::selectedAccount() const {
  return m_selected;
}

void ProfileSelectDialog::acceptDialog() {
  if (m_selectedIndex >= 0 && m_selectedIndex < m_accountsList.size()) {
    m_selected = m_accountsList.at(m_selectedIndex);
    accept();
  }
}

void ProfileSelectDialog::rejectDialog() {
  reject();
}
