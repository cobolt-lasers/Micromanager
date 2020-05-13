/**
 * \file        base.h
 *
 * \authors     Lukas Kalinski
 *
 * \copyright   Cobolt AB, 2020. All rights reserved. // TODO: Set proper license
 */

#ifndef __BASE_H
#define __BASE_H

#include <vector>
#define NAMESPACE_COBOLT_BEGIN namespace cobolt {
#define NAMESPACE_COBOLT_END   }

#define NAMESPACE_COBOLT_COMPATIBILITY_BEGIN( NS ) namespace cobolt { namespace compatibility { namespace NS {
#define NAMESPACE_COBOLT_COMPATIBILITY_END         }}}

NAMESPACE_COBOLT_BEGIN

namespace return_code
{
    const int ok = 0;
    const int error = 1;
    const int invalid_property_value = 3;
    const int unsupported_command = 11;
    const int illegal_port_change = 101001;
    const int serial_port_undefined = 101002;
    const int laser_startup_incomplete = 101003;
    const int invalid_value = 101004;
    const int property_not_settable_in_current_state = 101005;
    const int unsupported_device_property_value = 101006;
}

#define COBOLT_MM_DRIVER_VERSION "0.9.1b"

NAMESPACE_COBOLT_END

#include "Logger.h"

#endif // #ifndef __BASE_H