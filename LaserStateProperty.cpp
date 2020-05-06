/**
 * \file        LaserStateProperty.cpp
 *
 * \authors     Lukas Kalinski
 *
 * \copyright   Cobolt AB, 2020. All rights reserved. // TODO: Set proper license
 */

#include "LaserStateProperty.h"


NAMESPACE_COBOLT_BEGIN

LaserStateProperty::LaserStateProperty( Property::Stereotype stereotype, const std::string& name, LaserDriver* laserDriver, const std::string& getCommand ) :
    DeviceProperty( stereotype, name, laserDriver, getCommand )
{}

void LaserStateProperty::RegisterState( const std::string& deviceValue, const std::string& guiValue, const bool allowsShutter )
{
    stateMap_.insert( deviceValue, guiValue );

    if ( allowsShutter ) {
        shutterAllowedStates_.insert( deviceValue );
    }
}

int LaserStateProperty::GetValue( std::string& string ) const
{
    Parent::GetValue( string );

    if ( stateMap_.find( string ) == stateMap_.end() ) {
        return return_code::unsupported_device_property_value;
    }
    
    string = stateMap_.at( string );
    return return_code::ok;
}

bool LaserStateProperty::AllowsShutter() const
{
    std::string state;
    GetValue( state );

    return ( shutterAllowedStates_.find( state ) != shutterAllowedStates_.end() );
}

bool LaserStateProperty::IsCacheEnabled() const
{
    return false;
}

NAMESPACE_COBOLT_END
