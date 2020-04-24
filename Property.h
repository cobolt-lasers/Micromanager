/**
 * \file        Property.h
 *
 * \authors     Lukas Kalinski
 *
 * \copyright   Cobolt AB, 2020. All rights reserved. // TODO: Set proper license
 */

#ifndef __COBOLT__PROPERTY_H
#define __COBOLT__PROPERTY_H

#include "base.h"
#include "LaserDevice.h"

NAMESPACE_COBOLT_BEGIN

/**
 * \brief The interface  the property hierarchy sees when receiving GUI events
 *        about property get/set.
 */
class GuiProperty
{
public:

    virtual bool Set( const std::string& ) = 0;
    virtual bool Get( std::string& ) const = 0;
};

/**
 * \brief A GUI environment interface to provide functionality to properly setup a cobolt::Property's
 *        corresponding GUI property.
 */
class GuiEnvironment
{
public:

    virtual int RegisterAllowedGuiPropertyValue( const std::string& propertyName, const std::string& value ) = 0;
    virtual int RegisterAllowedGuiPropertyRange( const std::string& propertyName, double min, double max ) = 0;
};

class Property
{
public:

    enum Stereotype { String, Float, Integer };
    
    Property( const Stereotype stereotype, const std::string& name );
    
    virtual int IntroduceToGuiEnvironment( GuiEnvironment* );

    const std::string& GetName() const;
    std::string GetValue() const;
    Stereotype GetStereotype() const;

    virtual bool IsMutable() const;
    virtual int OnGuiSetAction( GuiProperty& );
    virtual int OnGuiGetAction( GuiProperty& guiProperty );

    virtual int FetchInto( std::string& string ) const = 0;

    /**
     * \brief The property object represented in a string. For logging/debug purposes.
     */
    virtual std::string ObjectString() const;

protected:

    void SetToUnknownValue( std::string& string ) const;
    void SetToUnknownValue( GuiProperty& guiProperty ) const;
    
private:

    Stereotype stereotype_;
    std::string name_;
};

NAMESPACE_COBOLT_END

#endif // #ifndef __COBOLT__PROPERTY_H
