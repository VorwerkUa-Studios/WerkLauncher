#pragma once

#include <QFile>

inline bool restoreReplacementBackup(const QString &original,
                                     const QString &backup,
                                     const QString &update) {
  if (QFile::exists(original) && !QFile::rename(original, update)) {
    return false;
  }
  return QFile::exists(backup) && QFile::rename(backup, original);
}
