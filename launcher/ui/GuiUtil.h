#pragma once

#include <QSize>
#include <QWidget>

namespace GuiUtil
{
QString uploadPaste(const QString &text, QWidget *parentWidget);
void setClipboardText(const QString &text);
void fitWindowToAvailableScreen(QWidget *window, const QSize &preferredSize);
QStringList BrowseForFiles(QString context, QString caption, QString filter, QString defaultPath, QWidget *parentWidget);
QString BrowseForFile(QString context, QString caption, QString filter, QString defaultPath, QWidget *parentWidget);
}
