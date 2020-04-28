/**
 * \file        MutableDeviceProperty.h
 *
 * \authors     Lukas Kalinski
 *
 * \copyright   Cobolt AB, 2020. All rights reserved. // TODO: Set proper license
 */

#ifndef __COBOLT__MUTABLE_DEVICE_PROPERTY_H
#define __COBOLT__MUTABLE_DEVICE_PROPERTY_H

#include "DeviceProperty.h"

NAMESPACE_COBOLT_BEGIN

class MutableDeviceProperty : public DeviceProperty
{
    typedef Property Parent;

public:

    MutableDeviceProperty( const Property::Stereotype stereotype, const std::string& name, LaserDevice* laserDevice, const std::string& getCommand );

    virtual int IntroduceToGuiEnvironment( GuiEnvironment* );
    virtual bool IsMutable() const;
    virtual int SetValue( const std::string& ) = 0;
    virtual int OnGuiSetAction( GuiProperty& guiProperty );
};

NAMESPACE_COBOLT_END

#endif // #ifndef __COBOLT__MUTABLE_DEVICE_PROPERTY_H
