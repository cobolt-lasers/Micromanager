/**
 * \file        DeviceProperty.cpp
 *
 * \authors     Lukas Kalinski
 *
 * \copyright   Cobolt AB, 2020. All rights reserved. // TODO: Set proper license
 */

#include "DeviceProperty.h"
#include "Laser.h"

NAMESPACE_COBOLT_BEGIN

DeviceProperty::DeviceProperty( Property::Stereotype stereotype, const std::string& name, LaserDevice* laserDevice, const std::string& getCommand ) :
    Property( stereotype, name ),
    laserDevice_( laserDevice ),
    getCommand_( getCommand ),
    doCache_( true )
{}

void DeviceProperty::SetCaching( const bool enabled )
{
    doCache_ = enabled;
}

std::string DeviceProperty::ObjectString() const
{
    return Property::ObjectString() + "getCommand_ = " + getCommand_ + "; ";
}

int DeviceProperty::FetchInto( std::string& string ) const
{
    int returnCode = return_code::ok;

    if ( doCache_ ) {

        if ( cachedValue_.length() == 0 ) {
            returnCode = laserDevice_->SendCommand( getCommand_, &cachedValue_ );
        }

        if ( returnCode == return_code::ok ) {
            string = cachedValue_;
        } else {
            ClearCache();
        }

    } else {

        returnCode = laserDevice_->SendCommand( getCommand_, &string );
    }

    if ( returnCode != return_code::ok ) {
        SetToUnknownValue( string );
    }

    return returnCode;
}

void DeviceProperty::ClearCache() const
{
    cachedValue_.clear();
}

NAMESPACE_COBOLT_END
