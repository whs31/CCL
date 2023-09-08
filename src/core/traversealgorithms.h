/* ---------------------------------------------------------------------
 * CCL - Carthography Convenience Library
 * Copyright (C) 2023 radar-mms.
 *
 * uav.radar-mms.com/gitlab
 * ---------------------------------------------------------------------- */

#pragma once

#include <deque>

class QPointF;
class QPolygonF;
class QLineF;
class QGeoPath;
class QGeoPolygon;
class QGeoCoordinate;

using std::deque;
using CoordinateArray1D = deque<QGeoCoordinate>;
using CoordinateArray2D = deque<CoordinateArray1D>;

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

  namespace Internal
  {
    QPointF rotateTraversePoint(const QPointF& point, const QPointF& origin, float angle) noexcept;
    float clampTraverseGridAngle(float angle) noexcept;
    deque<QLineF> findIntersectionWithPolygon(const deque<QLineF>& lines, const QPolygonF& poly) noexcept;
    deque<QLineF> adjustLineDirections(const deque<QLineF>& lines);
    void adjustTransects(CoordinateArray2D& transects, Entry entry) noexcept;
    void reverseInternalTransectPoints(CoordinateArray2D& t) noexcept;
    void reverseTransectOrder(CoordinateArray2D& t) noexcept;
  } // Internal
} // CCL::Traverse