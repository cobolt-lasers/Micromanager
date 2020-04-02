/**
 * \file        cobolt.h
 *
 * \authors     Lukas Kalinski
 *
 * \copyright   Cobolt AB, 2020. All rights reserved. // TODO: Set proper license
 */

#ifndef __COBOLT_H
#define __COBOLT_H

#include <vector>

#define NAMESPACE_COBOLT_BEGIN namespace cobolt {
#define NAMESPACE_COBOLT_END   }

#define NAMESPACE_COBOLT_COMPATIBILITY_BEGIN( NS ) namespace cobolt { namespace compatibility { namespace NS {
#define NAMESPACE_COBOLT_COMPATIBILITY_END         }}}

#define	__STRINGIFY( s ) #s
#define	STRINGIFY( s ) __STRINGIFY( s )

#define ENUM_VALUE_GENERATOR( e ) e,
#define ENUM_VALUE_GENERATOR2( e, _ ) e,
#define ENUM_VALUE_PAIR_GENERATOR( e, v ) e = v,
#define ENUM_VALUE_PAIR_GENERATOR3( e, v, _ ) e = v,

#define STRING_VALUE_GENERATOR( s ) std::string( #s ),
#define STRING_VALUE_GENERATOR2( _, s ) std::string( #s ),
#define STRING_VALUE_GENERATOR1( s, _ ) std::string( #s ),
#define CSTRING_VALUE_GENERATOR( s ) STRINGIFY( s ),
#define CSTRING_VALUE_GENERATOR2( _, s ) STRINGIFY( s ),
#define QUOTED_CSTRING_VALUE_GENERATOR( _1, _2, s ) s,

#define GENERATE_ENUM_STRING_MAP( FOREACH )                                                                                                                               \
    enum symbol { FOREACH( ENUM_VALUE_PAIR_GENERATOR3 ) __count__, __undefined__ };                                                                                       \
    static std::string symbol_strings[] = { FOREACH( QUOTED_CSTRING_VALUE_GENERATOR ) "", "" };                                                                           \
    inline symbol FromString( const std::string& s ) { for ( int i = 0; i < __count__; i++ ) if ( s == symbol_strings[ i ] ) return (symbol) i; return __undefined__; }  \
    inline std::string ToString( symbol t ) { if ( t < __count__ && t >= 0 ) return symbol_strings[ (int) t ]; return ""; }

NAMESPACE_COBOLT_BEGIN

namespace return_code
{
    const int ok = 0;
    const int error = 1;
    const int invalid_property_value = 3;
    const int unsupported_command = 11;
    const int illegal_port_change = 101001;
    const int serial_port_undefined = 101002;
    const int laser_off = 101003;
}

template <typename T>
std::vector<T> VectorFromArray( T* array )
{
    return std::vector<T>( array, array + sizeof( array ) / sizeof( array[ 0 ] ) );
}

NAMESPACE_COBOLT_END

#include "Logger.h"

#endif // #ifndef __COBOLT_H