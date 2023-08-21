/* ---------------------------------------------------------------------
 * CCL - Carthography Convenience Library
 * Copyright (C) 2023 radar-mms.
 *
 * uav.radar-mms.com/gitlab
 * ---------------------------------------------------------------------- */

#pragma once

#include <QtCore/QVariantList>
#include <QtPositioning/QGeoCoordinate>

namespace CCL
{
    class Orthodrom
    {
        public:
            Orthodrom(const QGeoCoordinate& first, const QGeoCoordinate& second);
            Orthodrom();

            auto get() const noexcept -> QList<QVariant>;
            void set(const QGeoCoordinate& first, const QGeoCoordinate& second) noexcept;

            double latitudeAt(double longitude) const noexcept;

        private:
            void distribute(uint16_t spacing) noexcept;
            double distance() const noexcept;

        private:
            double a1, a2;
            QGeoCoordinate first, second;
            QList<QVariant> path;
    };
} // CCL
