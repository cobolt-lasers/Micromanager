/**
 * \file        LaserShutterProperty.cpp
 *
 * \authors     Lukas Kalinski
 *
 * \copyright   Cobolt AB, 2020. All rights reserved. // TODO: Set proper license
 */

#include "LaserShutterProperty.h"
#include "Laser.h"

NAMESPACE_COBOLT_BEGIN

const std::string LaserShutterProperty::Value_Open = "open";
const std::string LaserShutterProperty::Value_Closed = "closed";

LaserShutterProperty::LaserShutterProperty( const std::string& name, LaserDriver* laserDriver, Laser* laser ) :
    EnumerationProperty( name, laserDriver, "N/A" ),
    laser_( laser ),
    isOpen_( false )
{
    RegisterEnumerationItem( "N/A", "l1r", Value_Open );
    RegisterEnumerationItem( "N/A", "l0r", Value_Closed );
}

int LaserShutterProperty::GetValue( std::string& string ) const
{
    if ( isOpen_ ) {
        string = Value_Open;
    } else {
        string = Value_Closed;
    }
    
    return return_code::ok;
}

int LaserShutterProperty::SetValue( const std::string& value )
{
    if ( !laser_->IsShutterEnabled() ) {
        return return_code::property_not_settable_in_current_state;
    }

    int returnCode = EnumerationProperty::SetValue( value );

    if ( returnCode == return_code::ok ) {
        isOpen_ = ( value == Value_Open );
        return return_code::ok;
    }

    return returnCode;
}

NAMESPACE_COBOLT_END
