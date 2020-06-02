///////////////////////////////////////////////////////////////////////////////
// FILE:       EnumerationProperty.cpp
// PROJECT:    MicroManager
// SUBSYSTEM:  DeviceAdapters
//-----------------------------------------------------------------------------
// DESCRIPTION:
// Cobolt Lasers Controller Adapter
//
// COPYRIGHT:     Cobolt AB, Stockholm, 2020
//                All rights reserved
//
// LICENSE:       MIT
//                Permission is hereby granted, free of charge, to any person obtaining a
//                copy of this software and associated documentation files( the "Software" ),
//                to deal in the Software without restriction, including without limitation the
//                rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
//                sell copies of the Software, and to permit persons to whom the Software is
//                furnished to do so, subject to the following conditions:
//                
//                The above copyright notice and this permission notice shall be included in all
//                copies or substantial portions of the Software.
//
//                THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
//                INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
//                PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
//                HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
//                OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
//                SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

// AUTHORS:
//                Lukas Kalinski / lukas.kalinski@coboltlasers.com (2020)

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
    return ( ResolveDeviceValue( enumerationItemName ) != "" );
}

/**
 * \brief Translates value in MM GUI to value on device. Returns empty string if resolving failed.
 */
std::string EnumerationProperty::ResolveDeviceValue( const std::string& guiValue ) const
{
    for ( enumeration_items_t::const_iterator enumerationItem = enumerationItems_.begin();
        enumerationItem != enumerationItems_.end();
        enumerationItem++ ) {

        if ( guiValue == enumerationItem->name ) {
            return enumerationItem->deviceValue;
        }
    }

    return "";
}

/**
 * \brief Translates value on device to value in MM GUI. Returns empty string if resolving failed.
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
