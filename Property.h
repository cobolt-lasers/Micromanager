/**
 * \file        Property.h
 *
 * \authors     Lukas Kalinski
 *
 * \copyright   Cobolt AB, 2020. All rights reserved. // TODO: Set proper license
 */

#ifndef __COBOLT__PROPERTY_H
#define __COBOLT__PROPERTY_H

#include <string>

#include "../MMDevice/Property.h"

#include "cobolt.h"
#include "types.h"
#include "LaserDevice.h"

NAMESPACE_COBOLT_BEGIN

typedef MM::PropertyBase GuiProperty;
typedef MM::PropertyType GuiType;
typedef MM::ActionType GuiAction;

class GuiEnvironment
{
public:

    virtual int RegisterAllowedGuiPropertyValue( const char* propertyName, const char* value ) = 0;
    virtual int RegisterAllowedGuiPropertyRange( double min, double max ) = 0;
};

template <typename T>
bool ResponseStringToGuiPropertyValue( std::string& string );
const char* g_GuiPropertyValue_InvalidResponse = "Invalid Value";

template <> bool ResponseStringToGuiPropertyValue<std::string>( std::string& string )
{
    return true;
}

template <> bool ResponseStringToGuiPropertyValue<double>( std::string& string )
{
    return true;
}

template <> bool ResponseStringToGuiPropertyValue<laser::analog_impedance::type>( std::string& string )
{
    const int value = atoi( string.c_str() );
    if ( value != 0 && value != 1 ) { string = g_GuiPropertyValue_InvalidResponse; return false; }
    string = laser::analog_impedance::ToString( ( laser::analog_impedance::type )value );
    return true;
}

template <> bool ResponseStringToGuiPropertyValue<laser::flag::type>( std::string& string )
{
    const int value = atoi( string.c_str() );
    if ( value != 0 && value != 1 ) { string = g_GuiPropertyValue_InvalidResponse; return false; }
    string = laser::flag::ToString( ( laser::flag::type )value );
    return true;
}

template <> bool ResponseStringToGuiPropertyValue<laser::run_mode::type>( std::string& string )
{
    const int value = atoi( string.c_str() );
    if ( value != 0 && value != 1 ) { string = g_GuiPropertyValue_InvalidResponse; return false; }
    string = laser::run_mode::ToString( ( laser::run_mode::type )value );
    return true;
}

template <typename T>
bool GuiPropertyValueToCommandArgument( std::string& string );

template <> bool GuiPropertyValueToCommandArgument<std::string>( std::string& string )
{
    return true;
}

template <> bool GuiPropertyValueToCommandArgument<double>( std::string& string )
{
    return true;
}

template <> bool GuiPropertyValueToCommandArgument<laser::analog_impedance::type>( std::string& string )
{
    const laser::analog_impedance::type value = laser::analog_impedance::FromString( string );
    if ( value == laser::analog_impedance::__undefined__ ) { return false; }
    string = std::to_string( (_Longlong) value );
    return true;
}

template <> bool GuiPropertyValueToCommandArgument<laser::flag::type>( std::string& string )
{
    const laser::flag::type value = laser::flag::FromString( string );
    if ( value == laser::flag::__undefined__ ) { return false; }
    string = std::to_string( (_Longlong) value );
    return true;
}

template <> bool GuiPropertyValueToCommandArgument<laser::run_mode::type>( std::string& string )
{
    const laser::run_mode::type value = laser::run_mode::FromString( string );
    if ( value == laser::run_mode::__undefined__ ) { return false; }
    string = std::to_string( (_Longlong) value );
    return true;
}

class Property
{
public:

    class FetchValueModifier
    {
    public:

        virtual void ApplyOn( GuiProperty* guiProperty ) = 0;
    };

    Property( const char* name ) :
        name_( name ),
        fetchValueModifier_( NULL )
    {}

    virtual ~Property()
    {
        if ( fetchValueModifier_ != NULL ) {
            delete fetchValueModifier_;
        }
    }

    /**
     * \brief Attaches a modifier that will the fetched value (e.g. if a certain model returns
     *        amperes but want milliamperes).
     *
     * \attention Takes object ownership.
     */
    void SetupWith( FetchValueModifier* fetchValueModifier )
    {
        if ( fetchValueModifier_ != NULL ) {
            delete fetchValueModifier_;
        }

        fetchValueModifier_ = fetchValueModifier;
    }

    virtual int IntroduceToGuiEnvironment( GuiEnvironment* )
    {}

    const char* Name() const
    {
        return name_;
    }

    GuiType TypeInGui() const;

    virtual bool MutableInGui() const
    {
        return false;
    }

    virtual int OnGuiAction( GuiProperty* guiProperty, const GuiAction action )
    {
        std::string string;
        int result = FetchAsString( string );

        if ( result != DEVICE_OK ) {

            SetToUnknownValue( guiProperty );
            return result;
        }
        
        guiProperty->Set( string.c_str() );

        if ( fetchValueModifier_ != NULL ) {
            fetchValueModifier_->ApplyOn( guiProperty );
        }
        
        return result;
    }

