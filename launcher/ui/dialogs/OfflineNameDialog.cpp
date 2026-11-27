#include "OfflineNameDialog.h"

#include <QDebug>
#include <QQuickWidget>
#include <QUrl>
#include <QVBoxLayout>

#include "ui/GuiUtil.h"
#include "ui/widgets/CustomTitleBar.h"

OfflineNameDialog::OfflineNameDialog(QString current_name, QWidget *parent)
    : QDialog(parent), m_textValue(current_name) {
    setWindowFlags(Qt::FramelessWindowHint | windowFlags());
    setWindowTitle(tr("Player name"));
    resize(420, 240);

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    auto *titleBar = new CustomTitleBar(this);
    titleBar->setTitle(tr("Player name"));
    mainLayout->addWidget(titleBar);

    auto *qmlWidget = new QQuickWidget(this);
    qmlWidget->setObjectName(QStringLiteral("offlineNameDialogQml"));
    qmlWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    qmlWidget->setInitialProperties(
        {{QStringLiteral("dialogController"),
          QVariant::fromValue(static_cast<QObject *>(this))}});

    connect(qmlWidget, &QQuickWidget::statusChanged, this,
            [qmlWidget](QQuickWidget::Status status) {
                if (status == QQuickWidget::Error) {
                    for (const auto &err : qmlWidget->errors()) {
                        qCritical() << "OfflineNameDialog QML error:" << err.toString();
                    }
                }
            });

    qmlWidget->setSource(QUrl(QStringLiteral("qrc:/qml/OfflineNameDialog.qml")));
    mainLayout->addWidget(qmlWidget);

    GuiUtil::fitWindowToAvailableScreen(this, size());
}

OfflineNameDialog::~OfflineNameDialog() = default;

void OfflineNameDialog::setTextValue(const QString &text) {
    if (m_textValue != text) {
        m_textValue = text;
        emit textValueChanged();
        emit canAcceptChanged();
    }
}

void OfflineNameDialog::acceptDialog() {
    if (canAccept()) {
        accept();
    }
}

void OfflineNameDialog::rejectDialog() {
    reject();
}

void OfflineNameDialog::retryOnline() {
    done(OnlineRequested);
}
