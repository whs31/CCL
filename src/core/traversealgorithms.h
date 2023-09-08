/* ---------------------------------------------------------------------
 * CCL - Carthography Convenience Library
 * Copyright (C) 2023 radar-mms.
 *
 * uav.radar-mms.com/gitlab
 * ---------------------------------------------------------------------- */

#pragma once

class QGeoPath;
class QGeoPolygon;

namespace CCL::Traverse
{
  enum class Entry
  {
    First,
    TopLeft = First,
    TopRight,
    BottomLeft,
    BottomRight,
    Last = BottomRight
  };

  QGeoPath buildTraverse(const QGeoPolygon& poly, float angle, float spacing, float turn_around, Entry entry);
} // CCL::Traverse