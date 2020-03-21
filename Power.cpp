/**
 * \file        Power.cpp
 *
 * \authors     Lukas Kalinski
 *
 * \copyright   Cobolt AB, 2019. All rights reserved.
 */

#include "Power.h"

NAMESPACE_COBOLT_BEGIN

Power operator* ( const Power& c, float factor )
{
    return Power::mW( ( Power::milliwatts_t )( (float) c.mW() * factor ) );
}

Power operator* ( float factor, const Power& c )
{
    return ( c * factor );
}

Power operator- ( const Power& minuend, const Power& subtrahend )
{
    return Power::mW( minuend.mW() - subtrahend.mW() );
}

Power operator+ ( const Power& term1, const Power& term2 )
{
    return Power::mW( term1.mW() + term2.mW() );
}

NAMESPACE_COBOLT_END
