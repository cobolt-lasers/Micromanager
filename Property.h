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

#include "cobolt.h"
#include "types.h"
#include "LaserDevice.h"

NAMESPACE_COBOLT_BEGIN

class Laser;

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

    virtual int RegisterAllowedGuiPropertyValue( const std::string& propertyName, std::string& value ) = 0;
    virtual int RegisterAllowedGuiPropertyRange( const std::string& propertyName, double min, double max ) = 0;
};

class Property
{
public:

    enum Stereotype { String, Float, Integer };
    
    class FetchValueModifier // TODO: Deprecated?
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
    {
        return return_code::ok;
    }

    const std::string& GetName() const
    {
        return name_;
    }

    virtual Stereotype GetStereotype() const = 0;

    virtual bool IsMutable() const
    {
        return false;
    }

    virtual int OnGuiSetAction( GuiProperty& )
    {
        Logger::Instance()->Log( "Ignoring 'set' action on read-only property.", true );
        return return_code::ok;
    }

    virtual int OnGuiGetAction( GuiProperty& guiProperty )
    {
        std::string string;
        int returnCode = FetchAsString( string );

        if ( returnCode != return_code::ok ) {

            SetToUnknownValue( guiProperty );
            return returnCode;
        }
        
        guiProperty.Set( string.c_str() );

        if ( fetchValueModifier_ != NULL ) {
            fetchValueModifier_->ApplyOn( guiProperty );
        }
        
        return returnCode;
    }

    template <typename T> T Get() const;
    template <> double Get() const
    {
        std::string string;
        const int returnCode = FetchAsString( string );
        
        if ( returnCode != return_code::ok ) {
            return 0.0f;
        }
        
        return atof( string.c_str() );
    }

