/**
 * \file        types.cpp
 *
 * \authors     Lukas Kalinski
 *
 * \copyright   Cobolt AB, 2020. All rights reserved. // TODO: Set proper license
 */

#include "types.h"

NAMESPACE_COBOLT_BEGIN

const char* g_GuiPropertyValue_InvalidResponse = "Invalid Value";

template <> bool CommandResponseValueStringToGuiValueString<std::string>( std::string& )
{
    return true;
}

template <> bool CommandResponseValueStringToGuiValueString<double>( std::string& )
{
    return true;
}

template <> bool CommandResponseValueStringToGuiValueString<type::analog_impedance::symbol>( std::string& string )
{
    const int value = atoi( string.c_str() );
    if ( value != 0 && value != 1 ) { string = g_GuiPropertyValue_InvalidResponse; return false; }
    string = type::analog_impedance::ToString( ( type::analog_impedance::symbol )value );
    return true;
}

template <> bool CommandResponseValueStringToGuiValueString<type::flag::symbol>( std::string& string )
{
    const int value = atoi( string.c_str() );
    if ( value != 0 && value != 1 ) { string = g_GuiPropertyValue_InvalidResponse; return false; }
    string = type::flag::ToString( (type::flag::symbol) value );
    return true;
}

template <> bool CommandResponseValueStringToGuiValueString<type::run_mode::cc_cp_mod::symbol>( std::string& string )
{
    const int value = atoi( string.c_str() );
    if ( value != 0 && value != 1 ) { string = g_GuiPropertyValue_InvalidResponse; return false; }
    string = type::run_mode::cc_cp_mod::ToString( (type::run_mode::cc_cp_mod::symbol) value );
    return true;
}

template <> bool CommandResponseValueStringToGuiValueString<type::toggle::symbol>( std::string& string )
{
    const int value = atoi( string.c_str() );
    if ( value != 0 && value != 1 ) { string = g_GuiPropertyValue_InvalidResponse; return false; }
    string = type::toggle::ToString( ( type::toggle::symbol ) value );
    return true;
}

/**
 * \brief Specializations of this function reformat GUI value strings into valid command argument strings.
 *
 * \example If run mode in gui is set to "Constant Power" then running the right specialization here will
 *          translate that into the corresponding run mode number.
 */
template <typename T>
bool GuiValueStringToCommandArgumentString( std::string& string );

template <> bool GuiValueStringToCommandArgumentString<std::string>( std::string& )
{
    return true;
}

template <> bool GuiValueStringToCommandArgumentString<double>( std::string& )
{
    return true;
}

template <> bool GuiValueStringToCommandArgumentString<type::analog_impedance::symbol>( std::string& string )
{
    const type::analog_impedance::symbol value = type::analog_impedance::FromString( string );
    if ( value == type::analog_impedance::__undefined__ ) { return false; }
    string = std::to_string( (_Longlong) value );
    return true;
}

template <> bool GuiValueStringToCommandArgumentString<type::flag::symbol>( std::string& string )
{
    const type::flag::symbol value = type::flag::FromString( string );
    if ( value == type::flag::__undefined__ ) { return false; }
    string = std::to_string( (_Longlong) value );
    return true;
}

template <> bool GuiValueStringToCommandArgumentString<type::run_mode::cc_cp_mod::symbol>( std::string& string )
{
    const type::run_mode::cc_cp_mod::symbol value = type::run_mode::cc_cp_mod::FromString( string );
    if ( value == type::run_mode::cc_cp_mod::__undefined__ ) { return false; }
    string = std::to_string( (_Longlong) value );
    return true;
}

template <> bool GuiValueStringToCommandArgumentString<type::toggle::symbol>( std::string& string )
{
    const type::toggle::symbol value = type::toggle::FromString( string );
    if ( value == type::toggle::__undefined__ ) { return false; }
    string = std::to_string( (_Longlong) value );
    return true;
}

NAMESPACE_COBOLT_END
