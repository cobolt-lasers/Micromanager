/**
 * \file        Voltage.h
 *
 * \authors     Lukas Kalinski
 *
 * \copyright   Cobolt AB, 2018. All rights reserved.
 */

#ifndef __COBOLT__VOLTAGE_H
#define __COBOLT__VOLTAGE_H

#include "PrefixedValue.h"

NAMESPACE_COBOLT_BEGIN

class Voltage : public PrefixedValue
{
public:

    typedef PrefixedValue Parent;

    typedef typename PrefixedValue::raw_t millivolts_t;
    typedef typename PrefixedValue::raw_t volts_t;

    static Voltage mV( millivolts_t millivolts )
    {
        return Voltage( millivolts );
    }

    static Voltage V( volts_t volts )
    {
        return Voltage( volts * 1000 );
    }

    static Voltage zero()
    {
        return Voltage( 0 );
    }

    Voltage() : PrefixedValue() {}

    inline volts_t V() const
    {
        return PrefixedValue::raw();
    }

    inline millivolts_t mV() const
    {
        return Parent::rawAs( Parent::Milli );
    }

    friend Voltage operator* ( const Voltage& c, float factor );
    friend Voltage operator* ( float factor, const Voltage& c );

    friend Voltage operator- ( const Voltage& minuend, const Voltage& subtrahend );
    friend Voltage operator+ ( const Voltage& term1, const Voltage& term2 );

private:

    Voltage( millivolts_t millivolts ) :
        Parent( millivolts, Parent::Milli )
    {
    }
};

Voltage operator* ( const Voltage& c, float factor );
Voltage operator* ( float factor, const Voltage& c );
Voltage operator- ( const Voltage& minuend, const Voltage& subtrahend );
Voltage operator+ ( const Voltage& term1, const Voltage& term2 );

NAMESPACE_COBOLT_END

#endif // #ifndef __COBOLT__VOLTAGE_H
