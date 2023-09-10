/* ---------------------------------------------------------------------
 * CCL - Cartography Convenience Library
 * Copyright (C) 2023 radar-mms.
 *
 * uav.radar-mms.com/gitlab
 * ---------------------------------------------------------------------- */

#include "tileloader.h"
#include <cmath>
#include <stdexcept>
#include <QtCore/QCoreApplication>
#include <QtCore/QDir>
#include <QtGui/QPolygonF>
#include <QtNetwork/QNetworkReply>
#include <QtPositioning/QGeoPolygon>
#include <QtPositioning/QGeoRectangle>
#define in :

namespace CCL
{
  TileLoader::Tile::Tile(int zoom, int x, int y)
    : zoom(zoom)
    , x(x)
    , y(y)
  {}

  TileLoader::TileLoader(QString storageUrl, QObject* parent)
    : QObject(parent)
    , m_nam(new QNetworkAccessManager(this))
    , m_total_tiles(0)
    , m_loaded_tiles(0)
    , m_parallel_loaded_count(0)
    , m_server_url("https://mt.google.com/vt/lyrs=y&hl=ru&z=%1&x=%2&y=%3")
    , m_storage_url(std::move(storageUrl))
  {
    connect(m_nam, &QNetworkAccessManager::finished, this, &TileLoader::onFinished);
  }

  void TileLoader::download(int zoom, int x, int y)
  {
    m_queue.emplace(zoom, x, y);
    m_total_tiles++;

    if(m_parallel_loaded_count < MAX_ALLOWED_PARALLEL)
      this->process();
  }

  void TileLoader::download(const QGeoPolygon& polygon, int zoom)
  {
    if(zoom > 20 or zoom < 0)
      throw std::invalid_argument("CCL.TileLoader.download: Maximum zoom must be in range [0 - 20]");

    QGeoRectangle rectangle = polygon.boundingGeoRectangle();
    double minLatitude = rectangle.bottomLeft().latitude();
    double maxLatitude = rectangle.topRight().latitude();
    double minLongitude = rectangle.bottomLeft().longitude();
    double maxLongitude = rectangle.topRight().longitude();

    if(minLongitude > maxLongitude)
    {
      minLongitude = rectangle.topRight().longitude();
      maxLongitude = rectangle.bottomLeft().longitude();
    }

    QPolygonF polygon2;
    for(int index = 0; index < polygon.size(); index++)
    {
      QGeoCoordinate point = polygon.coordinateAt(index);
      polygon2.append(QPointF(point.longitude(), point.latitude()));
    }

    for(int z = 0; z <= zoom; z++)
    {
      uint32_t minX = longitudeToTileX(minLongitude, z);
      uint32_t maxX = longitudeToTileX(maxLongitude, z);
      uint32_t minY = latitudeToTileY(maxLatitude, z);
      uint32_t maxY = latitudeToTileY(minLatitude, z);

      for(uint32_t x = minX; x <= maxX; x++)
      {
        for(uint32_t y = minY; y <= maxY; y++)
        {
          double x0 = tileXToLongitude(x, z);
          double y0 = tileYToLatitude(y, z);
          double x1 = tileXToLongitude(x + 1, z);
          double y1 = tileYToLatitude(y + 1, z);

          if(polygon2.intersects(QPolygonF(QRectF(QPointF(x0, y0), QPointF(x1, y1)))))
            this->download(z, (int)x, (int)y);
        }
      }
    }
  }

  void TileLoader::download(const QList<QVariant>& list, int zoom)
  {
    QGeoPolygon pass;
    for(const auto& point in list)
      pass.addCoordinate(point.value<QGeoCoordinate>());
    this->download(pass, zoom);
  }

