/**
 * \file        cobolt.h
 *
 * \authors     Lukas Kalinski
 *
 * \copyright   Cobolt AB, 2020. All rights reserved. // TODO: Set proper license
 */

#ifndef __COBOLT_H
#define __COBOLT_H

#define NAMESPACE_COBOLT_BEGIN namespace cobolt {
#define NAMESPACE_COBOLT_END   }

#define NAMESPACE_COBOLT_COMPATIBILITY_BEGIN( NS ) namespace cobolt { namespace compatibility { namespace NS {
#define NAMESPACE_COBOLT_COMPATIBILITY_END         }}}

#define	__STRINGIFY( s ) #s
#define	STRINGIFY( s ) __STRINGIFY( s )

#define ENUM_VALUE_GENERATOR( e ) e,
#define ENUM_VALUE_GENERATOR2( e, _ ) e,
#define ENUM_VALUE_PAIR_GENERATOR( e, v ) e = v,

#define STRING_VALUE_GENERATOR( s ) core::util::String( #s ),
#define STRING_VALUE_GENERATOR2( _, s ) core::util::String( #s ),
#define STRING_VALUE_GENERATOR1( s, _ ) core::util::String( #s ),
#define CSTRING_VALUE_GENERATOR( s ) STRINGIFY( s ),
#define CSTRING_VALUE_GENERATOR2( _, s ) STRINGIFY( s ),
#define QUOTED_CSTRING_VALUE_GENERATOR( _, s ) s,

#define GENERATE_ENUM_STRING( FOREACH )                                                                                                                     \
    enum type { FOREACH( ENUM_VALUE_GENERATOR2 ) __count__, __undefined__ };                                                                                \
    static const char* string[] = { FOREACH( QUOTED_CSTRING_VALUE_GENERATOR ) "", "" };                                                                     \
    type FromString( const std::string& s ) { for ( int i = 0; i < __count__; i++ ) if ( s == string[ i ] ) return (type) i; return type::__undefined__; }  \
    const char* ToString( type t ) { if ( t < __count__ && t >= 0 ) return string[ (int) t ]; return ""; }

#endif // #ifndef __COBOLT_H