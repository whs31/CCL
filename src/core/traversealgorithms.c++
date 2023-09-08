/* ---------------------------------------------------------------------
 * CCL - Carthography Convenience Library
 * Copyright (C) 2023 radar-mms.
 *
 * uav.radar-mms.com/gitlab
 * ---------------------------------------------------------------------- */

#include "traversealgorithms.h"
#include <deque>
#include <vector>
#include <QtCore/QPointF>
#include <QtPositioning/QGeoPath>
#include <QtPositioning/QGeoPolygon>

#define in :

using std::deque;
using std::vector;

namespace CCL::Traverse
{

  QGeoPath buildTraverse(const QGeoPolygon& poly, float angle, float spacing, float turn_around, Entry entry)
  {
    if(poly.isEmpty())
      return {};

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
        float x, y, d;
        auto vert = point;
        if(lf)
          convertGeoCoordinateToNED(vert, tg_origin, &y, &x, &d);
        else
        {
          lf = true;
          x = y = 0;
        }
      }
    }

      else {
        // Convert polygon to NED

        QList<QPointF> polygonPoints;
        QGeoCoordinate tangentOrigin = polygon.coordinateAt(0);
        //qInfo() << Q_FUNC_INFO << "tangentOrigin" << tangentOrigin;
        for (int i = 0; i < polygon.size(); i++) {
          qreal y, x, down;
          QGeoCoordinate vertex = polygon.coordinateAt(i);
          //qInfo() << Q_FUNC_INFO << vertex;
          if (i == 0) {
            // This avoids a nan calculation that comes out of convertGeoToNed
            x = y = 0;
          } else {
            convertGeoToNed(vertex, tangentOrigin, &y, &x, &down);
          }
          polygonPoints += QPointF(x, y);
          //qInfo() << Q_FUNC_INFO << polygonPoints.last().x() << polygonPoints.last().y();
        }

        // Generate transects

        qreal gridAngle = angle;
        qreal gridSpacing = spacing;
        if (gridSpacing < 0.5) {
          // We can't let gridSpacing get too small otherwise we will end up with too many transects.
          // So we limit to 0.5 meter spacing as min and set to huge value which will cause a single
          // transect to be added.
          gridSpacing = 100000;
        }

        gridAngle = clampGridAngle90(gridAngle);
        //gridAngle += refly ? 90 : 0;
        //qInfo() << Q_FUNC_INFO << "_rebuildTransectsPhase1 Clamped grid angle" << gridAngle;
        //qInfo() << Q_FUNC_INFO << "_rebuildTransectsPhase1 gridSpacing:gridAngle:refly" << gridSpacing << gridAngle/* << refly*/;

        // Convert polygon to bounding rect

        QPolygonF polygon;
        for (int i = 0; i < polygonPoints.count(); i++) {
          //qInfo() << Q_FUNC_INFO << "Vertex" << polygonPoints[i];
          polygon << polygonPoints[i];
        }
        polygon << polygonPoints[0];
        QRectF boundingRect = polygon.boundingRect();
        QPointF boundingCenter = boundingRect.center();
        //qInfo() << Q_FUNC_INFO << "Bounding rect" << boundingRect.topLeft().x() << boundingRect.topLeft().y() << boundingRect.bottomRight().x() << boundingRect.bottomRight().y();

        // Create set of rotated parallel lines within the expanded bounding rect. Make the lines larger than the
        // bounding box to guarantee intersection.

        QList<QLineF> lineList;

        // Transects are generated to be as long as the largest width/height of the bounding rect plus some fudge factor.
        // This way they will always be guaranteed to intersect with a polygon edge no matter what angle they are rotated to.
        // They are initially generated with the transects flowing from west to east and then points within the transect north to south.
        qreal maxWidth = qMax(boundingRect.width(), boundingRect.height()) + 2000.0;
        qreal halfWidth = maxWidth / 2.0;
        qreal transectX = boundingCenter.x() - halfWidth;
        qreal transectXMax = transectX + maxWidth;
        while (transectX < transectXMax) {
          qreal transectYTop = boundingCenter.y() - halfWidth;
          qreal transectYBottom = boundingCenter.y() + halfWidth;

          lineList += QLineF(rotatePoint(QPointF(transectX, transectYTop), boundingCenter, gridAngle), rotatePoint(QPointF(transectX, transectYBottom), boundingCenter, gridAngle));
          transectX += gridSpacing;
        }

        // Now intersect the lines with the polygon
        QList<QLineF> intersectLines;
        intersectLinesWithPolygon(lineList, polygon, intersectLines);

        // Less than two transects intersected with the polygon:
        //      Create a single transect which goes through the center of the polygon
        //      Intersect it with the polygon
        if (intersectLines.count() < 2) {
          QLineF firstLine = lineList.first();
          QPointF lineCenter = firstLine.pointAt(0.5);
          QPointF centerOffset = boundingCenter - lineCenter;
          firstLine.translate(centerOffset);
          lineList.clear();
          lineList.append(firstLine);
          intersectLines = lineList;
          intersectLinesWithPolygon(lineList, polygon, intersectLines);
        }

        // Make sure all lines are going the same direction. Polygon intersection leads to lines which
        // can be in varied directions depending on the order of the intesecting sides.
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

  }
} // CCL::Traverse