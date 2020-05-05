/**
 * \file        EnumerationProperty.cpp
 *
 * \authors     Lukas Kalinski
 *
 * \copyright   Cobolt AB, 2020. All rights reserved. // TODO: Set proper license
 */

#include "EnumerationProperty.h"
#include "Laser.h"

NAMESPACE_COBOLT_BEGIN

EnumerationProperty::EnumerationProperty( const std::string& name, LaserDriver* laserDriver, const std::string& getCommand ) :
    MutableDeviceProperty( Property::String, name, laserDriver, getCommand )
{}

int EnumerationProperty::IntroduceToGuiEnvironment( GuiEnvironment* environment )
{
    for ( enumeration_items_t::const_iterator enumerationItem = enumerationItems_.begin();
          enumerationItem != enumerationItems_.end();
          enumerationItem++ ) {

        const int returnCode = environment->RegisterAllowedGuiPropertyValue( GetName(), enumerationItem->name );
        if ( returnCode != return_code::ok ) {
            return returnCode;
        }

        Logger::Instance()->LogMessage( "EnumerationProperty[ " + GetName() + " ]::IntroduceToGuiEnvironment(): Registered valid value '" +
            enumerationItem->name + "' in GUI.", true );
    }

    return return_code::ok;
}

void EnumerationProperty::RegisterEnumerationItem( const std::string& deviceValue, const std::string& setCommand, const std::string& name )
{
    Logger::Instance()->LogMessage( "EnumerationProperty[ " + GetName() + " ]::RegisterEnumerationItem( { '" + 
        deviceValue + "' , '" + setCommand + "', '" + name + "' } )", true );

    EnumerationItem enumerationItem = { deviceValue, setCommand, name };

    enumerationItems_.push_back( enumerationItem );
}

int EnumerationProperty::GetValue( std::string& string ) const
{
    std::string deviceValue;
    Parent::GetValue( deviceValue );

    string = ResolveEnumerationItem( deviceValue );
    
    if ( string == "" ) {

        SetToUnknownValue( string );
        Logger::Instance()->LogError( "EnumerationProperty[" + GetName() + "]::GetValue( ... ): No matching GUI value found for command value '" + deviceValue + "'" );
        return return_code::error; // Not 'invalid_value', as the cause is not the user.
    }

    return return_code::ok;
}

int EnumerationProperty::SetValue( const std::string& enumerationItemName )
{
    for ( enumeration_items_t::const_iterator enumerationItem = enumerationItems_.begin();
          enumerationItem != enumerationItems_.end();
          enumerationItem++ ) {

        if ( enumerationItemName == enumerationItem->name ) {
            return laserDriver_->SendCommand( enumerationItem->setCommand );
        }
    }

    Logger::Instance()->LogError( "EnumerationProperty[ " + GetName() + " ]::SetValue(): Invalid enumeration item '" + enumerationItemName + "'" );
    return return_code::invalid_property_value;
}


bool EnumerationProperty::IsValidValue( const std::string& enumerationItemName )
{
    for ( enumeration_items_t::const_iterator enumerationItem = enumerationItems_.begin();
        enumerationItem != enumerationItems_.end();
        enumerationItem++ ) {

        if ( enumerationItemName == enumerationItem->name ) {
            return true;
        }
    }

    return false; 
}

/**
 * \brief Translates between value on device and value in MM GUI. Returns empty string if resolving failed.
 */
std::string EnumerationProperty::ResolveEnumerationItem( const std::string& deviceValue ) const
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
