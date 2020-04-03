/**
 * \file        Property.cpp
 *
 * \authors     Lukas Kalinski
 *
 * \copyright   Cobolt AB, 2020. All rights reserved. // TODO: Set proper license
 */

#include "Property.h"
#include "Laser.h"

NAMESPACE_COBOLT_BEGIN

namespace value
{
    namespace analog_impedance
    {
        StringValueMap high = { "0", "1 kOhm" };
        StringValueMap low  = { "1", "50 Ohm" };
    }

    namespace flag
    {
        StringValueMap enable  = { "1", "Enable" };
        StringValueMap disable = { "0", "Disable" };
    }

    namespace toggle
    {
        StringValueMap on  = { "1", "On" };
        StringValueMap off = { "0", "Off" };
    }
}
/// ###
/// Property Class


Property::Property( const std::string& name ) :
    name_( name ),
    doCache_( true )
{}

int Property::IntroduceToGuiEnvironment( GuiEnvironment* )
{
    return return_code::ok;
}

/**
 * \brief If caching is on, then the value will remain the same until it is
 *        changed on the Micromanager side. Thus properties that can change
 *        on laser side should NOT be cached.
 */
void Property::SetCaching( const bool enabled )
{
    doCache_ = enabled;
}

const std::string& Property::GetName() const
{
    return name_;
}

bool Property::IsMutable() const
{
    return false;
}

int Property::OnGuiSetAction( GuiProperty& )
{
    Logger::Instance()->LogMessage( "Property[" + GetName() + "]::OnGuiSetAction(): Ignoring 'set' action on read-only property.", true );
    return return_code::ok;
}

int Property::OnGuiGetAction( GuiProperty& guiProperty )
{
    std::string string;
    int returnCode = FetchIntoAndCacheIfEnabled( string );

    if ( returnCode != return_code::ok ) {

        SetToUnknownValue( guiProperty );
        return returnCode;
    }

    guiProperty.Set( string.c_str() );

    return returnCode;
}

/**
 * \brief The property object represented in a string. For logging/debug purposes.
 */
std::string Property::ObjectString() const
{
    return "name_ = " + name_ + "; ";
}
    
void Property::SetToUnknownValue( std::string& string ) const
{
    string = "Unknown";
}

void Property::SetToUnknownValue( GuiProperty& guiProperty ) const
{
    switch ( GetStereotype() ) {
    case Float:   guiProperty.Set( "0" ); break;
    case Integer: guiProperty.Set( "0" ); break;
    case String:  guiProperty.Set( "Unknown" ); break;
    }
}

void Property::ClearCacheIfCachingEnabled() const
{
    cachedValue_.clear();
}

int Property::FetchIntoAndCacheIfEnabled( std::string& string ) const
{
    int returnCode = return_code::ok;

    if ( doCache_ ) {

        if ( cachedValue_.length() == 0 && doCache_ ) {
            returnCode = FetchInto( cachedValue_ );
        }

        if ( returnCode == return_code::ok ) {
            string = cachedValue_;
        } else {
            ClearCacheIfCachingEnabled();
        }

    } else {

        returnCode = FetchInto( string );
    }

    return returnCode;
}

/// ###
/// Mutable Property Class

MutableProperty::MutableProperty( const std::string& name ) :
    Property( name )
{}

int MutableProperty::IntroduceToGuiEnvironment( GuiEnvironment* )
{
    return return_code::ok;
}

bool MutableProperty::IsMutable() const
{
    return true;
}

int MutableProperty::SetFrom( GuiProperty& guiProperty )
{
    std::string value;

    guiProperty.Get( value );

    Logger::Instance()->LogMessage( "MutableProperty[" + GetName() + "]::SetFrom( GuiProperty( '" + value + "' ) )", true );

    const int returnCode = Set( value );

    if ( returnCode != return_code::ok ) {

        Logger::Instance()->LogError( "MutableProperty[" + GetName() + "]::SetFrom( GuiProperty( '" + value + "' ) ): Failed" );
        SetToUnknownValue( guiProperty );
        return returnCode;
    }

    ClearCacheIfCachingEnabled();

    Logger::Instance()->LogMessage( "MutableProperty[" + GetName() + "]::SetFrom( GuiProperty( '" + value + "' ) ): Succeeded", true );

    guiProperty.Set( value );

    return return_code::ok;
}