    template <typename T> T Get() const;
    template <> double Get() const
    {
        std::string string;
        const int result = FetchAsString( string );
        
        if ( result != DEVICE_OK ) {
            return 0.0f;
        }
        
        return atof( string.c_str() );
    }


    virtual int FetchAsString( std::string& string ) const = 0;

protected:

    void SetToUnknownValue( GuiProperty* guiProperty ) const
    {
        switch ( TypeInGui() ) {
            case GuiType::Float:   guiProperty->Set( 0.0f );
            case GuiType::Integer: guiProperty->Set( 0L );
            case GuiType::String:  guiProperty->Set( "Unknown" );
        }
    }

private:

    const char* const name_;

    FetchValueModifier* fetchValueModifier_;
};

class MutableProperty : public Property
{
    typedef Property Parent;

public:

    class Constraint
    {
    public:

        virtual void ExportToGuiEnvironment( GuiEnvironment* ) const = 0;
    };

    MutableProperty( const char* name ) :
        Property( name ),
        constraint_( new NoConstraint() )
    {}

    virtual ~MutableProperty()
    {
        if ( constraint_ != NULL ) {
            delete constraint_;
        }
    }

    virtual int IntroduceToGuiEnvironment( GuiEnvironment* environment )
    {
        constraint_->ExportToGuiEnvironment( environment );
    }

    virtual bool MutableInGui() const
    {
        return true;
    }

    /**
     * \note Takes ownership of constraint.
     */
    void SetupWith( const Constraint* constraint )
    {
        if ( constraint_ != NULL ) {
            delete constraint_;
        }
        
        constraint_ = constraint;
    }

    virtual int SetFrom( GuiProperty* guiProperty ) = 0;

    virtual int OnGuiAction( GuiProperty* guiProperty, const GuiAction action )
    {
        if ( action == GuiAction::BeforeGet ) {

            return Parent::OnGuiAction( guiProperty, action );

        } else if ( action == GuiAction::AfterSet ) {

            return SetFrom( guiProperty );
        }
    }

private:

    class NoConstraint : public Constraint
    {
    public:

        virtual void ExportToGuiEnvironment( GuiEnvironment* ) const {}
    };

    const Constraint* constraint_;
};

template <typename T>
class DefaultProperty : public Property
{
public:

    DefaultProperty( const char* name, LaserDevice* laserDevice, const std::string& getCommand ) :
        Property( name ),
        laserDevice_( laserDevice ),
        getCommand_( getCommand )
    {}

    virtual int FetchAsString( std::string& string ) const
    {
        const int result = laserDevice_->SendCommand( getCommand_, string );

        ResponseStringToGuiPropertyValue<T>( string );

        return result;
    }
    
private:

    const LaserDevice* laserDevice_;
    const std::string getCommand_;
};

template <typename T>
class DefaultMutableProperty : public MutableProperty
{
public:

    DefaultMutableProperty( const char* name, LaserDevice* laserDevice, const std::string& getCommand, const std::string& setCommand ) :
        MutableProperty( name ),
        getCommand_( getCommand ),
        setCommand_( setCommand )
    {}

    virtual int FetchAsString( std::string& string ) const
    {
        return laserDevice_->SendCommand( getCommand_, string );
    }

    virtual int SetFrom( GuiProperty* guiProperty )
    {
        std::string response;
        std::string value;
        
        guiProperty->Get( value );

        if ( !GuiPropertyValueToCommandArgument<T>( value ) ) {
            return DEVICE_INVALID_PROPERTY_VALUE;
        }

        std::string preparedSetCommand = setCommand_ + " " + value;
        
        const int result = laserDevice_->SendCommand( preparedSetCommand, response );

        if ( result != DEVICE_OK ) {
            
            SetToUnknownValue( guiProperty );
            return result;
        }

        return DEVICE_OK;
    }

private:

    const LaserDevice* laserDevice_;

    const std::string getCommand_;
    const std::string setCommand_;
};

template <typename _EnumType>
class EnumConstraint : public MutableProperty::Constraint
{
public:

    EnumConstraint( const char* validValues[] ) :
        validValues_( validValues )
    {}

    virtual void ExportConstraintToGui( GuiEnvironment* environment ) const
    {
        for ( int i = 0; i < _EnumType::__count__; i++ ) {
            environment->RegisterAllowedGuiPropertyValue( validValues[ i ] );
        }
    }

private:

    const char* validValues_[];
};

class RangeConstraint : public MutableProperty::Constraint
{
public:

    RangeConstraint( const double min, const double max ) :
        min_( min ),
        max_( max )
    {}
    
    virtual void ExportToGuiEnvironment( GuiEnvironment* environment ) const
    {
        environment->RegisterAllowedGuiPropertyRange( min_, max_ );
    }

private:

    double min_;
    double max_;
};

NAMESPACE_COBOLT_END

#endif // #ifndef __COBOLT__PROPERTY_H