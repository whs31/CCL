/* ---------------------------------------------------------------------
 * CCL - Cartography Convenience Library
 * Copyright (C) 2023 radar-mms.
 *
 * uav.radar-mms.com/gitlab
 * ---------------------------------------------------------------------- */

#include "register.h"
#include <QtQml/qqml.h>
#include "CCL/TileLoader"

namespace CCL
{
  void registerQml() noexcept
  {
    qmlRegisterModule("CCL.Tiles", 1, 0);
    
  }
} // CCL