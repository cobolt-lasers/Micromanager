/**
 * \file        PrefixedValue.h
 *
 * \authors     Lukas Kalinski
 *
 * \copyright   Cobolt AB, 2019. All rights reserved.
 */

#ifndef __COBOLT__PREFIXED_VALUE_H
#define __COBOLT__PREFIXED_VALUE_H

#include <math.h>
#include "cobolt.h"

NAMESPACE_COBOLT_BEGIN

class PrefixedValue
{
public:

    typedef float coefficient_t;
    typedef float raw_t;

    enum Prefix { Micro = -6, Milli = -3, NoPrefix = 0 };

    PrefixedValue( const coefficient_t coefficient, const Prefix prefix ) :
        _coefficient( coefficient ),
        _base( pow( 10.0f, prefix ) )
    {
    }

    inline bool isZero( raw_t acceptableError = 0 ) const
    {
        return ( abs( raw() ) < acceptableError );
    }

    /**
     * Returns a new PrefixedValue object representing the same value as this one,
     * but with the new prefix.
     */
    inline PrefixedValue convertedTo( const Prefix prefix ) const
    {
        return PrefixedValue( _coefficient * _base / prefixToBase( prefix ), prefix );
    }

    virtual bool operator == ( const PrefixedValue& rhs )
    {
        return raw() == rhs.raw();
    }

    virtual bool operator != ( const PrefixedValue& rhs )
    {
        return raw() != rhs.raw();
    }

    virtual bool operator <  ( const PrefixedValue& rhs )
    {
        return raw() < rhs.raw();
    }

    virtual bool operator >  ( const PrefixedValue& rhs )
    {
        return raw() > rhs.raw();
    }

    virtual bool operator >= ( const PrefixedValue& rhs )
    {
        return raw() >= rhs.raw();
    }

    virtual bool operator <= ( const PrefixedValue& rhs )
    {
        return raw() <= rhs.raw();
    }

protected:
    typedef float base_t;

    static inline base_t prefixToBase( const Prefix prefix )
    {
        return (base_t) pow( 10.0f, (int)prefix );
    }

    PrefixedValue() : _coefficient( 0 ), _base( 1 ) {}

    inline raw_t raw() const
    {
        return ( _coefficient * _base );
    }

    /**
     * Returns the full raw/primitive value converted to the specified prefix. E.g, if we have a current, say 1.5 A, then
     * calling this function with prefix mA will return 1500 (mA).
     */
    inline raw_t rawAs( const Prefix prefix ) const
    {
        return ( _coefficient * _base / prefixToBase( prefix ) );
    }

    friend bool operator == ( const PrefixedValue& c1, const PrefixedValue& c2 );
    friend bool operator != ( const PrefixedValue& c1, const PrefixedValue& c2 );
    friend bool operator <  ( const PrefixedValue& c1, const PrefixedValue& c2 );
    friend bool operator >  ( const PrefixedValue& c1, const PrefixedValue& c2 );
    friend bool operator >= ( const PrefixedValue& c1, const PrefixedValue& c2 );
    friend bool operator <= ( const PrefixedValue& c1, const PrefixedValue& c2 );

private:

    coefficient_t _coefficient;
    base_t _base;
};


bool operator == ( const PrefixedValue& c1, const PrefixedValue& c2 );
bool operator != ( const PrefixedValue& c1, const PrefixedValue& c2 );
bool operator <  ( const PrefixedValue& c1, const PrefixedValue& c2 );
bool operator >  ( const PrefixedValue& c1, const PrefixedValue& c2 );
bool operator >= ( const PrefixedValue& c1, const PrefixedValue& c2 );
bool operator <= ( const PrefixedValue& c1, const PrefixedValue& c2 );

NAMESPACE_COBOLT_END

#endif // #ifndef __COBOLT__PREFIXED_VALUE_H
