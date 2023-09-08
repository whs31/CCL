/* ---------------------------------------------------------------------
 * CCL - Carthography Convenience Library
 * Copyright (C) 2023 radar-mms.
 *
 * uav.radar-mms.com/gitlab
 * ---------------------------------------------------------------------- */

#pragma once

#include <deque>

using std::deque;

class QPointF;
class QPolygonF;
class QLineF;
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

  namespace Internal
  {
    QPointF rotateTraversePoint(const QPointF& point, const QPointF& origin, float angle) noexcept;
    float clampTraverseGridAngle(float angle) noexcept;
    deque<QLineF> findIntersectionWithPolygon(const deque<QLineF>& lines, const QPolygonF& poly) noexcept;
    deque<QLineF> adjustLineDirections(const deque<QLineF>& lines);
  } // Internal
} // CCL::Traverse