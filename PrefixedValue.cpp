/**
 * \file        PrefixedValue.cpp
 *
 * \authors     Lukas Kalinski
 *
 * \copyright   Cobolt AB, 2019. All rights reserved.
 */

#include "PrefixedValue.h"

NAMESPACE_COBOLT_BEGIN

bool operator == ( const PrefixedValue& c1, const PrefixedValue& c2 )
{
    return ( c1.raw() == c2.raw() );
}

bool operator != ( const PrefixedValue& c1, const PrefixedValue& c2 )
{
    return ( c1.raw() != c2.raw() );
}

bool operator < ( const PrefixedValue& c1, const PrefixedValue& c2 )
{
    return ( c1.raw() < c2.raw() );
}

bool operator > ( const PrefixedValue& c1, const PrefixedValue& c2 )
{
    return ( c1.raw() > c2.raw() );
}

bool operator >= ( const PrefixedValue& c1, const PrefixedValue& c2 )
{
    return ( c1.raw() >= c2.raw() );
}

bool operator <= ( const PrefixedValue& c1, const PrefixedValue& c2 )
{
    return ( c1.raw() <= c2.raw() );
}

NAMESPACE_COBOLT_END
