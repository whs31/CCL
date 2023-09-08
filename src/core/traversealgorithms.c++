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
      for(const QLineF& line in result)
      {
        QGeoCoordinate coord;
        deque<QGeoCoordinate> transect;
        transect.emplace_back(geo2NED())
      }
    }


        QList<QLineF> resultLines;
        adjustLineDirection(intersectLines, resultLines);

        // Convert from NED to Geo
        QList<QList<QGeoCoordinate>> transects;
        for (const QLineF& line : resultLines) {
          QGeoCoordinate          coord;
          QList<QGeoCoordinate>   transect;

          convertNedToGeo(line.p1().y(), line.p1().x(), 0, tangentOrigin, &coord);
          transect.append(coord);
          convertNedToGeo(line.p2().y(), line.p2().x(), 0, tangentOrigin, &coord);
          transect.append(coord);

          transects.append(transect);
        }

        adjustTransectsToEntryPointLocation(transects, entryPoint);

        //                if (refly) {
        //                    _optimizeTransectsForShortestDistance(_transects.last().last().coord, transects);
        //                }

        //                if (_flyAlternateTransectsFact.rawValue().toBool()) {
        //                    QList<QList<QGeoCoordinate>> alternatingTransects;
        //                    for (int i=0; i<transects.count(); i++) {
        //                        if (!(i & 1)) {
        //                            alternatingTransects.append(transects[i]);
        //                        }
        //                    }
        //                    for (int i=transects.count()-1; i>0; i--) {
        //                        if (i & 1) {
        //                            alternatingTransects.append(transects[i]);
        //                        }
        //                    }
        //                    transects = alternatingTransects;
        //                }

        // Adjust to lawnmower pattern
        bool reverseVertices = false;
        for (int i=0; i<transects.count(); i++) {
          // We must reverse the vertices for every other transect in order to make a lawnmower pattern
          QList<QGeoCoordinate> transectVertices = transects[i];
          if (reverseVertices) {
            reverseVertices = false;
            QList<QGeoCoordinate> reversedVertices;
            for (int j=transectVertices.count()-1; j>=0; j--) {
              reversedVertices.append(transectVertices[j]);
            }
            transectVertices = reversedVertices;
          } else {
            reverseVertices = true;
          }
          transects[i] = transectVertices;
        }

        // Convert to CoordInfo transects and append to _transects
        QList<QGeoCoordinate> resultPath;
        for (QList<QGeoCoordinate>& transect : transects) {
          //                    QGeoCoordinate                                  coord;
          //                    QList<TransectStyleComplexItem::CoordInfo_t>    coordInfoTransect;
          //                    TransectStyleComplexItem::CoordInfo_t           coordInfo;

          //                    coordInfo = { transect[0], CoordTypeSurveyEntry };
          //                    coordInfoTransect.append(coordInfo);
          //                    coordInfo = { transect[1], CoordTypeSurveyExit };
          //                    coordInfoTransect.append(coordInfo);

          //                    // For hover and capture we need points for each camera location within the transect
          //                    if (triggerCamera() && hoverAndCaptureEnabled()) {
          //                        double transectLength = transect[0].distanceTo(transect[1]);
          //                        double transectAzimuth = transect[0].azimuthTo(transect[1]);
          //                        if (triggerDistance() < transectLength) {
          //                            int cInnerHoverPoints = static_cast<int>(floor(transectLength / triggerDistance()));
          //                            qCDebug(SurveyComplexItemLog) << "cInnerHoverPoints" << cInnerHoverPoints;
          //                            for (int i=0; i<cInnerHoverPoints; i++) {
          //                                QGeoCoordinate hoverCoord = transect[0].atDistanceAndAzimuth(triggerDistance() * (i + 1), transectAzimuth);
          //                                TransectStyleComplexItem::CoordInfo_t coordInfo = { hoverCoord, CoordTypeInteriorHoverTrigger };
          //                                coordInfoTransect.insert(1 + i, coordInfo);
          //                            }
          //                        }
          //                    }

          // Extend the transect ends for turnaround
          qreal turnAroundDistance = turnAround;
          if (turnAroundDistance != 0) {
            qreal azimuth = transect[0].azimuthTo(transect[1]);
            transect[0] = transect[0].atDistanceAndAzimuth(-turnAroundDistance, azimuth);
            transect[0].setAltitude(qQNaN());
            transect[1] = transect[1].atDistanceAndAzimuth(turnAroundDistance, azimuth);
            transect[1].setAltitude(qQNaN());
          }

          //                    _transects.append(coordInfoTransect);

          for(auto point : transect)
            resultPath.append(point);
        }

        result.setPath(resultPath);

  }
} // CCL::Traverse