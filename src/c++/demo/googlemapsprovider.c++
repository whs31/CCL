//
// Created by whs31 on 19.09.23.
//

#include "googlemapsprovider.h"
#include <QtCore/QCoreApplication>
#include <QtCore/QDir>
#include <QtCore/QFile>

namespace CCL
{
  GoogleMapsProvider::GoogleMapsProvider(QObject* parent)
    : QObject(parent)
  {
    this->create();
  }

  QString GoogleMapsProvider::path() { return QCoreApplication::applicationDirPath() + "/config/osm"; }
  void GoogleMapsProvider::create() noexcept
  {
    QDir dir(path());
    if(not dir.exists())
      dir.mkpath(path());
    QFile::copy(":/osmconfigs/satellite", path() + "/satellite");
    QFile::copy(":/osmconfigs/terrain", path() + "/terrain");
    QFile::copy(":/osmconfigs/transit", path() + "/transit");
    QFile::copy(":/osmconfigs/cycle", path() + "/cycle");
    QFile::copy(":/osmconfigs/hiking", path() + "/hiking");
    QFile::copy(":/osmconfigs/night-transit", path() + "/night-transit");
    QFile::copy(":/osmconfigs/street", path() + "/street");
  }
} // CCL