/**
 * \file        Current.h
 *
 * \authors     Lukas Kalinski
 *
 * \copyright   Cobolt AB, 2018. All rights reserved.
 */

#ifndef __COBOLT__CURRENT_H
#define __COBOLT__CURRENT_H

#include "PrefixedValue.h"

NAMESPACE_COBOLT_BEGIN

// TODO: Add strategy pattern? Making it possible to create currents out of adc values? -> NO, let ADC class provide a current object instead.

class Current : public PrefixedValue
{
public:

    typedef PrefixedValue Parent;
    
    using Parent::Prefix;

    typedef typename PrefixedValue::raw_t milliamperes_t;
    typedef typename PrefixedValue::raw_t amperes_t;

    static Current mA( milliamperes_t milliamperes )
    {
        return Current( milliamperes );
    }

    static Current A( amperes_t amperes )
    {
        return Current( amperes * 1000.0f );
    }

    static Current zero()
    {
        return Current( 0 );
    }

    Current() : PrefixedValue() {}

    inline amperes_t A() const
    {
        return PrefixedValue::raw();
    }

    inline milliamperes_t mA() const
    {
        return Parent::rawAs( Parent::Milli );
    }

    friend Current operator* ( const Current& c, float factor );
    friend Current operator* ( float factor, const Current& c );

    friend Current operator- ( const Current& minuend, const Current& subtrahend );
    friend Current operator+ ( const Current& term1, const Current& term2 );

private:

    Current( milliamperes_t milliamperes ) :
        PrefixedValue( milliamperes, Parent::Milli )
    {
    }
};

Current operator* ( const Current& c, float factor );
Current operator* ( float factor, const Current& c );
Current operator- ( const Current& minuend, const Current& subtrahend );
Current operator+ ( const Current& term1, const Current& term2 );

NAMESPACE_COBOLT_END

#endif // #ifndef __COBOLT__CURRENT_H
