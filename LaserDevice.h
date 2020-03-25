/**
 * \file        LaserDevice.h
 *
 * \authors     Lukas Kalinski
 *
 * \copyright   Cobolt AB, 2020. All rights reserved. // TODO: Set proper license
 */

#ifndef __COBOLT__LASER_DEVICE_H
#define __COBOLT__LASER_DEVICE_H

#include <string>

namespace cobolt
{
    class LaserDevice
    {
    public:

        /**
         * \brief Sends a command to the laser device. Returns true on success or false otherwise.
         */
        virtual bool SendCommand( const std::string& command, std::string* response = NULL ) const = 0;
    };
}

#endif // #ifndef __COBOLT__LASER_DEVICE_H