  int TileLoader::estimate(const QList<QVariant>& list, int zoom_max)
  {
    QGeoPolygon polygon;
    for(const auto& point in list)
      polygon.addCoordinate(point.value<QGeoCoordinate>());

    int ret = 0;
    QGeoRectangle rectangle = polygon.boundingGeoRectangle();
    double minLatitude = rectangle.bottomLeft().latitude();
    double maxLatitude = rectangle.topRight().latitude();
    double minLongitude = rectangle.bottomLeft().longitude();
    double maxLongitude = rectangle.topRight().longitude();

    if(minLongitude > maxLongitude)
    {
      minLongitude = rectangle.topRight().longitude();
      maxLongitude = rectangle.bottomLeft().longitude();
    }

    QPolygonF polygon2;
    for(int index = 0; index < polygon.size(); index++)
    {
      QGeoCoordinate point = polygon.coordinateAt(index);
      polygon2.append(QPointF(point.longitude(), point.latitude()));
    }

    for(int zoom = 0; zoom <= zoom_max; zoom++)
    {
      uint32_t minX = longitudeToTileX(minLongitude, zoom);
      uint32_t maxX = longitudeToTileX(maxLongitude, zoom);
      uint32_t minY = latitudeToTileY(maxLatitude, zoom);
      uint32_t maxY = latitudeToTileY(minLatitude, zoom);

      for(uint32_t x = minX; x <= maxX; x++)
      {
        for(uint32_t y = minY; y <= maxY; y++)
        {
          double x0 = tileXToLongitude(x, zoom);
          double y0 = tileYToLatitude(y, zoom);
          double x1 = tileXToLongitude(x + 1, zoom);
          double y1 = tileYToLatitude(y + 1, zoom);

          if(polygon2.intersects(QPolygonF(QRectF(QPointF(x0, y0), QPointF(x1, y1)))))
            ret++;
        }
      }
    }

    return ret;
  }

  QByteArray TileLoader::tileAt(int zoom, int x, int y)
  {
    QByteArray ret;
    QFile tileFile(storageUrl() + QString("/%1/%2/%3").arg(zoom).arg(x).arg(y));
    if(tileFile.exists())
    {
      tileFile.open(QIODevice::ReadOnly);
      ret = tileFile.readAll();
      tileFile.close();
    }

    return ret;
  }

  void TileLoader::onFinished(QNetworkReply* reply)
  {
    if(reply->error() == QNetworkReply::NoError)
    {
      QDir tilePath(storageUrl() + reply->property("filePath").toString());
      if(not tilePath.exists())
        tilePath.mkpath(".");

      QFile tileFile(tilePath.filePath(reply->property("fileName").toString()));
      tileFile.open(QIODevice::WriteOnly);
      tileFile.write(reply->readAll());
      tileFile.close();
    }

    reply->deleteLater();
    m_parallel_loaded_count--;
    m_loaded_tiles++;

    emit progressChanged();

    if(not m_queue.empty())
      this->process();

    if(m_total_tiles == m_loaded_tiles)
    {
      m_total_tiles = 0;
      m_loaded_tiles = 0;
    }
  }

  void TileLoader::process()
  {
    Tile tileItem = m_queue.front();
    m_queue.pop();
    QNetworkReply* reply = m_nam->get(QNetworkRequest(serverUrl().arg(tileItem.zoom).arg(tileItem.x).arg(tileItem.y)));
    reply->setProperty("filePath", QString("/%1/%2").arg(tileItem.zoom).arg(tileItem.x));
    reply->setProperty("fileName", QString("%1").arg(tileItem.y));

    m_parallel_loaded_count++;
  }

  uint32_t TileLoader::longitudeToTileX(double longitude, uint8_t zoom) { return static_cast<uint32_t>((longitude + 180.0) / 360.0 * (1 << zoom)); }
  uint32_t TileLoader::latitudeToTileY(double latitude, uint8_t zoom)
  {
    return static_cast<uint32_t>((1.0 - asinh(tan(latitude * M_PI / 180.0)) / M_PI) / 2.0 * (1 << zoom));
  }
  double TileLoader::tileXToLongitude(uint32_t x, uint8_t zoom) { return x / qreal(1 << zoom) * 360.0 - 180; }
  double TileLoader::tileYToLatitude(uint32_t y, uint8_t zoom)
  {
    double n = M_PI * (1 - 2.0 * y / double(1 << zoom));
    return 180.0 / M_PI * atan(0.5 * (exp(n) - exp(-n)));
  }

  QString TileLoader::serverUrl() const { return m_server_url; }
  void TileLoader::setServerUrl(const QString& x) {
    if(x == m_server_url)
      return;
    m_server_url = x;
    emit serverUrlChanged();
  }

  QString TileLoader::storageUrl() const { return m_storage_url; }
  void TileLoader::setStorageUrl(const QString& x) {
    if(x == m_storage_url)
      return;
    m_storage_url = x;
    emit storageUrlChanged();
  }

  int TileLoader::progress() const { return (m_loaded_tiles * 100 / m_total_tiles); }
} // CCL