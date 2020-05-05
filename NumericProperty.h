/**
 * \file        Property.h
 *
 * \authors     Lukas Kalinski
 *
 * \copyright   Cobolt AB, 2020. All rights reserved. // TODO: Set proper license
 */

#ifndef __COBOLT__NUMERIC_PROPERTY_H
#define __COBOLT__NUMERIC_PROPERTY_H

#include "MutableDeviceProperty.h"

NAMESPACE_COBOLT_BEGIN

template <typename T>
class NumericProperty : public MutableDeviceProperty
{
public:

    NumericProperty( const std::string& name, LaserDriver* laserDriver, const std::string& getCommand, const std::string& setCommandBase, const T min, const T max ) :
        MutableDeviceProperty( ResolveStereotype<T>(), name, laserDriver, getCommand ),
        setCommandBase_( setCommandBase ),
        min_( min ),
        max_( max )
    {}

    virtual int IntroduceToGuiEnvironment( GuiEnvironment* environment )
    {
        return environment->RegisterAllowedGuiPropertyRange( GetName(), min_, max_ );
    }

    virtual int SetValue( const std::string& value )
    {
        if ( !IsValidValue( value ) ) {

            Logger::Instance()->LogError( "NumericProperty[" + GetName() + "]::SetValue( ... ): Invalid value '" + value + "'" );
            return return_code::invalid_value;
        }

        return laserDriver_->SendCommand( setCommandBase_ + " " + value );
    }
    
protected:

    bool IsValidValue( const std::string& value ) const
    {
        T numericValue = (T) atof( value.c_str() );
        return ( min_ <= numericValue && numericValue <= max_ );
    }

private:

    template <typename S>   static Property::Stereotype ResolveStereotype();
    template <>             static Property::Stereotype ResolveStereotype<int>() { return Property::Integer; }
    template <>             static Property::Stereotype ResolveStereotype<double>() { return Property::Float; }
    
    std::string setCommandBase_;

    T min_;
    T max_;
}; 

NAMESPACE_COBOLT_END

#endif // #ifndef __COBOLT__NUMERIC_PROPERTY_H
