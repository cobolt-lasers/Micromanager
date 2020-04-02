/**
 * \file        types.cpp
 *
 * \authors     Lukas Kalinski
 *
 * \copyright   Cobolt AB, 2020. All rights reserved. // TODO: Set proper license
 */

#include "types.h"

NAMESPACE_COBOLT_BEGIN

namespace value
{
    namespace analog_impedance
    {
        StringValueMap high = { "0", "1 kOhm" };
        StringValueMap low = { "1", "50 Ohm" };

        const StringValueMap* values[] = { &high, &low, NULL };
    }

    namespace flag
    {
        StringValueMap enable = { "1", "Enable" };
        StringValueMap disable = { "0", "Disable" };

        const StringValueMap* values[] = { &enable, &disable, NULL };
    }

    namespace toggle
    {
        StringValueMap on = { "1", "On" };
        StringValueMap off = { "0", "Off" };

        const StringValueMap* values[] = { &on, &off, NULL };
    }
}

NAMESPACE_COBOLT_END
