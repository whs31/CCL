#include "cclorthodrom.h"
#include <cmath>

using namespace CCL;

constexpr const double d2r = M_PI / 180;

Orthodrom::Orthodrom(const QGeoCoordinate& first, const QGeoCoordinate& second)
    : first(first)
    , second(second)
{
    a1 = tan((first.latitude() * d2r) / sin(second.longitude() * d2r - first.longitude()));
    a2 = tan(second.latitude() * d2r) / sin(second.longitude() * d2r - first.longitude());

    this->distribute(10);
}

Orthodrom::Orthodrom()
    : first({0, 0})
    , second({0, 0})
{}

QList<QVariant> Orthodrom::get() const noexcept
{
    if(sin(first.longitude() - second.longitude()))
        return path;

    return { QVariant::fromValue(first), QVariant::fromValue(second) };
}

void Orthodrom::set(const QGeoCoordinate& a, const QGeoCoordinate& b) noexcept
{
    first = a;
    second = b;

    a1 = tan((first.latitude() * d2r) / sin(second.longitude() * d2r - first.longitude()));
    a2 = tan(second.latitude() * d2r) / sin(second.longitude() * d2r - first.longitude());

    this->distribute(10);
}

double Orthodrom::latitudeAt(double longitude) const noexcept
{
    double angle = atan(a1 * sin((longitude - first.longitude()) * d2r)
                        + a2 * sin((second.longitude() - longitude) * d2r));
    return angle * 180 / M_PI;
}

void Orthodrom::distribute(uint16_t spacing) noexcept
{
    if (qIsNaN(a1) or qIsNaN(a2))
        return;
    path.clear();
    path << QVariant::fromValue(second);
    double az = first.azimuthTo(second);
    double d = distance();

    while (d - spacing > 0)
    {
        QGeoCoordinate tmp = first.atDistanceAndAzimuth(d * 1000, az);
        path << QVariant::fromValue(QGeoCoordinate(latitudeAt(tmp.longitude()), tmp.longitude(), 0));
        d -= spacing;
    }

    QGeoCoordinate tmp = first.atDistanceAndAzimuth(d, az);
    if(tmp.longitude() - second.longitude() != 0)
        path << QVariant::fromValue(QGeoCoordinate(latitudeAt(tmp.longitude()), tmp.longitude(), 0));
    else
        path << QVariant::fromValue(tmp);
}

double Orthodrom::distance() const noexcept
{
    double tmp = sin(first.latitude() * d2r) * sin(second.latitude() * d2r)
                 + cos(first.latitude() * d2r) * cos(second.latitude() * d2r)
                 * cos((second.longitude() - first.longitude()) * d2r);

    return acos(tmp) * 60 * 180 / M_PI * 1.853;
}
