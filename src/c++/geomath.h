/* ---------------------------------------------------------------------
 * CCL - Carthography Convenience Library
 * Copyright (C) 2023 radar-mms.
 *
 * uav.radar-mms.com/gitlab
 * ---------------------------------------------------------------------- */

#pragma once

#include <cstdint>

class QGeoCoordinate;
class QPointF;

namespace CCL
{
    struct NEDPoint
    {
      NEDPoint();
      NEDPoint(float x, float y, float z);

      float x;
      float y;
      float z;
    };

    double mqiZoomLevel(double latitude, float meters_per_pixel = 1) noexcept;
    QPointF geo2webmercator(const QGeoCoordinate& geo, uint8_t zoom = 19) noexcept;
    NEDPoint geo2NED(const QGeoCoordinate& coord, const QGeoCoordinate& origin) noexcept;
    QGeoCoordinate ned2geo(const NEDPoint& ned, const QGeoCoordinate& origin) noexcept;
} // CCL
