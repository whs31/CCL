/* ---------------------------------------------------------------------
 * CCL - Carthography Convenience Library
 * Copyright (C) 2023 radar-mms.
 *
 * uav.radar-mms.com/gitlab
 * ---------------------------------------------------------------------- */

#pragma once

#include <global/cclglobal.h>
#include <QtCore/QVariantList>
#include <QtPositioning/QGeoCoordinate>

namespace CCL
{
    class CCL_EXPORT Orthodrom
    {
        public:
            Orthodrom(const QGeoCoordinate& first, const QGeoCoordinate& second);
            Orthodrom();

            auto get() const -> QList<QVariant>;
            void set(const QGeoCoordinate& first, const QGeoCoordinate& second);

            double latitudeAt(double longitude);

        private:
            void distribute(uint16_t spacing);
            double distance() const;

        private:
            double a1, a2;
            QGeoCoordinate first, second;
            QList<QVariant> path;
    };
} // CCL
