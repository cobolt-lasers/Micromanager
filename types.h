/**
 * \file        types.h
 *
 * \brief       Brings together all custom types needed by the Cobolt adapter
 *              as well as their various conversion functions.
 *
 * \authors     Lukas Kalinski
 *
 * \copyright   Cobolt AB, 2020. All rights reserved. // TODO: Set proper license
 */

#ifndef __COBOLT__TYPE_H
#define __COBOLT__TYPE_H

#include <string>

#include "cobolt.h"

NAMESPACE_COBOLT_BEGIN

namespace type
{
     #define FOREACH_ANALOG_IMPEDANCE_VALUE( GENERATOR ) \
        GENERATOR( high,    0,  "1 kOhm" ) \
        GENERATOR( low,     1,  "50 Ohm" )
    namespace analog_impedance { GENERATE_ENUM_STRING_MAP( FOREACH_ANALOG_IMPEDANCE_VALUE ); }
    #undef FOREACH_ANALOG_IMPEDANCE_VALUE
    
    #define FOREACH_FLAG_VALUE( GENERATOR ) \
        GENERATOR( disabled,    0,  "Disabled" ) \
        GENERATOR( enabled,     1,  "Enabled" )
    namespace flag { GENERATE_ENUM_STRING_MAP( FOREACH_FLAG_VALUE ); }
    #undef FOREACH_FLAG_VALUE
    
    namespace run_mode
    {
        #define FOREACH_RUN_MODE_VALUE( GENERATOR ) \
            GENERATOR( constant_current, 0,   "Constant Current" ) \
            GENERATOR( constant_power,   1,   "Constant Power" ) \
            GENERATOR( modulation,       2,   "Modulation" )
        namespace cc_cp_mod { GENERATE_ENUM_STRING_MAP( FOREACH_RUN_MODE_VALUE ); }
        #undef FOREACH_RUN_MODE_VALUE
    }

    #define FOREACH_TOGGLE_VALUE( GENERATOR ) \
        GENERATOR( off, 0,  "Off" ) \
        GENERATOR( on,  1,  "On" )
    namespace toggle { GENERATE_ENUM_STRING_MAP( FOREACH_TOGGLE_VALUE ); }
    #undef FOREACH_TOGGLE_VALUE
}

/**
 * \brief Specializations of this function reformat strings received from get command executions into
 *        corresponding value strings to be shown in the GUI.
 *
 * \example The result of a get run mode command is a number that the proper specialization of this
 *          function will translate into a string intended for GUI presentation, thus a command response
 *          saying '1' would be translated into, for example, "Constant Power".
 */
template <typename T>
bool CommandResponseValueStringToGuiValueString( std::string& ) { return false; }

template <> bool CommandResponseValueStringToGuiValueString<std::string>( std::string& );
template <> bool CommandResponseValueStringToGuiValueString<double>( std::string& );
template <> bool CommandResponseValueStringToGuiValueString<type::analog_impedance::symbol>( std::string& );
template <> bool CommandResponseValueStringToGuiValueString<type::flag::symbol>( std::string& );
template <> bool CommandResponseValueStringToGuiValueString<type::run_mode::cc_cp_mod::symbol>( std::string& );

/**
 * \brief Specializations of this function reformat GUI value strings into valid command argument strings.
 *
 * \example If run mode in gui is set to "Constant Power" then running the right specialization here will
 *          translate that into the corresponding run mode number.
 */
template <typename T>
bool GuiValueStringToCommandArgumentString( std::string& ) { return false; }

template <> bool GuiValueStringToCommandArgumentString<std::string>( std::string& );
template <> bool GuiValueStringToCommandArgumentString<double>( std::string& );
template <> bool GuiValueStringToCommandArgumentString<type::analog_impedance::symbol>( std::string& );
template <> bool GuiValueStringToCommandArgumentString<type::flag::symbol>( std::string& );
template <> bool GuiValueStringToCommandArgumentString<type::run_mode::cc_cp_mod::symbol>( std::string& );

NAMESPACE_COBOLT_END

#endif // #ifndef __COBOLT__TYPE_H