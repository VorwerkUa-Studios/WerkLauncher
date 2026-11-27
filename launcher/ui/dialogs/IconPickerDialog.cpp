#include "IconPickerDialog.h"

#include <QFileDialog>
#include <QLabel>
#include <QQuickImageProvider>
#include <QQuickWidget>
#include <QQmlEngine>
#include <QUrl>
#include <QVBoxLayout>

#include "Application.h"
#include "DesktopServices.h"
#include "icons/IconList.h"
#include "icons/IconUtils.h"
#include "ui/GuiUtil.h"
#include "ui/widgets/CustomTitleBar.h"

namespace {
class DialogThemedIconProvider : public QQuickImageProvider {
public:
    DialogThemedIconProvider() : QQuickImageProvider(QQuickImageProvider::Pixmap) {}

    QPixmap requestPixmap(const QString &id, QSize *size, const QSize &requestedSize) override {
        QString cleanId = id;
        int queryIndex = cleanId.indexOf('?');
        if (queryIndex != -1) {
            cleanId = cleanId.left(queryIndex);
        }

        QIcon icon = APPLICATION->getThemedIcon(cleanId);
        if (icon.isNull()) {
            icon = APPLICATION->icons()->getIcon(cleanId);
        }
        if (icon.isNull()) {
            return QPixmap();
        }
        QSize actualSize = requestedSize.isValid() ? requestedSize : QSize(48, 48);
        if (size) {
            *size = actualSize;
        }
        return icon.pixmap(actualSize);
    }
};
}

IconPickerDialog::IconPickerDialog(QWidget *parent)
    : QDialog(parent) {
  setWindowFlags(Qt::FramelessWindowHint | windowFlags());
  setWindowTitle(tr("Select Icon"));
  resize(520, 480);
  setWindowModality(Qt::WindowModal);

  auto *mainLayout = new QVBoxLayout(this);
  mainLayout->setContentsMargins(0, 0, 0, 0);
  mainLayout->setSpacing(0);

  auto *titleBar = new CustomTitleBar(this);
  titleBar->setTitle(tr("Select Icon"));
  mainLayout->addWidget(titleBar);

  auto *qmlWidget = new QQuickWidget(this);
  qmlWidget->setObjectName(QStringLiteral("iconPickerDialogQml"));
  qmlWidget->engine()->addImageProvider(QLatin1String("themedicon"), new DialogThemedIconProvider());
  qmlWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
  qmlWidget->setInitialProperties(
      {{QStringLiteral("dialogController"),
        QVariant::fromValue(static_cast<QObject *>(this))}});

  connect(qmlWidget, &QQuickWidget::statusChanged, this,
          [qmlWidget](QQuickWidget::Status status) {
            if (status == QQuickWidget::Error) {
              for (const auto &err : qmlWidget->errors()) {
                qCritical() << "IconPickerDialog QML error:" << err.toString();
              }
            }
          });

  qmlWidget->setSource(QUrl(QStringLiteral("qrc:/qml/IconPickerDialog.qml")));
  mainLayout->addWidget(qmlWidget);

  GuiUtil::fitWindowToAvailableScreen(this, size());
}

IconPickerDialog::~IconPickerDialog() = default;

QStringList IconPickerDialog::iconKeys() const {
  QStringList keys;
  auto list = APPLICATION->icons();
  if (list) {
    for (int i = 0; i < list->rowCount(); ++i) {
      QModelIndex idx = list->index(i);
      keys.append(idx.data(Qt::UserRole).toString());
    }
  }
  return keys;
}

bool IconPickerDialog::canRemove() const {
  auto list = APPLICATION->icons();
  if (!list || selectedIconKey.isEmpty()) {
    return false;
  }
  return list->iconFileExists(selectedIconKey);
}

void IconPickerDialog::setSelectedKey(const QString &key) {
  if (selectedIconKey != key) {
    selectedIconKey = key;
    emit selectedKeyChanged();
  }
}

int IconPickerDialog::execWithSelection(QString selection) {
  setSelectedKey(selection);
  return QDialog::exec();
}

void IconPickerDialog::addNewIcon() {
  QString selectIcons = tr("Select Icons");
  auto filter = IconUtils::getIconFilter();
  QStringList fileNames = QFileDialog::getOpenFileNames(
      this, selectIcons, QString(), tr("Icons %1").arg(filter));
  if (!fileNames.isEmpty()) {
    APPLICATION->icons()->installIcons(fileNames);
    emit iconKeysChanged();
  }
}

void IconPickerDialog::removeSelectedIcon() {
  if (canRemove()) {
    APPLICATION->icons()->deleteIcon(selectedIconKey);
    setSelectedKey("default");
    emit iconKeysChanged();
  }
}

void IconPickerDialog::openFolder() {
  DesktopServices::openDirectory(APPLICATION->icons()->getDirectory());
}

void IconPickerDialog::chooseIcon(const QString &key) {
  setSelectedKey(key);
  accept();
}

void IconPickerDialog::acceptDialog() {
  accept();
}

void IconPickerDialog::rejectDialog() {
  reject();
}
