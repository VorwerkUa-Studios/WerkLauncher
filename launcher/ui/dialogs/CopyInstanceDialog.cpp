#include "CopyInstanceDialog.h"

#include <QLabel>
#include <QQuickWidget>
#include <QUrl>
#include <QVBoxLayout>

#include "Application.h"
#include "InstanceList.h"
#include "icons/IconList.h"
#include "ui/GuiUtil.h"
#include "ui/dialogs/IconPickerDialog.h"
#include "ui/widgets/CustomTitleBar.h"

CopyInstanceDialog::CopyInstanceDialog(InstancePtr original, QWidget *parent)
    : QDialog(parent), m_original(original) {
    setWindowFlags(Qt::FramelessWindowHint | windowFlags());
    setWindowTitle(tr("Copy Instance"));
    resize(460, 420);

    m_iconKey = original->iconKey();
    m_instName = original->name();
    m_instGroup = APPLICATION->instances()->getInstanceGroup(m_original->id());

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    auto *titleBar = new CustomTitleBar(this);
    titleBar->setTitle(tr("Copy Instance"));
    mainLayout->addWidget(titleBar);

    auto *qmlWidget = new QQuickWidget(this);
    qmlWidget->setObjectName(QStringLiteral("copyInstanceDialogQml"));
    qmlWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    qmlWidget->setInitialProperties(
        {{QStringLiteral("dialogController"),
          QVariant::fromValue(static_cast<QObject *>(this))}});

    connect(qmlWidget, &QQuickWidget::statusChanged, this,
            [qmlWidget](QQuickWidget::Status status) {
                if (status == QQuickWidget::Error) {
                    for (const auto &err : qmlWidget->errors()) {
                        qCritical() << "CopyInstanceDialog QML error:" << err.toString();
                    }
                }
            });

    qmlWidget->setSource(QUrl(QStringLiteral("qrc:/qml/CopyInstanceDialog.qml")));
    mainLayout->addWidget(qmlWidget);

    GuiUtil::fitWindowToAvailableScreen(this, size());
}

CopyInstanceDialog::~CopyInstanceDialog() = default;

QStringList CopyInstanceDialog::availableGroups() const {
    auto groupList = APPLICATION->instances()->getGroups();
    groupList.removeDuplicates();
    groupList.sort(Qt::CaseInsensitive);
    groupList.removeOne("");
    groupList.push_front("");
    return groupList;
}

void CopyInstanceDialog::setInstName(const QString &value) {
    if (m_instName != value) {
        m_instName = value;
        emit instNameChanged();
        emit canAcceptChanged();
    }
}

void CopyInstanceDialog::setInstGroup(const QString &value) {
    if (m_instGroup != value) {
        m_instGroup = value;
        emit instGroupChanged();
    }
}

void CopyInstanceDialog::setIconKey(const QString &value) {
    if (m_iconKey != value) {
        m_iconKey = value;
        emit iconKeyChanged();
    }
}

void CopyInstanceDialog::setShouldCopySaves(bool value) {
    if (m_copySaves != value) {
        m_copySaves = value;
        emit shouldCopySavesChanged();
    }
}

void CopyInstanceDialog::setShouldKeepPlaytime(bool value) {
    if (m_keepPlaytime != value) {
        m_keepPlaytime = value;
        emit shouldKeepPlaytimeChanged();
    }
}

void CopyInstanceDialog::pickIcon() {
    IconPickerDialog dlg(this);
    dlg.execWithSelection(m_iconKey);
    if (dlg.result() == QDialog::Accepted) {
        setIconKey(dlg.selectedIconKey);
    }
}

void CopyInstanceDialog::acceptDialog() {
    if (canAccept()) {
        accept();
    }
}

void CopyInstanceDialog::rejectDialog() {
    reject();
}
