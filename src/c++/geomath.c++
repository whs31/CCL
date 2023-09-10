#include "geomath.h"
#include <cmath>
#include <QtCore/QPointF>
#include <QtCore/QDebug>
#include <QtPositioning/QGeoCoordinate>

__attribute__((constructor)) static void describe() { qInfo() << "<CCL> Library loaded. Version 0.1"; }

constexpr const double MAP_SCALE_RATIO = 156'543.03392;
constexpr const float EARTH_RADIUS = 6371000;

constexpr const float TO_RADIANS = M_PI / 180.0f;
constexpr const float TO_DEGREES = 180.0f / M_PI;

double logf(double base, double value) noexcept { return (std::log(value) / std::log(base)); }

namespace CCL
{
  NEDPoint::NEDPoint()
    : x(0)
    , y(0)
    , z(0)
  {}

  NEDPoint::NEDPoint(float x, float y, float z)
    : x(x)
    , y(y)
    , z(z)
  {}

  double mqiZoomLevel(double latitude, float meters_per_pixel) noexcept
  {
    if(not meters_per_pixel)
      return logf(2, MAP_SCALE_RATIO * cos(latitude * M_PI / 180));
    return logf(2, MAP_SCALE_RATIO * cos(latitude * M_PI / 180) / meters_per_pixel);
  }

  QPointF geo2webmercator(const QGeoCoordinate& geo, uint8_t zoom) noexcept
  {
    return QPointF((1.0 - asinh(tan(geo.latitude() * M_PI / 180.0)) / M_PI) / 2.0 * (1 << zoom),
                   (geo.longitude() + 180.0) / 360.0 * (1 << zoom));
  }

  NEDPoint geo2NED(const QGeoCoordinate& coord, const QGeoCoordinate& origin) noexcept
  {
    if(coord == origin)
      return {};

    double sin_lat = std::sin(TO_RADIANS * coord.latitude());
    double cos_lat = std::cos(TO_RADIANS * coord.latitude());
    double cos_d_lon = std::cos(TO_RADIANS * coord.longitude() - TO_RADIANS * origin.longitude());
    double ref_sin_lat = std::sin(TO_RADIANS * origin.latitude());
    double ref_cos_lat = std::cos(TO_RADIANS * origin.latitude());

    double c = std::acos(ref_sin_lat * sin_lat + ref_cos_lat * cos_lat * cos_d_lon);
    double k = (std::fabs(c) < std::numeric_limits<double>::epsilon()) ? 1.0 : (c / sin(c));

    return { static_cast<float>(k * (ref_cos_lat * sin_lat - ref_sin_lat * cos_lat * cos_d_lon) * EARTH_RADIUS),
             static_cast<float>(k * cos_lat * sin(TO_RADIANS * coord.longitude() - TO_RADIANS * origin.longitude()) * EARTH_RADIUS),
             static_cast<float>(-(coord.altitude() - origin.altitude())) };
  }

  QGeoCoordinate ned2geo(const NEDPoint& ned, const QGeoCoordinate& origin) noexcept
  {
    double c = std::hypot(ned.x / EARTH_RADIUS, ned.y / EARTH_RADIUS);
    double ref_sin_lat = std::sin(origin.latitude() * TO_RADIANS);
    double ref_cos_lat = std::cos(origin.latitude() * TO_RADIANS);
    double lat_rad, lon_rad;

    if(std::fabs(c) > std::numeric_limits<double>::epsilon())
    {
      lat_rad = asin(std::cos(c) * ref_sin_lat + (ned.x / EARTH_RADIUS * std::sin(c) * ref_cos_lat) / c);
      lon_rad = (origin.longitude() * TO_RADIANS + atan2(ned.y / EARTH_RADIUS * std::sin(c),
                                                         c * ref_cos_lat * std::cos(c) - ned.x / EARTH_RADIUS * ref_sin_lat * std::sin(c)));
    }
    else
    {
      lat_rad = origin.latitude() * TO_RADIANS;
      lon_rad = origin.longitude() * TO_RADIANS;
    }
    return { TO_DEGREES * lat_rad, TO_DEGREES * lon_rad, -ned.z + origin.altitude() };
  }
} // CCL
