/**
 * \file        LaserDriver.h
 *
 * \authors     Lukas Kalinski
 *
 * \copyright   Cobolt AB, 2020. All rights reserved. // TODO: Set proper license
 */

#ifndef __COBOLT__LASER_DRIVER_H
#define __COBOLT__LASER_DRIVER_H

#include <string>

namespace cobolt
{
    class LaserDriver
    {
    public:

        /**
         * \brief Sends a command to the laser device. Returns true on success or false otherwise.
         */
        virtual int SendCommand( const std::string& command, std::string* response = NULL ) = 0;
    };
}

#endif // #ifndef __COBOLT__LASER_DRIVER_H