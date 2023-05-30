/* ---------------------------------------------------------------------
 * CCL - Carthography Convenience Library
 * Copyright (C) 2023 radar-mms.
 *
 * uav.radar-mms.com/gitlab
 * ---------------------------------------------------------------------- */

#pragma once

#include <QtCore/QtGlobal>

#if defined CCL_LIBRARY && !defined CCL_STATIC
    #define CCL_EXPORT Q_DECL_EXPORT
#elif !defined LPVL_LIBRARY && !defined CCL_STATIC
    #define CCL_EXPORT Q_DECL_IMPORT
#else
    #define CCL_EXPORT
#endif