int MutableProperty::OnGuiSetAction( GuiProperty& guiProperty )
{
    return SetFrom( guiProperty );
}

bool MutableProperty::IsValidValue( const std::string& ) const
{
    return true;
}

/// ###
/// Statig String Property Class

/**
 * \brief Represents a property whose value is set immediately on creation and will not change after that.
 */

StaticStringProperty::StaticStringProperty( const std::string& name, const std::string& value ) :
    Property( name ),
    value_( value )
{}

Property::Stereotype StaticStringProperty::GetStereotype() const
{
    return String;
}

int StaticStringProperty::FetchInto( std::string& string ) const
{
    string = value_;
    return return_code::ok;
}

std::string StaticStringProperty::ObjectString() const
{
    return Property::ObjectString() + ( "value_ = " + value_ + "; " );
}

/// ###
/// Enumeration Property Class

EnumerationProperty::EnumerationProperty( const std::string& name, LaserDevice* laserDevice, const std::string& getCommand, const std::string& setCommand ) :
    BasicMutableProperty<std::string>( name, laserDevice, getCommand, setCommand )
{}

int EnumerationProperty::IntroduceToGuiEnvironment( GuiEnvironment* environment )
{
    for ( valid_values_t::const_iterator validValue = validValues_.begin();
        validValue != validValues_.end(); validValue++ ) {

        const int returnCode = environment->RegisterAllowedGuiPropertyValue( GetName(), validValue->guiValueAlias );
        if ( returnCode != return_code::ok ) {
            return returnCode;
        }

        Logger::Instance()->LogMessage( "EnumerationProperty[ " + GetName() + " ]::IntroduceToGuiEnvironment(): Registered valid value '" +
            validValue->guiValueAlias + "' in GUI.", true );
    }

    return return_code::ok;
}

void EnumerationProperty::RegisterValidValue( const StringValueMap& validValue )
{
    Logger::Instance()->LogMessage( "EnumerationProperty[ " + GetName() + " ]::RegisterValidValue( { '" +
        validValue.commandValue + "', '" + validValue.guiValueAlias + "' } )", true );
    validValues_.push_back( validValue );
}

int EnumerationProperty::FetchInto( std::string& string ) const
{
    std::string commandValue;
    Parent::FetchInto( commandValue );

    for ( valid_values_t::const_iterator validValue = validValues_.begin();
        validValue != validValues_.end(); validValue++ ) {

        if ( commandValue == validValue->commandValue ) {
            string = validValue->guiValueAlias;
            return return_code::ok;
        }
    }

    string = "Invalid Value";
    Logger::Instance()->LogError( "EnumerationProperty[" + GetName() + "]::FetchInto( ... ): No matching GUI value found for command value '" + commandValue + "'" );
    return return_code::error;
}

int EnumerationProperty::Set( const std::string& guiValue )
{
    for ( valid_values_t::const_iterator validValue = validValues_.begin();
        validValue != validValues_.end(); validValue++ ) {

        if ( guiValue == validValue->guiValueAlias ) {

            Parent::Set( validValue->commandValue );
            return return_code::ok;
        }
    }

    Logger::Instance()->LogError( "EnumerationProperty[ " + GetName() + " ]::Set(): Failed to interpret gui value '" + guiValue + "'" );
    return return_code::error;
}

bool EnumerationProperty::IsValidValue( const std::string& value ) const
{
    for ( valid_values_t::const_iterator validValue = validValues_.begin();
        validValue != validValues_.end(); validValue++ ) {

        // We interpret both the GUI and the command value as valid values:
        if ( validValue->guiValueAlias == value || validValue->commandValue == value ) {
            return true;
        }
    }

    return false;
}

/// ###
/// Bool Property Class

