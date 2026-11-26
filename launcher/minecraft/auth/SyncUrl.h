#pragma once

#include <QHostAddress>
#include <QUrl>

inline bool isSafeSyncUrl(const QUrl &url) {
  if (url.scheme() == "https") {
    return true;
  }
  if (url.scheme() != "http") {
    return false;
  }

  const QString host = url.host().toLower();
  if (host == "localhost") {
    return true;
  }

  QHostAddress address;
  return address.setAddress(host) && address.isLoopback();
}
