#include "GameOptionsPage.h"

#include <QLabel>
#include <QQuickWidget>
#include <QStackedLayout>
#include <QUrl>
#include <QVariant>
#include <QVariantMap>

#include "minecraft/MinecraftInstance.h"
#include "minecraft/gameoptions/GameOptions.h"

GameOptionsPage::GameOptionsPage(MinecraftInstance *inst, QWidget *parent)
    : QWidget(parent), m_model(inst->gameOptionsModel())
{
    auto *stack = new QStackedLayout(this);
    stack->setContentsMargins(0, 0, 0, 0);

    auto *qmlWidget = new QQuickWidget(this);
    qmlWidget->setObjectName(QStringLiteral("gameOptionsPageQml"));
    qmlWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    qmlWidget->setInitialProperties(
        {{QStringLiteral("pageController"),
          QVariant::fromValue(static_cast<QObject *>(this))}});
    stack->addWidget(qmlWidget);
    stack->setCurrentWidget(qmlWidget);

    auto *errorLabel =
        new QLabel(tr("The game options page could not be loaded."), this);
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
        qCritical() << "Game options page QML failed to load:"
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
    qmlWidget->setSource(QUrl(QStringLiteral("qrc:/qml/GameOptionsPage.qml")));
    if (qmlWidget->status() == QQuickWidget::Error ||
        (qmlWidget->status() == QQuickWidget::Ready && !qmlWidget->rootObject())) {
        showLoadError();
    }
}

GameOptionsPage::~GameOptionsPage() = default;

QVariantList GameOptionsPage::options() const
{
    QVariantList result;
    if (!m_model) {
        return result;
    }
    result.reserve(m_model->rowCount());
    for (int row = 0; row < m_model->rowCount(); ++row) {
        QVariantMap option;
        option.insert(
            QStringLiteral("key"),
            m_model->data(m_model->index(row, 0), Qt::DisplayRole).toString());
        option.insert(
            QStringLiteral("value"),
            m_model->data(m_model->index(row, 1), Qt::DisplayRole).toString());
        result.append(option);
    }
    return result;
}
