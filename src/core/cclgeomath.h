/* ---------------------------------------------------------------------
 * CCL - Carthography Convenience Library
 * Copyright (C) 2023 radar-mms.
 *
 * uav.radar-mms.com/gitlab
 * ---------------------------------------------------------------------- */

#pragma once

#include <global/cclglobal.h>

class QGeoCoordinate;
class QPointF;

namespace CCL
{
    CCL_EXPORT double mqiZoomLevel(double latitude, float meters_per_pixel = 1) noexcept;
    CCL_EXPORT QPointF geo2webmercator(const QGeoCoordinate& geo, uint8_t zoom = 19) noexcept;
} // CCL
