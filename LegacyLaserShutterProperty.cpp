/**
 * \file        LegacyLaserShutterProperty.cpp
 *
 * \authors     Lukas Kalinski
 *
 * \copyright   Cobolt AB, 2020. All rights reserved. // TODO: Set proper license
 */

#include "LegacyLaserShutterProperty.h"
#include "Laser.h"

NAMESPACE_COBOLT_BEGIN

const std::string LegacyLaserShutterProperty::Value_Open = "open";
const std::string LegacyLaserShutterProperty::Value_Closed = "closed";

LegacyLaserShutterProperty::LegacyLaserShutterProperty( const std::string& name, LaserDevice* laserDevice ) :
    MutableDeviceProperty( Property::String, name, laserDevice, "N/A" ),
    savedLaserState_( NULL )
{
}

LegacyLaserShutterProperty::~LegacyLaserShutterProperty()
{
    if ( savedLaserState_ != NULL ) {
        delete savedLaserState_;
    }
}

int LegacyLaserShutterProperty::GetValue( std::string& string ) const
{
    if ( IsOpen() ) {
        string = Value_Open;
    } else {
        string = Value_Closed;
    }

    return return_code::ok;
}

int LegacyLaserShutterProperty::Set( const std::string& value )
{
    int returnCode = return_code::ok;

    if ( value == Value_Closed && IsOpen() ) { // Close requested.

        savedLaserState_ = new LaserState();

        if ( laserDevice_->SendCommand( "glc?", &savedLaserState_->currentSetpoint ) != return_code::ok ||
             laserDevice_->SendCommand( "gam?", &savedLaserState_->runMode ) != return_code::ok ) {

            Logger::Instance()->LogError( "LegacyLaserShutterProperty[ " + GetName() + " ]::Set( '" + value + "' ): Failed to save laser state" );
            return return_code::error;
        }

        returnCode = laserDevice_->SendCommand( "ecc" );
        if ( returnCode == return_code::ok ) {
            returnCode = laserDevice_->SendCommand( "slc 0" );
        }

    } else if ( !IsOpen() ) { // Open requested.

        returnCode = laserDevice_->SendCommand( "sam " + savedLaserState_->runMode );
        if ( returnCode == return_code::ok ) {
            returnCode = laserDevice_->SendCommand( "slc " + savedLaserState_->currentSetpoint );
        }

        delete savedLaserState_;
        savedLaserState_ = NULL;

    } else {

        Logger::Instance()->LogMessage( "LegacyLaserShutterProperty[" + GetName() + "]::Set( '" + value + "' ): Ignored request as requested state is already set", true );
    }

    return returnCode;
}

bool LegacyLaserShutterProperty::IsOpen() const
{
    return ( savedLaserState_ == NULL );
}

NAMESPACE_COBOLT_END
