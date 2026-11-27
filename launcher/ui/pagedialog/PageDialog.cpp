#include "PageDialog.h"

#include <QKeyEvent>
#include <QVBoxLayout>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

#include "Application.h"
#include "settings/SettingsObject.h"

#include "../widgets/CustomTitleBar.h"
#include "ui/widgets/IconLabel.h"
#include "ui/widgets/PageContainer.h"


PageDialog::PageDialog(BasePageProvider *pageProvider, QString defaultId,
                       QWidget *parent)
    : QWidget(parent) {

  setWindowTitle(tr("Settings"));
  m_container = new PageContainer(pageProvider, defaultId, this);

  QVBoxLayout *mainLayout = new QVBoxLayout;
  mainLayout->setContentsMargins(0, 0, 0, 0);
  mainLayout->setSpacing(0);

  mainLayout->addWidget(m_container);

  setLayout(mainLayout);

}

void PageDialog::closeEvent(QCloseEvent *event) {
  qDebug() << "Paged dialog close requested";
  if (m_container->prepareToClose()) {
    qDebug() << "Paged dialog close approved";
    QWidget::closeEvent(event);
  }
}
