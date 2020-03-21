/**
 * \file        Power.h
 *
 * \authors     Lukas Kalinski
 *
 * \copyright   Cobolt AB, 2019. All rights reserved.
 */

#ifndef __COBOLT__POWER_H
#define __COBOLT__POWER_H

#include "PrefixedValue.h"

NAMESPACE_COBOLT_BEGIN

class Power : public PrefixedValue
{
public:

    typedef PrefixedValue Parent;

    typedef typename PrefixedValue::raw_t milliwatts_t;
    typedef typename PrefixedValue::raw_t watts_t;

    static Power mW( milliwatts_t milliwatts )
    {
        return Power( milliwatts );
    }

    static Power W( watts_t watts )
    {
        return Power( (milliwatts_t) ( watts * 1000.0f ) );
    }

    static Power zero()
    {
        return Power( 0 );
    }

    Power() : PrefixedValue() {}

    inline watts_t W() const
    {
        return PrefixedValue::raw();
    }

    inline milliwatts_t mW() const
    {
        return PrefixedValue::rawAs( Milli );
    }

    friend Power operator* ( const Power& c, float factor );
    friend Power operator* ( float factor, const Power& c );

    friend Power operator- ( const Power& minuend, const Power& subtrahend );
    friend Power operator+ ( const Power& term1, const Power& term2 );

private:

    Power( milliwatts_t milliwatts ) :
        Parent( milliwatts, Parent::Milli )
    {
    }
};

Power operator* ( const Power& c, float factor );
Power operator* ( float factor, const Power& c );
Power operator- ( const Power& minuend, const Power& subtrahend );
Power operator+ ( const Power& term1, const Power& term2 );

NAMESPACE_COBOLT_END

#endif // #ifndef __COBOLT__POWER_H
