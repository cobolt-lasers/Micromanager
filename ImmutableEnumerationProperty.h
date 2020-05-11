/**
 * \file        ImmutableEnumerationProperty.h
 *
 * \authors     Lukas Kalinski
 *
 * \copyright   Cobolt AB, 2020. All rights reserved. // TODO: Set proper license
 */

#ifndef __COBOLT__IMMUTABLE_ENUMERATION_PROPERTY_H
#define __COBOLT__IMMUTABLE_ENUMERATION_PROPERTY_H

#include "DeviceProperty.h"

NAMESPACE_COBOLT_BEGIN

/**
 * Any (mutable) property that only can be set to one of a pre-defined set of values.
 */
class ImmutableEnumerationProperty : public DeviceProperty
{
    typedef DeviceProperty Parent;

public:
    
    ImmutableEnumerationProperty( const std::string& name, LaserDriver* laserDriver, const std::string& getCommand );

    /**
     * \param deviceValue The response of the getCommand that corresponds to the enumeration item (e.g. 1 might be matched to 'enabled').
     * \param name The name of the value (e.g. 'on' or 'enabled' or 'constant current'). Use it when presenting the property in the GUI.
     */
    void RegisterEnumerationItem( const std::string& deviceValue, const std::string& name );

    virtual int GetValue( std::string& string ) const;

private:

    std::string ResolveEnumerationItem( const std::string& deviceValue ) const;

    struct EnumerationItem
    {
        std::string deviceValue;
        std::string name;
    };

    typedef std::vector<EnumerationItem> enumeration_items_t;

    enumeration_items_t enumerationItems_;
};

NAMESPACE_COBOLT_END

#endif // #ifndef __COBOLT__IMMUTABLE_ENUMERATION_PROPERTY_H
