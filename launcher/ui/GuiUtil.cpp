#include "GuiUtil.h"

#include <QClipboard>
#include <QApplication>
#include <QFileDialog>
#include <QGuiApplication>
#include <QScreen>

#include "ui/dialogs/ProgressDialog.h"
#include "ui/dialogs/CustomMessageBox.h"
#include "net/PasteUpload.h"

#include "Application.h"
#include <settings/SettingsObject.h>
#include <DesktopServices.h>
#include <BuildConfig.h>

QString GuiUtil::uploadPaste(const QString &text, QWidget *parentWidget)
{
    ProgressDialog dialog(parentWidget);
    auto APIKeySetting = APPLICATION->settings()->get("PasteEEAPIKey").toString();
    if(APIKeySetting == "multimc")
    {
        APIKeySetting = BuildConfig.PASTE_EE_KEY;
    }
    std::unique_ptr<PasteUpload> paste(new PasteUpload(parentWidget, text, APIKeySetting));

    if (!paste->validateText())
    {
        CustomMessageBox::selectable(
            parentWidget, QObject::tr("Upload failed"),
            QObject::tr("The log file is too big. You'll have to upload it manually."),
            QMessageBox::Warning)->exec();
        return QString();
    }

    dialog.execWithTask(paste.get());
    if (!paste->wasSuccessful())
    {
        CustomMessageBox::selectable(
            parentWidget,
            QObject::tr("Upload failed"),
            paste->failReason(),
            QMessageBox::Critical
        )->exec();
        return QString();
    }
    else
    {
        const QString link = paste->pasteLink();
        setClipboardText(link);
        CustomMessageBox::selectable(
            parentWidget, QObject::tr("Upload finished"),
            QObject::tr("The <a href=\"%1\">link to the uploaded log</a> has been placed in your clipboard.").arg(link),
            QMessageBox::Information)->exec();
        return link;
    }
}

void GuiUtil::setClipboardText(const QString &text)
{
    QApplication::clipboard()->setText(text);
}

void GuiUtil::fitWindowToAvailableScreen(QWidget *window, const QSize &preferredSize)
{
    if (!window) {
        return;
    }

    QScreen *screen = window->parentWidget() ? window->parentWidget()->screen()
                                             : window->screen();
    if (!screen) {
        screen = QGuiApplication::primaryScreen();
    }
    if (!screen) {
        return;
    }

    const QRect available = screen->availableGeometry();
    const QSize maxSize(qMax(1, available.width() - 48),
                        qMax(1, available.height() - 48));
    window->resize(preferredSize.boundedTo(maxSize));
    const QSize size = window->size();

    const QWidget *parent = window->parentWidget();
    const QPoint center = parent ? parent->frameGeometry().center()
                                 : available.center();
    const int maxX = qMax(available.left(),
                          available.right() - size.width() + 1);
    const int maxY = qMax(available.top(),
                          available.bottom() - size.height() + 1);
    const QPoint topLeft(
        qBound(available.left(), center.x() - size.width() / 2, maxX),
        qBound(available.top(), center.y() - size.height() / 2, maxY));
    window->move(topLeft);
}

static QStringList BrowseForFileInternal(QString context, QString caption, QString filter, QString defaultPath, QWidget *parentWidget, bool single)
{
    static QMap<QString, QString> savedPaths;

    QFileDialog w(parentWidget, caption);
    QSet<QString> locations;
    auto f = [&](QStandardPaths::StandardLocation l)
    {
        QString location = QStandardPaths::writableLocation(l);
        QFileInfo finfo(location);
        if (!finfo.exists()) {
            return;
        }
        locations.insert(location);
    };
    f(QStandardPaths::DesktopLocation);
    f(QStandardPaths::DocumentsLocation);
    f(QStandardPaths::DownloadLocation);
    f(QStandardPaths::HomeLocation);
    QList<QUrl> urls;
    for (auto location : locations)
    {
        urls.append(QUrl::fromLocalFile(location));
    }
    urls.append(QUrl::fromLocalFile(defaultPath));

    w.setFileMode(single ? QFileDialog::ExistingFile : QFileDialog::ExistingFiles);
    w.setAcceptMode(QFileDialog::AcceptOpen);
    w.setNameFilter(filter);

    QString pathToOpen;
    if(savedPaths.contains(context))
    {
        pathToOpen = savedPaths[context];
    }
    else
    {
        pathToOpen = defaultPath;
    }
    if(!pathToOpen.isEmpty())
    {
        QFileInfo finfo(pathToOpen);
        if(finfo.exists() && finfo.isDir())
        {
            w.setDirectory(finfo.absoluteFilePath());
        }
    }

    w.setSidebarUrls(urls);

    if (w.exec())
    {
        savedPaths[context] = w.directory().absolutePath();
        return w.selectedFiles();
    }
    savedPaths[context] = w.directory().absolutePath();
    return {};
}

QString GuiUtil::BrowseForFile(QString context, QString caption, QString filter, QString defaultPath, QWidget *parentWidget)
{
    auto resultList = BrowseForFileInternal(context, caption, filter, defaultPath, parentWidget, true);
    if(resultList.size())
    {
        return resultList[0];
    }
    return QString();
}


QStringList GuiUtil::BrowseForFiles(QString context, QString caption, QString filter, QString defaultPath, QWidget *parentWidget)
{
    return BrowseForFileInternal(context, caption, filter, defaultPath, parentWidget, false);
}
