/**
 * \file        EnumerationProperty.h
 *
 * \authors     Lukas Kalinski
 *
 * \copyright   Cobolt AB, 2020. All rights reserved. // TODO: Set proper license
 */

#ifndef __COBOLT__ENUMERATION_PROPERTY_H
#define __COBOLT__ENUMERATION_PROPERTY_H

#include "MutableDeviceProperty.h"

NAMESPACE_COBOLT_BEGIN

/**
 * Any (mutable) property that only can be set to one of a pre-defined set of values.
 */
class EnumerationProperty : public MutableDeviceProperty
{
    typedef MutableDeviceProperty Parent;

public:
    
    EnumerationProperty( const std::string& name, LaserDevice* laserDevice, const std::string& getCommand );

    virtual int IntroduceToGuiEnvironment( GuiEnvironment* environment );

    /**
     * \param deviceValue The response of the getCommand that corresponds to the enumeration item (e.g. 1 might be matched to 'enabled').
     * \param setCommand The set command (with argument, if applicable) to send when intending to set the property to the particular enumeration item.
     * \param name The name of the value (e.g. 'on' or 'enabled' or 'constant current'). Use it when presenting the property in the GUI.
     */
    void RegisterEnumerationItem( const std::string& deviceValue, const std::string& setCommand, const std::string& name );

    virtual int GetValue( std::string& string ) const;
    virtual int SetValue( const std::string& guiValue );

private:

    struct EnumerationItem
    {
        std::string deviceValue;
        std::string setCommand;
        std::string name;
    };

    typedef std::vector<EnumerationItem> enumeration_items_t;

    enumeration_items_t enumerationItems_;
};

NAMESPACE_COBOLT_END

#endif // #ifndef __COBOLT__ENUMERATION_PROPERTY_H
