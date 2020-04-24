/**
 * \file        LegacyLaserShutterProperty.h
 *
 * \authors     Lukas Kalinski
 *
 * \copyright   Cobolt AB, 2020. All rights reserved. // TODO: Set proper license
 */

#ifndef __COBOLT__LEGACY_LASER_SHUTTER_PROPERTY_H
#define __COBOLT__LEGACY_LASER_SHUTTER_PROPERTY_H

#include "EnumerationProperty.h"

NAMESPACE_COBOLT_BEGIN

class LegacyLaserShutterProperty : public MutableDeviceProperty
{
public:

    static const std::string Value_Open;
    static const std::string Value_Closed;

    LegacyLaserShutterProperty( const std::string& name, LaserDevice* laserDevice );
    virtual ~LegacyLaserShutterProperty();

    virtual int FetchInto( std::string& string ) const;
    virtual int Set( const std::string& value );
    
private:

    struct LaserState
    {
        std::string runMode;
        std::string currentSetpoint;
    };

    bool IsOpen() const;

    LaserState* savedLaserState_;
};

NAMESPACE_COBOLT_END

#endif // #ifndef __COBOLT__LEGACY_LASER_SHUTTER_PROPERTY_H