    template <> std::string Get() const
    {
        std::string string;
        const int returnCode = FetchAsString( string );

        if ( returnCode != return_code::ok ) {
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
    
    template <typename T> Stereotype ResolveStereotype() const                              { return String;  } // Default
    template <> Stereotype ResolveStereotype<std::string>() const                           { return String;  }
    template <> Stereotype ResolveStereotype<type::analog_impedance::symbol>() const       { return String;  }
    template <> Stereotype ResolveStereotype<type::flag::symbol>() const                   { return String;  }
    template <> Stereotype ResolveStereotype<type::run_mode::cc_cp_mod::symbol>() const    { return String;  }
    template <> Stereotype ResolveStereotype<type::toggle::symbol>() const                 { return String;  }
    template <> Stereotype ResolveStereotype<double>() const                                { return Float;   }
    template <> Stereotype ResolveStereotype<int>() const                                   { return Integer; }

private:

    std::string name_;

    FetchValueModifier* fetchValueModifier_;
};

class MutableProperty : public Property
{
    typedef Property Parent;

public:

    class Constraint
    {
    public:

        virtual int ExportToGuiEnvironment( const std::string& propertyName, GuiEnvironment* ) const = 0;
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
        constraint_->ExportToGuiEnvironment( GetName(), environment );
        return return_code::ok;
    }

    virtual bool IsMutable() const
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
        
        const int returnCode = Set( value );

        if ( returnCode != return_code::ok ) {

            SetToUnknownValue( guiProperty );
            return returnCode;
        }

        return return_code::ok;
    }

    virtual int Set( const std::string& ) = 0;
    
    virtual int OnGuiSetAction( GuiProperty& guiProperty )
    {
        return SetFrom( guiProperty );
    }

private:

    class NoConstraint : public Constraint
    {
    public:

        virtual int ExportToGuiEnvironment( const std::string&, GuiEnvironment* ) const { return return_code::ok; }
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
    
    virtual Stereotype GetStereotype() const
    {
        return String;
    }

    virtual int FetchAsString( std::string& string ) const
    {
        string = value_;
        return return_code::ok;
    }

private:

    std::string value_;
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

    virtual Stereotype GetStereotype() const
    {
        return ResolveStereotype<T>();
    }

    virtual int FetchAsString( std::string& string ) const // Whatever is changed here should be replicated in BasicMutableProperty::FetchAsString( ... )
    {
        int returnCode = laserDevice_->SendCommand( getCommand_, &string );
        if ( returnCode != return_code::ok ) { SetToUnknownValue( string ); return returnCode; }
        if ( !CommandResponseValueStringToGuiValueString<T>( string ) ) { returnCode = return_code::error; }
        return returnCode;
    }
    
private:

    LaserDevice* laserDevice_;
    std::string getCommand_;
};

template <typename T>
class BasicMutableProperty : public MutableProperty
{
public:

    BasicMutableProperty( const std::string& name, LaserDevice* laserDevice, const std::string& getCommand, const std::string& setCommand ) :
        MutableProperty( name ),
        laserDevice_( laserDevice ),
        getCommand_( getCommand ),
        setCommand_( setCommand )
    {}

    virtual Stereotype GetStereotype() const
    {
        return ResolveStereotype<T>();
    }

    virtual int FetchAsString( std::string& string ) const // Whatever is changed here should be replicated in BasicProperty::FetchAsString( ... )
    {
        int returnCode = laserDevice_->SendCommand( getCommand_, &string );
        if ( returnCode != return_code::ok ) { SetToUnknownValue( string ); return returnCode; }
        if ( !CommandResponseValueStringToGuiValueString<T>( string ) ) { returnCode = return_code::error; }
        return returnCode;
    }

    virtual int Set( const std::string& value )
    {
        std::string argValue = value;
        
        if ( !GuiValueStringToCommandArgumentString<T>( argValue ) ) {
            return return_code::invalid_property_value;
        }
        
        std::string preparedSetCommand = setCommand_ + " " + argValue;
        return laserDevice_->SendCommand( preparedSetCommand );
    }

protected:

    LaserDevice* laserDevice_;

private:

    std::string getCommand_;
    std::string setCommand_;
};

class ToggleProperty : public BasicMutableProperty<type::toggle::symbol>
{
public:

    ToggleProperty( const std::string& name, LaserDevice* laserDevice, const std::string& getCommand, const std::string& onCommand, const std::string& offCommand ) :
        BasicMutableProperty<type::toggle::symbol>( name, laserDevice, getCommand, "N/A" ),
        onCommand_( onCommand ),
        offCommand_( offCommand )
    {}

    virtual int Set( const std::string& value )
    {
        type::toggle::symbol toggleSymbol = type::toggle::FromString( value );

        if ( toggleSymbol == type::toggle::__undefined__ ) {
            return return_code::invalid_property_value;
        }
        
        switch ( toggleSymbol ) {

            case type::toggle::on:  return laserDevice_->SendCommand( onCommand_ );
            case type::toggle::off: return laserDevice_->SendCommand( offCommand_ );
        }
        
        return return_code::error;
    }

private:

    std::string onCommand_;
    std::string offCommand_;
};

class LaserPausedProperty : public ToggleProperty
{
    typedef ToggleProperty Parent;

public:

    LaserPausedProperty( const std::string& name, LaserDevice* laserDevice ) :
        ToggleProperty( name, laserDevice, "N/A", "l1r", "l0r" ),
        toggle_( type::toggle::ToString( type::toggle::off ) )
    {}
    
    virtual int FetchAsString( std::string& string ) const
    {
        string = toggle_;
        return return_code::ok;
    }

    virtual int Set( const std::string& value )
    {
        const int returnCode = Parent::Set( value );

        if ( returnCode == return_code::ok ) {
            toggle_ = value;
        }
        
        return returnCode;
    }
    
private:

    std::string toggle_;
};

class LaserSimulatedPausedProperty : public BasicMutableProperty<type::toggle::symbol>
{
public:
    
    LaserSimulatedPausedProperty( const std::string& name, Laser* laser );

    virtual int FetchAsString( std::string& string ) const
    {
        string = toggle_;
        return return_code::ok;
    }

    virtual int Set( const std::string& value )
    {
        type::toggle::symbol toggleSymbol = type::toggle::FromString( value );

        if ( toggleSymbol == type::toggle::__undefined__ ) {
            return return_code::invalid_property_value;
        }

        if ( value == toggle_ ) {
            return return_code::ok;
        }

        int returnCode = return_code::ok;
        switch ( toggleSymbol ) {

            // TODO NOW: Save and load laser state

            case type::toggle::on:
                returnCode = laserDevice_->SendCommand( "cc" );
                if ( returnCode == return_code::ok ) {
                    returnCode = laserDevice_->SendCommand( "slc 0" );
                }
                break;

            case type::toggle::off:
                returnCode = laserDevice_->SendCommand( "cc" );
                if ( returnCode == return_code::ok ) {
                    returnCode = laserDevice_->SendCommand( "slc 0" );
                }
                break;
        }

        return returnCode;
    }

private:

    struct LaserState
    {
        std::string runMode_;
        std::string a;
    };

    Laser* laser_;
    std::string toggle_;
};

template <typename _EnumType>
class EnumConstraint : public MutableProperty::Constraint
{
public:

    EnumConstraint( std::string* validValues, const int count ) :
        validValues_( validValues ),
        count_( count )
    {}

    virtual int ExportToGuiEnvironment( const std::string& propertyName, GuiEnvironment* environment ) const
    {
        for ( int i = 0; i < count_; i++ ) {
            const int returnCode = environment->RegisterAllowedGuiPropertyValue( propertyName, validValues_[ i ] );
            if ( returnCode != return_code::ok ) {
                return returnCode;
            }
        }
        
        return return_code::ok;
    }

private:

    std::string* validValues_;
    int count_;
};

class RangeConstraint : public MutableProperty::Constraint
{
public:

    RangeConstraint( double min, const double max ) :
        min_( min ),
        max_( max )
    {}
    
    virtual int ExportToGuiEnvironment( const std::string& propertyName, GuiEnvironment* environment ) const
    {
        return environment->RegisterAllowedGuiPropertyRange( propertyName, min_, max_ );
    }

private:

    double min_;
    double max_;
};

NAMESPACE_COBOLT_END

#endif // #ifndef __COBOLT__PROPERTY_H