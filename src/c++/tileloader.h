/* ---------------------------------------------------------------------
 * CCL - Cartography Convenience Library
 * Copyright (C) 2023 radar-mms.
 *
 * uav.radar-mms.com/gitlab
 * ---------------------------------------------------------------------- */

#pragma once

#include <queue>
#include <QtCore/QObject>
#define invokable Q_INVOKABLE
#define slot Q_SLOT

using std::queue;

class QGeoPolygon;
class QNetworkAccessManager;
class QNetworkReply;

namespace CCL
{
  class TileLoader : public QObject
  {
    Q_OBJECT
    Q_PROPERTY(QString serverUrl READ serverUrl WRITE setServerUrl NOTIFY serverUrlChanged FINAL)
    Q_PROPERTY(QString storageUrl READ storageUrl WRITE setStorageUrl NOTIFY storageUrlChanged FINAL)
    Q_PROPERTY(int progress READ progress NOTIFY progressChanged STORED false FINAL);

    constexpr static const uint8_t MAX_ALLOWED_PARALLEL = 4;

    public:
      explicit TileLoader(QString storageUrl, QObject* parent = nullptr);

      [[nodiscard]] QString serverUrl() const;    void setServerUrl(const QString&);
      [[nodiscard]] QString storageUrl() const;   void setStorageUrl(const QString&);
      [[nodiscard]] int progress() const;

      void download(int zoom, int x, int y);
      void download(const QGeoPolygon&, int zoom = 18);
      invokable void download(const QList<QVariant>&, int zoom = 18);
      QByteArray tileAt(int zoom, int x, int y);

      [[nodiscard]] invokable static int estimate(const QList<QVariant>&, int zoom = 18);

    signals:
      void serverUrlChanged();
      void storageUrlChanged();
      void progressChanged();

    private:
      slot void onFinished(QNetworkReply* reply);
      void process();

      static uint32_t longitudeToTileX(double longitude, uint8_t zoom);
      static uint32_t latitudeToTileY(double latitude, uint8_t zoom);
      static double tileXToLongitude(uint32_t x, uint8_t zoom);
      static double tileYToLatitude(uint32_t y, uint8_t zoom);

    private:
      struct Tile
      {
        Tile(int zoom, int x, int y);

        int zoom;
        int x;
        int y;
      };

      QNetworkAccessManager* m_nam;
      long m_total_tiles;
      long m_loaded_tiles;
      queue<Tile> m_queue;
      int m_parallel_loaded_count;
      QString m_server_url;
      QString m_storage_url;
  };
} // CCL