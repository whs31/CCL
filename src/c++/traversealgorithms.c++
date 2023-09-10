/* ---------------------------------------------------------------------
 * CCL - Carthography Convenience Library
 * Copyright (C) 2023 radar-mms.
 *
 * uav.radar-mms.com/gitlab
 * ---------------------------------------------------------------------- */

#include "traversealgorithms.h"
#include <vector>
#include <stdexcept>
#include <cmath>
#include <QtCore/QPointF>
#include <QtCore/QLineF>
#include <QtCore/QRectF>
#include <QtGui/QPolygonF>
#include <QtPositioning/QGeoPath>
#include <QtPositioning/QGeoPolygon>
#include "CCL/Geomath"

#define in :

using std::vector;

namespace CCL::Traverse
{
  namespace Internal
  {
    QPointF rotateTraversePoint(const QPointF& point, const QPointF& origin, float angle) noexcept
    {
      return {((point.x() - origin.x()) * std::cos(M_PI / 180.0f * (-angle)))
              - ((point.y() - origin.y()) * std::sin(M_PI / 180.0f * (-angle))) + origin.x(),
          ((point.x() - origin.x()) * std::sin(M_PI / 180.0f * (-angle)))
          + ((point.y() - origin.y()) * std::cos(M_PI / 180.0f * (-angle))) + origin.y()};
    }

    float clampTraverseGridAngle(float angle) noexcept
    {
      if(angle > 90.0f)
        angle -= 180.0f;
      else if(angle < -90.0f)
        angle += 180.0f;
      return angle;
    }

    deque<QLineF> findIntersectionWithPolygon(const deque<QLineF>& lines, const QPolygonF& poly) noexcept
    {
      deque<QLineF> ret;

      for(const QLineF& line in lines)
      {
        deque<QPointF> intersections;
        for(int i = 0; i < poly.count() - 1; i++)
        {
          QPointF p;
          QLineF polygon_line = QLineF(poly[i], poly[i + 1]);
          if(line.intersects(polygon_line, &p) == QLineF::BoundedIntersection
             and not std::count(intersections.cbegin(), intersections.cend(), p))
            intersections.push_back(p);
        }

        if(intersections.size() > 1)
        {
          QPointF f, s;
          float cmd = 0;
          for(const QPointF& first in intersections)
          {
            for(const QPointF& second in intersections)
            {
              auto nmd = static_cast<float>(QLineF(first, second).length());
              if(nmd > cmd)
              {
                f = first;
                s = second;
                cmd = nmd;
              }
            }
          }
          ret.emplace_back(f, s);
        }
      }
      return ret;
    }

    deque<QLineF> adjustLineDirections(const deque<QLineF>& lines)
    {
      deque<QLineF> ret;
      float a = 0;
      bool first = true;
      for(const QLineF& line in lines)
      {
        QLineF adj;
        if(first)
        {
          a = static_cast<float>(line.angle());
          first = false;
        }
        if(std::abs(line.angle() - a) > 1.0)
        {
          adj.setP1(line.p2());
          adj.setP2(line.p1());
        }
        else
          adj = line;
        ret.push_back(adj);
      }
      return ret;
    }

    void adjustTransects(CoordinateArray2D& transects, Entry entry) noexcept
    {
      if(transects.empty())
        return;

      if(entry == Entry::BottomLeft or entry == Entry::BottomRight)
        reverseInternalTransectPoints(transects);
      if(entry == Entry::TopRight or entry == Entry::BottomRight)
        reverseTransectOrder(transects);
    }

    void reverseInternalTransectPoints(CoordinateArray2D& t) noexcept
    {
      for(auto& transect in t)
      {
        deque<QGeoCoordinate> q;
        for(int i = (int)transect.size() - 1; i >= 0; i--)
          q.push_back(transect[i]);
        transect = q;
      }
    }

    void reverseTransectOrder(CoordinateArray2D& t) noexcept { std::reverse(t.begin(), t.end()); }
  } // Internal

  QGeoPath buildTraverse(const QGeoPolygon& poly, float angle, float spacing, float turn_around, Entry entry)
  {
    if(poly.isEmpty())
      return {};

    if(spacing < 0.5f)
      throw std::invalid_argument("CCL.Traverse.build: spacing is too low (< 0.5)");

    QGeoPath ret;

    if(poly.size() < 3)
    {
      ret.addCoordinate(poly.path().front());
      ret.addCoordinate(poly.path().back());
    }
    else
    {
      deque<QPointF> poly_points;
      QGeoCoordinate tg_origin = poly.path().front();

      bool lf = false;
      for(const QGeoCoordinate& point in poly.path())
      {
        NEDPoint ned;
        if(lf)
          ned = geo2NED(point, tg_origin);
        else
          lf = true;
        poly_points.emplace_back(ned.x, ned.y);
      }

      float grid_angle = Internal::clampTraverseGridAngle(angle);
      QPolygonF polygon;
      for(const QPointF& point in poly_points)
        polygon << point;
      polygon << poly_points.front();

      QRectF bounding_rect = polygon.boundingRect();
      deque<QLineF> lines;
      float max_w = static_cast<float>(std::max(bounding_rect.width(), bounding_rect.height()) + 2000.0f);
      float transect_x = static_cast<float>(bounding_rect.center().x() - max_w / 2);
      while(transect_x < transect_x + max_w)
      {
        lines.emplace_back(Internal::rotateTraversePoint({transect_x, bounding_rect.center().y() - max_w / 2}, bounding_rect.center(), grid_angle),
                           Internal::rotateTraversePoint({transect_x, bounding_rect.center().y() + max_w / 2}, bounding_rect.center(), grid_angle));
        transect_x += spacing;
      }

      deque<QLineF> intersection_lines = Internal::findIntersectionWithPolygon(lines, polygon);
      if(intersection_lines.size() < 2)
      {
        lines.clear();
        lines.push_back(QLineF(lines.front()).translated(bounding_rect.center() - lines.front().pointAt(0.5)));
        intersection_lines = Internal::findIntersectionWithPolygon(lines, polygon);
      }

      deque<QLineF> result = Internal::adjustLineDirections(intersection_lines);
      CoordinateArray2D transects;
      for(const QLineF& line in result)
      {
        deque<QGeoCoordinate> transect;
        transect.emplace_back(ned2geo({static_cast<float>(line.p1().y()), static_cast<float>(line.p1().x()), 0},
                                      tg_origin));
        transect.emplace_back(ned2geo({static_cast<float>(line.p2().y()), static_cast<float>(line.p2().x()), 0},
                                      tg_origin));
        transects.push_back(transect);
      }

      Internal::adjustTransects(transects, entry);
      bool rv = false;
      for(CoordinateArray1D& transect in transects)
      {
        CoordinateArray1D transect_vertices = transect;
        if(rv)
        {
          rv = false;
          std::reverse(transect_vertices.begin(), transect_vertices.end());
        }
        else
          rv = true;
        transect = transect_vertices;
      }
      QList<QGeoCoordinate> result_path;
      for(CoordinateArray1D& transect in transects)
      {
        double tad = turn_around;
        if(tad != 0)
        {
          double azimuth = transect.front().azimuthTo(transect[1]);
          transect.front() = transect.front().atDistanceAndAzimuth(-tad, azimuth);
          transect.front().setAltitude(qQNaN());
          transect[1] = transect[1].atDistanceAndAzimuth(tad, azimuth);
          transect[1].setAltitude(qQNaN());
        }

        for(const QGeoCoordinate& coordinate in transect)
          result_path.push_back(coordinate);
      }

      ret.setPath(result_path);
    }

    return ret;
  }
} // CCL::Traverse