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

    virtual int RegisterAllowedGuiPropertyValue( const char* propertyName, const char* value ) = 0;
    virtual int RegisterAllowedGuiPropertyRange( double min, double max ) = 0;
};

class Property
{
public:

    enum Stereotype { String, Float, Integer };
    
    class FetchValueModifier
    {
    public:

        virtual void ApplyOn( GuiProperty& guiProperty ) = 0;
    };

    Property( const std::string& name ) :
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

    const std::string& Name() const
    {
        return name_;
    }

    Stereotype GetStereotype() const;

    virtual bool MutableInGui() const
    {
        return false;
    }

    virtual int OnGuiSetAction( GuiProperty& guiProperty )
    {
        Logger::Instance()->Log( "Ignoring 'set' action on read-only property.", true );
        return DEVICE_OK;
    }

    virtual int OnGuiGetAction( GuiProperty& guiProperty )
    {
        std::string string;
        int result = FetchAsString( string );

        if ( result != DEVICE_OK ) {

            SetToUnknownValue( guiProperty );
            return result;
        }
        
        guiProperty.Set( string.c_str() );

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

    template <> std::string Get() const
    {
        std::string string;
        const int result = FetchAsString( string );

        if ( result != DEVICE_OK ) {
            SetToUnknownValue( string );
        }
        
        return string;
    }

    virtual int FetchAsString( std::string& string ) const = 0;

protected:

    void SetToUnknownValue( std::string& string ) const
    {
        string = "Unknown";
    }
    
    void SetToUnknownValue( GuiProperty& guiProperty ) const
    {
        switch ( GetStereotype() ) {
            case Float:   guiProperty.Set( "0" ); break;
            case Integer: guiProperty.Set( "0" ); break;
            case String:  guiProperty.Set( "Unknown" ); break;
        }
    }
    
private:

    const std::string name_;

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

    MutableProperty( const std::string& name ) :
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

    int SetFrom( GuiProperty& guiProperty )
    {
        std::string value;

        guiProperty.Get( value );
        
        if ( !FormatBeforeSetFromGui( value ) ) {
            return DEVICE_INVALID_PROPERTY_VALUE;
        }

        const int result = Set( value );

        if ( result != DEVICE_OK ) {

            SetToUnknownValue( guiProperty );
            return result;
        }

        return DEVICE_OK;
    }

    virtual int FormatBeforeSetFromGui( std::string& ) const = 0;
    virtual int Set( const std::string& ) = 0;
    
    virtual int OnGuiSetAction( GuiProperty& guiProperty )
    {
        return SetFrom( guiProperty );
    }

private:

    class NoConstraint : public Constraint
    {
    public:

        virtual void ExportToGuiEnvironment( GuiEnvironment* ) const {}
    };

    const Constraint* constraint_;
};

/**
 * \brief Represents a property whose value is set immediately on creation and will not change after that.
 */
class StaticStringProperty : public Property
{
public:

    StaticStringProperty( const std::string& name, const std::string& value ) :
        Property( name ),
        value_( value )
    {}
    
    virtual int FetchAsString( std::string& string ) const
    {
        string = value_;
        return DEVICE_OK;
    }

private:

    const std::string value_;
};

template <typename T>
class BasicProperty : public Property
{
public:

    BasicProperty( const std::string& name, LaserDevice* laserDevice, const std::string& getCommand ) :
        Property( name ),
        laserDevice_( laserDevice ),
        getCommand_( getCommand )
    {}

    virtual int FetchAsString( std::string& string ) const // Whatever is changed here should be replicated in BasicMutableProperty::FetchAsString( ... )
    {
        const int result = laserDevice_->SendCommand( getCommand_, &string );
        if ( result != DEVICE_OK ) { SetToUnknownValue( string ); return result; }
        CommandResponseValueStringToGuiValueString<T>( string );
        return result;
    }
    
private:

    const LaserDevice* laserDevice_;
    const std::string getCommand_;
};

template <typename T>
class BasicMutableProperty : public MutableProperty
{
public:

    BasicMutableProperty( const std::string& name, LaserDevice* laserDevice, const std::string& getCommand, const std::string& setCommand ) :
        MutableProperty( name ),
        getCommand_( getCommand ),
        setCommand_( setCommand )
    {}

    virtual int FetchAsString( std::string& string ) const // Whatever is changed here should be replicated in BasicProperty::FetchAsString( ... )
    {
        const int result = laserDevice_->SendCommand( getCommand_, &string );
        if ( result != DEVICE_OK ) { SetToUnknownValue( string ); return result; }
        CommandResponseValueStringToGuiValueString<T>( string );
        return result;
    }

    virtual int FormatBeforeSetFromGui( std::string& string ) const
    {
        return GuiValueStringToCommandArgumentString<T>( string );
    }

    virtual int Set( const std::string& value )
    {
        std::string preparedSetCommand = setCommand_ + " " + value;
        return laserDevice_->SendCommand( preparedSetCommand );
    }

protected:

    const LaserDevice* laserDevice_;

private:

    const std::string getCommand_;
    const std::string setCommand_;
};

class ToggleProperty : public BasicMutableProperty<laser::toggle::symbol>
{
public:

    ToggleProperty( const std::string& name, LaserDevice* laserDevice, const std::string& getCommand, const std::string& onCommand, const std::string& offCommand ) :
        BasicMutableProperty<laser::toggle::symbol>( name, laserDevice, getCommand, "N/A" ),
        onCommand_( onCommand ),
        offCommand_( offCommand )
    {}

    virtual int Set( const std::string& value )
    {
        laser::toggle::symbol toggleSymbol = laser::toggle::FromString( value );

        if ( toggleSymbol == laser::toggle::__undefined__ ) {
            return DEVICE_INVALID_PROPERTY_VALUE;
        }
        
        switch ( toggleSymbol ) {

            case laser::toggle::on:  return laserDevice_->SendCommand( onCommand_ );
            case laser::toggle::off: return laserDevice_->SendCommand( offCommand_ );
        }
        
        return DEVICE_ERR;
    }

private:

    const std::string& onCommand_;
    const std::string& offCommand_;
};

class LaserPausedProperty : public ToggleProperty
{
    typedef ToggleProperty Parent;

public:

    LaserPausedProperty( const std::string& name, LaserDevice* laserDevice ) :
        ToggleProperty( name, laserDevice, "N/A", "l1r", "l0r" ),
        toggle_( laser::toggle::ToString( laser::toggle::off ) )
    {}
    
    virtual int FetchAsString( std::string& string ) const
    {
        string = toggle_;
        return DEVICE_OK;
    }

    virtual int Set( const std::string& value )
    {
        if ( Parent::Set( value ) == DEVICE_OK ) {
            toggle_ = value;
        }
    }
    
private:

    std::string toggle_;
};

template <typename _EnumType>
class EnumConstraint : public MutableProperty::Constraint
{
public:

    EnumConstraint( std::string validValues[] ) :
        validValues_( validValues )
    {}

    virtual void ExportToGuiEnvironment( GuiEnvironment* environment ) const
    {
        for ( int i = 0; i < _EnumType::__count__; i++ ) {
            environment->RegisterAllowedGuiPropertyValue( validValues[ i ] );
        }
    }

private:

    std::string validValues_[];
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