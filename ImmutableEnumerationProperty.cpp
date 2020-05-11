/**
 * \file        ImmutableEnumerationProperty.cpp
 *
 * \authors     Lukas Kalinski
 *
 * \copyright   Cobolt AB, 2020. All rights reserved. // TODO: Set proper license
 */

#include "ImmutableEnumerationProperty.h"
#include "Laser.h"

NAMESPACE_COBOLT_BEGIN
 
ImmutableEnumerationProperty::ImmutableEnumerationProperty( const std::string& name, LaserDriver* laserDriver, const std::string& getCommand ) :
    DeviceProperty( Property::String, name, laserDriver, getCommand )
{}

void ImmutableEnumerationProperty::RegisterEnumerationItem( const std::string& deviceValue, const std::string& name )
{
    Logger::Instance()->LogMessage( "ImmutableEnumerationProperty[ " + GetName() + " ]::RegisterEnumerationItem( { '" + 
        deviceValue + "', '" + name + "' } )", true );

    EnumerationItem enumerationItem = { deviceValue, name };

    enumerationItems_.push_back( enumerationItem );
}

int ImmutableEnumerationProperty::GetValue( std::string& string ) const
{
    std::string deviceValue;
    Parent::GetValue( deviceValue );

    string = ResolveEnumerationItem( deviceValue );
    
    if ( string == "" ) {

        SetToUnknownValue( string );
        Logger::Instance()->LogError( "ImmutableEnumerationProperty[" + GetName() + "]::GetValue( ... ): No matching GUI value found for command value '" + deviceValue + "'" );
        return return_code::error;
    }

    return return_code::ok;
}

/**
 * \brief Translates value on device to value in MM GUI. Returns empty string if resolving failed.
 */
std::string ImmutableEnumerationProperty::ResolveEnumerationItem( const std::string& deviceValue ) const
{
    std::string enumerationItemName;

    for ( enumeration_items_t::const_iterator enumerationItem = enumerationItems_.begin();
        enumerationItem != enumerationItems_.end();
        enumerationItem++ ) {

        if ( enumerationItem->deviceValue == deviceValue ) {
            enumerationItemName = enumerationItem->name;
            break;
        }
    }

    return enumerationItemName;
}

NAMESPACE_COBOLT_END
