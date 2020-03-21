/**
 * \file        Logger.h
 *
 * \authors     Lukas Kalinski
 *
 * \copyright   Cobolt AB, 2020. All rights reserved. // TODO: Set proper license
 */

#ifndef __COBOLT__LOGGER
#define __COBOLT__LOGGER

#include <string>

namespace cobolt
{
    class Logger
    {
        virtual int LogMessage( const char* msg, bool debugOnly ) const = 0;
    };
}

#endif // #ifndef __COBOLT__LOGGER