BoolProperty::BoolProperty( const std::string& name, LaserDevice* laserDevice,
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

int BoolProperty::Set( const std::string& value )
{
    if ( !IsValidValue( value ) ) {
        Logger::Instance()->LogError( "BoolProperty[ " + GetName() + " ]::Set( '" + value + "' ): Invalid value" );
        return return_code::invalid_property_value;
    }

    if ( value == value::toggle::on ) {
        return laserDevice_->SendCommand( setTrueCommand_ );
    } else {
        return laserDevice_->SendCommand( setFalseCommand_ );
    }
}

std::string BoolProperty::ObjectString() const
{
    return BasicMutableProperty<std::string>::ObjectString() + ( "setTrueCommand_ = " + setTrueCommand_ + "; setFalseCommand_ = " + setFalseCommand_ + "; " );
}

/// ###
/// Laser Paused Property Class

LaserPausedProperty::LaserPausedProperty( const std::string& name, LaserDevice* laserDevice ) :
    BoolProperty( name, laserDevice, BoolProperty::OnOff, "N/A", "l1r", "l0r" ),
    guiValue_( value::toggle::off.guiValueAlias )
{}

int LaserPausedProperty::FetchInto( std::string& string ) const
{
    string = guiValue_;
    return return_code::ok;
}

int LaserPausedProperty::Set( const std::string& value )
{
    const int returnCode = Parent::Set( value );

    if ( returnCode == return_code::ok ) {
        guiValue_ = value;
    }

    return returnCode;
}

std::string LaserPausedProperty::ObjectString() const
{
    return BoolProperty::ObjectString() + ( "guiValue_ = " + guiValue_ + "; " );
}

/// ###
/// Laser Simulated Paused Property Class

LaserSimulatedPausedProperty::LaserSimulatedPausedProperty( const std::string& name, LaserDevice* laserDevice ) :
    EnumerationProperty( name, laserDevice, "N/A", "N/A" ),
    savedLaserState_( NULL )
{
    RegisterValidValue( value::toggle::on );
    RegisterValidValue( value::toggle::off );
}

LaserSimulatedPausedProperty::~LaserSimulatedPausedProperty()
{
    if ( savedLaserState_ != NULL ) {
        delete savedLaserState_;
    }
}

int LaserSimulatedPausedProperty::FetchInto( std::string& string ) const
{
    if ( IsPaused() ) {
        string = value::toggle::on.guiValueAlias;
    } else {
        string = value::toggle::off.guiValueAlias;
    }

    return return_code::ok;
}

int LaserSimulatedPausedProperty::Set( const std::string& value )
{
    if ( !IsValidValue( value ) ) {
        Logger::Instance()->LogError( "LaserSimulatedPausedProperty[ " + GetName() + " ]::Set( '" + value + "' ): Invalid value" );
        return return_code::invalid_property_value;
    }

    int returnCode = return_code::ok;

    if ( value == value::toggle::on && !IsPaused() ) {

        savedLaserState_ = new LaserState();

        if ( laserDevice_->SendCommand( "glc?", &savedLaserState_->currentSetpoint ) != return_code::ok ||
            laserDevice_->SendCommand( "gam?", &savedLaserState_->runMode ) != return_code::ok ) {

            Logger::Instance()->LogError( "LaserSimulatedPausedProperty[ " + GetName() + " ]::Set( '" + value + "' ): Failed to save laser state" );
            return return_code::error;
        }

        returnCode = laserDevice_->SendCommand( "ecc" );
        if ( returnCode == return_code::ok ) {
            returnCode = laserDevice_->SendCommand( "slc 0" );
        }

    } else if ( IsPaused() ) {

        returnCode = laserDevice_->SendCommand( "sam " + savedLaserState_->runMode );
        if ( returnCode == return_code::ok ) {
            returnCode = laserDevice_->SendCommand( "slc " + savedLaserState_->currentSetpoint );
        }

        delete savedLaserState_;
        savedLaserState_ = NULL;

    } else {

        Logger::Instance()->LogMessage( "LaserSimulatedPausedProperty[" + GetName() + "]::Set( '" + value + "' ): Ignored request as requested state is already set", true );
    }

    return returnCode;
}

bool LaserSimulatedPausedProperty::IsPaused() const
{
    return ( savedLaserState_ != NULL );
}

NAMESPACE_COBOLT_END
