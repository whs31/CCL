#include "cclgeomath.h"
#include <QtCore/QPointF>
#include <QtCore/QDebug>
#include <QtPositioning/QGeoCoordinate>
#include <cmath>

__attribute__((constructor)) static void describe() { qInfo() << "<CCL> Library loaded. Version 0.1"; }

constexpr const double MAP_SCALE_RATIO = 156'543.03392;

double logf(double base, double value) noexcept { return (std::log(value) / std::log(base)); }

double CCL::mqiZoomLevel(double latitude, float meters_per_pixel) noexcept
{
    if(not meters_per_pixel)
        return logf(2, MAP_SCALE_RATIO * cos(latitude * M_PI / 180));
    return logf(2, MAP_SCALE_RATIO * cos(latitude * M_PI / 180) / meters_per_pixel);
}

QPointF CCL::geo2webmercator(const QGeoCoordinate& geo, uint8_t zoom) noexcept
{
    return QPointF((1.0 - asinh(tan(geo.latitude() * M_PI / 180.0)) / M_PI) / 2.0 * (1 << zoom),
                   (geo.longitude() + 180.0) / 360.0 * (1 << zoom));

}
