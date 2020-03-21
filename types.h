/**
 * \file        types.h
 *
 * \brief       Brings together all custom types needed by the Cobolt adapter
 *              as well as their ToString/FromString converter functions.
 *
 * \authors     Lukas Kalinski
 *
 * \copyright   Cobolt AB, 2020. All rights reserved. // TODO: Set proper license
 */

#ifndef __COBOLT__TYPE_H
#define __COBOLT__TYPE_H

#include <string>

#include "cobolt.h"

#include "Current.h"
#include "Power.h"
#include "Voltage.h"

NAMESPACE_COBOLT_BEGIN

namespace laser
{
    #define FOREACH_TOGGLE_VALUE( GENERATOR ) \
        GENERATOR( on,  "On" ) \
        GENERATOR( off, "Off" )

    namespace toggle { GENERATE_ENUM_STRING( FOREACH_TOGGLE_VALUE ); }

    #undef FOREACH_TOGGLE_VALUE

    #define FOREACH_FLAG_VALUE( GENERATOR ) \
        GENERATOR( enabled,  "Enabled" ) \
        GENERATOR( disabled, "Disabled" )

    namespace flag { GENERATE_ENUM_STRING( FOREACH_FLAG_VALUE ); }

    #undef FOREACH_FLAG_VALUE

    #define FOREACH_ANALOG_IMPEDANCE_VALUE( GENERATOR ) \
        GENERATOR( low,  "50 Ohm" ) \
        GENERATOR( high, "1 kOhm" )

    namespace analog_impedance { GENERATE_ENUM_STRING( FOREACH_ANALOG_IMPEDANCE_VALUE ); }

    #undef FOREACH_ANALOG_IMPEDANCE_VALUE
    
    #define FOREACH_RUN_MODE_VALUE( GENERATOR ) \
        GENERATOR( constant_current,    "Constant Current Mode" ) \
        GENERATOR( constant_power,      "Constant Power Mode" ) \
        GENERATOR( modulation,          "Modulation Mode" )

    namespace run_mode { GENERATE_ENUM_STRING( FOREACH_RUN_MODE_VALUE ); }

    #undef FOREACH_RUN_MODE_VALUE
}

template <typename T, typename S>
S convert_to( const T& t );

template <> std::string convert_to<Current, std::string> ( const Current& c ) { return std::to_string( (long double) c.mA() ); }
template <> std::string convert_to<Power, std::string>   ( const Power& c )   { return std::to_string( (long double) c.mW() ); }
template <> std::string convert_to<Voltage, std::string> ( const Voltage& c ) { return std::to_string( (long double) c.mV() ); }

template <> double convert_to<Current, double> ( const Current& c ) { return c.mA(); }
template <> double convert_to<Power, double>   ( const Power& c )   { return c.mW(); }
template <> double convert_to<Voltage, double> ( const Voltage& c ) { return c.mV(); }

template <> std::string convert_to<laser::toggle::type, std::string>            ( const laser::toggle::type& t )           { return laser::toggle::ToString( t ); }
template <> std::string convert_to<laser::flag::type, std::string>              ( const laser::flag::type& t )             { return laser::flag::ToString( t ); }
template <> std::string convert_to<laser::run_mode::type, std::string>          ( const laser::run_mode::type& t )         { return laser::run_mode::ToString( t ); }
template <> std::string convert_to<laser::analog_impedance::type, std::string>  ( const laser::analog_impedance::type& t ) { return laser::analog_impedance::ToString( t ); }

template <> laser::toggle::type             convert_to<std::string, laser::toggle::type>( const std::string& s )           { return laser::toggle::FromString( s ); }
template <> laser::flag::type               convert_to<std::string, laser::flag::type>( const std::string& s )             { return laser::flag::FromString( s ); }
template <> laser::run_mode::type           convert_to<std::string, laser::run_mode::type>( const std::string& s )         { return laser::run_mode::FromString( s ); }
template <> laser::analog_impedance::type   convert_to<std::string, laser::analog_impedance::type>( const std::string& s ) { return laser::analog_impedance::FromString( s ); }

NAMESPACE_COBOLT_END

#endif // #ifndef __COBOLT__TYPE_H