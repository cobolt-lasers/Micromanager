/**
 * \file        Property.cpp
 *
 * \authors     Lukas Kalinski
 *
 * \copyright   Cobolt AB, 2020. All rights reserved. // TODO: Set proper license
 */

#include "Property.h"
#include "Laser.h"

NAMESPACE_COBOLT_BEGIN

int Property::NextPropertyId_ = 1;

Property::Property( const Stereotype stereotype, const std::string& name ) :
    stereotype_( stereotype ),
    name_( name )
{
    const std::string propertyIdStr = std::to_string( (long double) NextPropertyId_++ );
    name_ = ( std::string( 2 - propertyIdStr.length(), '0' ) + propertyIdStr ) + "-" + name;
}

int Property::IntroduceToGuiEnvironment( GuiEnvironment* )
{
    return return_code::ok;
}

const std::string& Property::GetName() const
{
    return name_;
}

std::string Property::GetValue() const
{
    std::string value;
    GetValue( value );
    return value;
}

Property::Stereotype Property::GetStereotype() const
{
    return stereotype_;
}

bool Property::IsMutable() const
{
    return false;
}

int Property::OnGuiSetAction( GuiProperty& )
{
    Logger::Instance()->LogMessage( "Property[" + GetName() + "]::OnGuiSetAction(): Ignoring 'set' action on read-only property.", true );
    return return_code::ok;
}

int Property::OnGuiGetAction( GuiProperty& guiProperty )
{
    std::string string;
    int returnCode = GetValue( string );

    if ( returnCode != return_code::ok ) {

        SetToUnknownValue( guiProperty );
        return returnCode;
    }

    guiProperty.Set( string.c_str() );

    return returnCode;
}

/**
 * \brief The property object represented in a string. For logging/debug purposes.
 */
std::string Property::ObjectString() const
{
    return "stereotype = " + std::to_string( (long double) stereotype_ ) + "; name_ = " + name_ + "; ";
}

void Property::SetToUnknownValue( std::string& string ) const
{
    string = "Unknown";
}

void Property::SetToUnknownValue( GuiProperty& guiProperty ) const
{
    switch ( GetStereotype() ) {
        case Float:   guiProperty.Set( "0" ); break;
        case Integer: guiProperty.Set( "0" ); break;
        case String:  guiProperty.Set( "Unknown" ); break;
    }
}

NAMESPACE_COBOLT_END
