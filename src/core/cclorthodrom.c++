#include "cclorthodrom.h"
#include <cmath>
#include <QtMath>

using namespace CCL;

constexpr const double d2r = M_PI / 180;

Orthodrom::Orthodrom(const QGeoCoordinate& first, const QGeoCoordinate& second)
    : first(first)
    , second(second)
{
    a1 = qTan(qDegreesToRadians(first.latitude())) / qSin(qDegreesToRadians(second.longitude() - first.longitude()));
    a2 = qTan(qDegreesToRadians(second.latitude())) / qSin(qDegreesToRadians(second.longitude() - first.longitude()));

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

    a1 = qTan(qDegreesToRadians(first.latitude())) / qSin(qDegreesToRadians(second.longitude() - first.longitude()));
    a2 = qTan(qDegreesToRadians(second.latitude())) / qSin(qDegreesToRadians(second.longitude() - first.longitude()));

    this->distribute(10);
}

double Orthodrom::latitudeAt(double longitude) const noexcept
{
    double angle = qAtan(a2 * qSin(qDegreesToRadians(longitude - first.longitude()))
                         + a1 * qSin(qDegreesToRadians(second.longitude() - longitude)));

    return qRadiansToDegrees(angle);
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
    double tmp = qSin(qDegreesToRadians(first.latitude())) * qSin(qDegreesToRadians(second.latitude())) +
                 qCos(qDegreesToRadians(first.latitude())) * qCos(qDegreesToRadians(second.latitude()))
                     * qCos(qDegreesToRadians(second.longitude() - first.longitude()));
    double angle = qRadiansToDegrees(qAcos(tmp)) * 60;

    return angle * 1.853;
}
