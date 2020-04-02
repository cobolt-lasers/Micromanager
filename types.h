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

/**
 * \brief Maps the value accepted by a command and the corresponding value as seen in and received from the GUI.
 */
struct StringValueMap
{
    std::string commandValue;   // Value accepted by laser serial interface.
    std::string guiValue;       // Value as seen in the GUI.
};

inline bool operator == ( const std::string& lhs, const StringValueMap& rhs )
{
    return ( lhs == rhs.guiValue || lhs == rhs.commandValue );
}

inline bool operator != ( const std::string& lhs, const StringValueMap& rhs )
{
    return !( lhs != rhs );
}

namespace value // TODO: Move somewhere?
{
    namespace analog_impedance
    {
        extern StringValueMap high;
        extern StringValueMap low;
        extern const StringValueMap* values[]; // TODO: Do we need this?
    }

    namespace flag
    {
        extern StringValueMap enable;
        extern StringValueMap disable;
        extern const StringValueMap* values[];
    }

    namespace toggle
    {
        extern StringValueMap on;
        extern StringValueMap off;
        extern const StringValueMap* values[];
    }
}

NAMESPACE_COBOLT_END

#endif // #ifndef __COBOLT__TYPE_H