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
#include <vector>

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

    virtual int RegisterAllowedGuiPropertyValue( const std::string& propertyName, const std::string& value ) = 0;
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

    /**
     * \brief The property object represented in a string. For logging/debug purposes.
     */
    virtual std::string ObjectString() const
    {
        return "name_ = " + name_ + "; ";
    }

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
    
    template <typename T> Stereotype ResolveStereotype() const                              { return String;  } // Default // TODO: Do we need these functions?
    template <> Stereotype ResolveStereotype<std::string>() const                           { return String;  }
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

    MutableProperty( const std::string& name ) :
        Property( name )
    {}

    virtual int IntroduceToGuiEnvironment( GuiEnvironment* )
    {
        return return_code::ok;
    }

    virtual bool IsMutable() const
    {
        return true;
    }

    int SetFrom( GuiProperty& guiProperty )
    {
        std::string value;

        guiProperty.Get( value );
        
        Logger::Instance()->Log( "Attempting to set property '" + GetName() + "' to value=" + value, true );

        const int returnCode = Set( value );

        if ( returnCode != return_code::ok ) {

            Logger::Instance()->Log( "Failed set property '" + GetName() + "'", true );
            SetToUnknownValue( guiProperty );
            return returnCode;
        }

        Logger::Instance()->Log( "Successfully set property '" + GetName() + "' to value=" + Get<std::string>(), true );

        guiProperty.Set( value );

        return return_code::ok;
    }

    virtual int Set( const std::string& ) = 0;
    
    virtual int OnGuiSetAction( GuiProperty& guiProperty )
    {
        return SetFrom( guiProperty );
    }

protected:

    virtual bool IsValidValue( const std::string& ) const
    {
        return true;
    }
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

    virtual std::string ObjectString() const
    {
        return Property::ObjectString() + ( "value_ = " + value_ + "; " );
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
        return returnCode;
    }

    virtual std::string ObjectString() const
    {
        return Property::ObjectString() + ( "getCommand_ = " + getCommand_ + "; " );
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
        return returnCode;
    }

    virtual int Set( const std::string& value )
    {
        std::string argValue = value;
        
        std::string preparedSetCommand = setCommand_ + " " + argValue;
        return laserDevice_->SendCommand( preparedSetCommand );
    }

    virtual std::string ObjectString() const
    {
        return MutableProperty::ObjectString() + ( "getCommand_ = " + getCommand_ + "; setCommand_ = " + setCommand_ + "; " );
    }

protected:

    LaserDevice* laserDevice_;

private:

    std::string getCommand_;
    std::string setCommand_;
};

class EnumerationProperty : public BasicMutableProperty<std::string>
{
    typedef BasicMutableProperty<std::string> Parent;

public:
    
    EnumerationProperty( const std::string& name, LaserDevice* laserDevice, const std::string& getCommand, const std::string& setCommand ) :
        BasicMutableProperty<std::string>( name, laserDevice, getCommand, setCommand )
    {}

    virtual int IntroduceToGuiEnvironment( GuiEnvironment* environment )
    {
        for ( valid_values_t::const_iterator validValue = validValues_.begin();
              validValue != validValues_.end(); validValue++ ) {

            const int returnCode = environment->RegisterAllowedGuiPropertyValue( GetName(), validValue->guiValue );
            if ( returnCode != return_code::ok ) {
                return returnCode;
            }

            Logger::Instance()->Log( "EnumerationProperty::IntroduceToGuiEnvironment(): Registered valid value '" +
                validValue->guiValue + "' for property '" + GetName() + "'.", true );
        }

        return return_code::ok;
    }

    void RegisterValidValue( const StringValueMap& validValue )
    {
        validValues_.push_back( validValue );
    }

    virtual int FetchAsString( std::string& string ) const
    {
        std::string commandValue;
        Parent::FetchAsString( commandValue );

        for ( valid_values_t::const_iterator validValue = validValues_.begin();
              validValue != validValues_.end(); validValue++ ) {

            if ( commandValue == validValue->commandValue ) {
                string = validValue->guiValue;
                return return_code::ok;
            }
        }

        string = "Invalid Value";
        
        return return_code::error;
    }

    virtual int Set( const std::string& guiValue )
    {
        for ( valid_values_t::const_iterator validValue = validValues_.begin();
            validValue != validValues_.end(); validValue++ ) {

            if ( guiValue == validValue->guiValue ) {
                
                Parent::Set( validValue->commandValue );
                return return_code::ok;
            }
        }
        
        Logger::Instance()->Log( "EnumerationProperty::Set(): Failed to interpret gui value '" + guiValue + "'", true );
        return return_code::error;
    }

protected:

    virtual bool IsValidValue( const std::string& value ) const
    {
        for ( valid_values_t::const_iterator validValue = validValues_.begin();
            validValue != validValues_.end(); validValue++ ) {

            // We interpret both the GUI and the command value as valid values:
            if ( validValue->guiValue == value || validValue->commandValue == value ) {
                return true;
            }
        }
        
        return false;
    }

private:

    typedef std::vector<StringValueMap> valid_values_t;

    valid_values_t validValues_;
};

template <typename T>
class NumericProperty : public BasicMutableProperty<T>
{
public:

