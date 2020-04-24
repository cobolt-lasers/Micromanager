/**
 * \file        MutableDeviceProperty.cpp
 *
 * \authors     Lukas Kalinski
 *
 * \copyright   Cobolt AB, 2020. All rights reserved. // TODO: Set proper license
 */

#include "MutableDeviceProperty.h"
#include "Laser.h"

NAMESPACE_COBOLT_BEGIN

MutableDeviceProperty::MutableDeviceProperty( const Property::Stereotype stereotype, const std::string& name, LaserDevice* laserDevice, const std::string& getCommand ) :
    DeviceProperty( stereotype, name, laserDevice, getCommand )
{}

int MutableDeviceProperty::IntroduceToGuiEnvironment( GuiEnvironment* )
{
    return return_code::ok;
}

bool MutableDeviceProperty::IsMutable() const
{
    return true;
}

int MutableDeviceProperty::OnGuiSetAction( GuiProperty& guiProperty )
{
    std::string value;

    guiProperty.Get( value );

    const int returnCode = Set( value );

    if ( returnCode != return_code::ok ) {

        Logger::Instance()->LogError( "MutableDeviceProperty[" + GetName() + "]::OnGuiSetAction( GuiProperty( '" + value + "' ) ): Failed" );
        SetToUnknownValue( guiProperty );
        return returnCode;
    }

    ClearCache();

    Logger::Instance()->LogMessage( "MutableDeviceProperty[" + GetName() + "]::OnGuiSetAction( GuiProperty( '" + value + "' ) ): Succeeded", true );

    guiProperty.Set( value );

    return return_code::ok;
}

NAMESPACE_COBOLT_END
