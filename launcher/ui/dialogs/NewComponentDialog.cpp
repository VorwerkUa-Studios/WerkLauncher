#include "NewComponentDialog.h"

#include <QDebug>
#include <QQuickWidget>
#include <QRegularExpression>
#include <QUrl>
#include <QVBoxLayout>

#include "ui/GuiUtil.h"
#include "ui/widgets/CustomTitleBar.h"

NewComponentDialog::NewComponentDialog(const QString &initialName,
                                       const QString &initialUid,
                                       QWidget *parent)
    : QDialog(parent),
      m_name(initialName),
      m_uid(initialUid),
      m_originalPlaceholderText(QStringLiteral("org.multimc.custom.component")) {
    setWindowFlags(Qt::FramelessWindowHint | windowFlags());
    setWindowTitle(tr("Add Empty Component"));
    resize(420, 260);

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    auto *titleBar = new CustomTitleBar(this);
    titleBar->setTitle(tr("New Component"));
    mainLayout->addWidget(titleBar);

    auto *qmlWidget = new QQuickWidget(this);
    qmlWidget->setObjectName(QStringLiteral("newComponentDialogQml"));
    qmlWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    qmlWidget->setInitialProperties(
        {{QStringLiteral("dialogController"),
          QVariant::fromValue(static_cast<QObject *>(this))}});

    connect(qmlWidget, &QQuickWidget::statusChanged, this,
            [qmlWidget](QQuickWidget::Status status) {
                if (status == QQuickWidget::Error) {
                    for (const auto &err : qmlWidget->errors()) {
                        qCritical() << "NewComponentDialog QML error:" << err.toString();
                    }
                }
            });

    qmlWidget->setSource(QUrl(QStringLiteral("qrc:/qml/NewComponentDialog.qml")));
    mainLayout->addWidget(qmlWidget);

    GuiUtil::fitWindowToAvailableScreen(this, size());

    updateDialogState();
}

NewComponentDialog::~NewComponentDialog() = default;

void NewComponentDialog::updateDialogState() {
    auto protoUid = m_name.toLower();
    protoUid.remove(QRegularExpression("[^a-z]"));
    if (protoUid.isEmpty()) {
        m_placeholderUid = m_originalPlaceholderText;
    } else {
        m_placeholderUid = "org.multimc.custom." + protoUid;
    }
    emit placeholderUidChanged();
    emit canAcceptChanged();
}

QString NewComponentDialog::name() const {
    if (!m_name.isEmpty()) {
        return m_name.trimmed();
    }
    return QString();
}

void NewComponentDialog::setName(const QString &name) {
    if (m_name != name) {
        m_name = name;
        emit nameChanged();
        updateDialogState();
    }
}

QString NewComponentDialog::uid() const {
    if (!m_uid.trimmed().isEmpty()) {
        return m_uid.trimmed();
    }
    if (!m_placeholderUid.isEmpty() && m_placeholderUid != m_originalPlaceholderText) {
        return m_placeholderUid.trimmed();
    }
    return QString();
}

void NewComponentDialog::setUid(const QString &uid) {
    if (m_uid != uid) {
        m_uid = uid;
        emit uidChanged();
        emit canAcceptChanged();
    }
}

bool NewComponentDialog::canAccept() const {
    return !name().isEmpty() && !uid().isEmpty() && !m_uidBlacklist.contains(uid());
}

void NewComponentDialog::setBlacklist(QStringList badUids) {
    m_uidBlacklist = badUids;
    emit canAcceptChanged();
}

void NewComponentDialog::acceptDialog() {
    if (canAccept()) {
        accept();
    }
}

void NewComponentDialog::rejectDialog() {
    reject();
}
