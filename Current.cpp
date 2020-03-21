/**
 * \file        Current.cpp
 *
 * \authors     Lukas Kalinski
 *
 * \copyright   Cobolt AB, 2019. All rights reserved.
 */

#include "Current.h"

NAMESPACE_COBOLT_BEGIN

Current operator* ( const Current& c, float factor )
{
    return Current::A( (float) c.raw() * factor );
}

Current operator* ( float factor, const Current& c )
{
    return ( c * factor );
}

Current operator- ( const Current& minuend, const Current& subtrahend )
{
    return Current::mA( minuend.mA() - subtrahend.mA() );
}

Current operator+ ( const Current& term1, const Current& term2 )
{
    return Current::mA( term1.mA() + term2.mA() );
}

NAMESPACE_COBOLT_END