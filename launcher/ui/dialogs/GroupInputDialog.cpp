#include "GroupInputDialog.h"

#include <QDebug>
#include <QQuickWidget>
#include <QUrl>
#include <QVBoxLayout>

#include "ui/GuiUtil.h"
#include "ui/widgets/CustomTitleBar.h"

GroupInputDialog::GroupInputDialog(const QString &title, const QString &label,
                                   const QString &initialText, QWidget *parent)
    : QDialog(parent),
      m_title(title),
      m_label(label),
      m_textValue(initialText) {
    setWindowFlags(Qt::FramelessWindowHint | windowFlags());
    setWindowTitle(title);
    resize(420, 240);

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    auto *titleBar = new CustomTitleBar(this);
    titleBar->setTitle(title);
    mainLayout->addWidget(titleBar);

    auto *qmlWidget = new QQuickWidget(this);
    qmlWidget->setObjectName(QStringLiteral("groupInputDialogQml"));
    qmlWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    qmlWidget->setInitialProperties(
        {{QStringLiteral("dialogController"),
          QVariant::fromValue(static_cast<QObject *>(this))}});

    connect(qmlWidget, &QQuickWidget::statusChanged, this,
            [qmlWidget](QQuickWidget::Status status) {
                if (status == QQuickWidget::Error) {
                    for (const auto &err : qmlWidget->errors()) {
                        qCritical() << "GroupInputDialog QML error:" << err.toString();
                    }
                }
            });

    qmlWidget->setSource(QUrl(QStringLiteral("qrc:/qml/GroupInputDialog.qml")));
    mainLayout->addWidget(qmlWidget);

    GuiUtil::fitWindowToAvailableScreen(this, size());
}

GroupInputDialog::~GroupInputDialog() = default;

void GroupInputDialog::setTextValue(const QString &text) {
    if (m_textValue != text) {
        m_textValue = text;
        emit textValueChanged();
        emit canAcceptChanged();
    }
}

void GroupInputDialog::acceptDialog() {
    if (canAccept()) {
        accept();
    }
}

void GroupInputDialog::rejectDialog() {
    reject();
}
