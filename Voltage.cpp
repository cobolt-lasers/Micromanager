/**
 * \file        Voltage.cpp
 *
 * \authors     Lukas Kalinski
 *
 * \copyright   Cobolt AB, 2018. All rights reserved.
 */

#include "Voltage.h"

NAMESPACE_COBOLT_BEGIN

Voltage operator* ( const Voltage& c, float factor )
{
    return Voltage::mV( ( Voltage::millivolts_t )( (float) c.raw() * factor ) );
}

Voltage operator* ( float factor, const Voltage& c )
{
    return ( c * factor );
}

Voltage operator- ( const Voltage& minuend, const Voltage& subtrahend )
{
    return Voltage::mV( minuend.mV() - subtrahend.mV() );
}

Voltage operator+ ( const Voltage& term1, const Voltage& term2 )
{
    return Voltage::mV( term1.mV() + term2.mV() );
}

NAMESPACE_COBOLT_END