    NumericProperty( const std::string& name, LaserDevice* laserDevice, const std::string& getCommand, const std::string& setCommand, const T min, const T max ) :
        BasicMutableProperty<T>( name, laserDevice, getCommand, setCommand ),
        min_( min ),
        max_( max )
    {}

    virtual int IntroduceToGuiEnvironment( GuiEnvironment* environment )
    {
        return environment->RegisterAllowedGuiPropertyRange( GetName(), min_, max_ );
    }

protected:

    virtual bool IsValidValue( const std::string& value ) const
    {
        T numericValue = (T) atof( value.c_str() );
        return ( min_ <= numericValue && numericValue <= max_ );
    }

private:

    T min_;
    T max_;
};

class BoolProperty : public EnumerationProperty
{
public:

    enum stereotype_t { EnableDisable, OnOff };

    BoolProperty( const std::string& name, LaserDevice* laserDevice,
                  const stereotype_t stereotype, const std::string& getCommand,
                  const std::string& setTrueCommand, const std::string& setFalseCommand ) :
        EnumerationProperty( name, laserDevice, getCommand, "N/A" ),
        setTrueCommand_( setTrueCommand ),
        setFalseCommand_( setFalseCommand )
    {
        if ( stereotype == OnOff ) {

            RegisterValidValue( value::toggle::on );
            RegisterValidValue( value::toggle::off );

        } else {

            RegisterValidValue( value::flag::enable );
            RegisterValidValue( value::flag::disable );
        }
    }

    virtual int Set( const std::string& value )
    {
        if ( !IsValidValue( value ) ) {
            Logger::Instance()->Log( "Invalid value '" + value + "'", true );
            return return_code::invalid_property_value;
        }

        if ( value == value::toggle::on ) {
            return laserDevice_->SendCommand( setTrueCommand_ );
        } else {
            return laserDevice_->SendCommand( setFalseCommand_ );
        }
    }

    virtual std::string ObjectString() const
    {
        return BasicMutableProperty<std::string>::ObjectString() + ( "setTrueCommand_ = " + setTrueCommand_ + "; setFalseCommand_ = " + setFalseCommand_ + "; " );
    }

private:

    std::string setTrueCommand_;
    std::string setFalseCommand_;
};

class LaserPausedProperty : public BoolProperty
{
    typedef BoolProperty Parent;

public:

    LaserPausedProperty( const std::string& name, LaserDevice* laserDevice ) :
        BoolProperty( name, laserDevice, BoolProperty::OnOff, "N/A", "l1r", "l0r" ),
        guiValue_( value::toggle::off.guiValue )
    {}
    
    virtual int FetchAsString( std::string& string ) const
    {
        string = guiValue_;
        return return_code::ok;
    }

    virtual int Set( const std::string& value )
    {
        const int returnCode = Parent::Set( value );

        if ( returnCode == return_code::ok ) {
            guiValue_ = value;
        }
        
        return returnCode;
    }

    virtual std::string ObjectString() const
    {
        return BoolProperty::ObjectString() + ( "guiValue_ = " + guiValue_ + "; " );
    }

private:

    std::string guiValue_;
};

class LaserSimulatedPausedProperty : public EnumerationProperty
{
public:

    LaserSimulatedPausedProperty( const std::string& name, LaserDevice* laserDevice );
    virtual ~LaserSimulatedPausedProperty();

    virtual int FetchAsString( std::string& string ) const
    {
        if ( IsPaused() ) {
            string = value::toggle::on.guiValue;
        } else {
            string = value::toggle::off.guiValue;
        }
        
        return return_code::ok;
    }

    virtual int Set( const std::string& value )
    {
        if ( !IsValidValue( value ) ) {
            Logger::Instance()->Log( "Invalid value '" + value + "'", true );
            return return_code::invalid_property_value;
        }
        
        int returnCode = return_code::ok;

        if ( value == value::toggle::on && !IsPaused() ) {

            savedLaserState_ = new LaserState();

            if ( laserDevice_->SendCommand( "glc?", &savedLaserState_->currentSetpoint ) != return_code::ok ||
                 laserDevice_->SendCommand( "gam?", &savedLaserState_->runMode ) != return_code::ok ) {

                Logger::Instance()->Log( "LaserSimulatedPausedProperty::Set(): Failed to save laser state.", true );
                return return_code::error;
            }

            returnCode = laserDevice_->SendCommand( "cc" );
            if ( returnCode == return_code::ok ) {
                returnCode = laserDevice_->SendCommand( "slc 0" );
            }

        } else if ( IsPaused() ) {

            returnCode = laserDevice_->SendCommand( "sam " + savedLaserState_->runMode );
            if ( returnCode == return_code::ok ) {
                returnCode = laserDevice_->SendCommand( "slc " +savedLaserState_->currentSetpoint );
            }
            
            delete savedLaserState_;
            savedLaserState_ = NULL;

        } else {
            
            Logger::Instance()->Log( "LaserSimulatedPausedProperty::Set(): Ignored request as requested pause state is already set.", true );
        }
        
        return returnCode;
    }
    
private:

    struct LaserState
    {
        std::string runMode;
        std::string currentSetpoint;
    };

    bool IsPaused() const
    {
        return ( savedLaserState_ != NULL );
    }

    LaserState* savedLaserState_;
};

NAMESPACE_COBOLT_END

#endif // #ifndef __COBOLT__PROPERTY_H