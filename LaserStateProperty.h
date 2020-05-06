/**
 * \file        LaserStateProperty.h
 *
 * \authors     Lukas Kalinski
 *
 * \copyright   Cobolt AB, 2020. All rights reserved. // TODO: Set proper license
 */

#ifndef __COBOLT__LASER_STATE_PROPERTY_H
#define __COBOLT__LASER_STATE_PROPERTY_H

#include "DeviceProperty.h"
#include <map>
#include <set>

NAMESPACE_COBOLT_BEGIN

class LaserStateProperty : public DeviceProperty
{
    typedef DeviceProperty Parent;

public:

    LaserStateProperty( Property::Stereotype stereotype, const std::string& name, LaserDriver* laserDriver, const std::string& getCommand );

    void RegisterState( const std::string& deviceValue, const std::string& guiValue, const bool allowsShutter );

    int GetValue( std::string& string ) const;
    bool AllowsShutter() const;

protected:

    bool IsCacheEnabled() const;

private:

    std::map<std::string, std::string> stateMap_;
    std::set<std::string> shutterAllowedStates_;
};

NAMESPACE_COBOLT_END

#endif // #ifndef __COBOLT__LASER_STATE_PROPERTY_H
