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

#include "base.h"
#include "LaserDevice.h"

NAMESPACE_COBOLT_BEGIN

/**
 * \brief The Micromanager GUI sometimes uses aliases for actual command argument values. This struct
 *        abstracts that difference away by bringing together two valid values as one value with two natures.
 */
struct StringValueMap
{
    std::string commandValue;   // Value accepted by laser serial interface.
    std::string guiValueAlias;  // Value as seen in the GUI.
};

inline bool operator == ( const std::string& lhs, const StringValueMap& rhs ) {  return ( lhs == rhs.guiValueAlias || lhs == rhs.commandValue ); }
inline bool operator != ( const std::string& lhs, const StringValueMap& rhs ) { return !( lhs != rhs ); }

namespace value
{
    namespace analog_impedance
    {
        extern StringValueMap high;
        extern StringValueMap low;
    }

    namespace flag
    {
        extern StringValueMap enable;
        extern StringValueMap disable;
    }

    namespace toggle
    {
        extern StringValueMap on;
        extern StringValueMap off;
    }
}

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
    
    Property( const std::string& name );

    virtual int IntroduceToGuiEnvironment( GuiEnvironment* );

    /**
     * \brief If caching is on, then the value will remain the same until it is
     *        changed on the Micromanager side. Thus properties that can change
     *        on laser side should NOT be cached.
     */
    void SetCaching( const bool enabled );

    const std::string& GetName() const;

    virtual Stereotype GetStereotype() const = 0;

    virtual bool IsMutable() const;
    virtual int OnGuiSetAction( GuiProperty& );
    virtual int OnGuiGetAction( GuiProperty& guiProperty );

    template <typename T> T Get() const;
    template <> double Get() const
    {
        std::string string;
        const int returnCode = FetchIntoAndCacheIfEnabled( string );
        
        if ( returnCode != return_code::ok ) {
            return 0.0f;
        }
        
        return atof( string.c_str() );
    }

    template <> std::string Get() const
    {
        std::string string;
        const int returnCode = FetchIntoAndCacheIfEnabled( string );

        if ( returnCode != return_code::ok ) {
            SetToUnknownValue( string );
        }
        
        return string;
    }

    virtual int FetchInto( std::string& string ) const = 0;

    /**
     * \brief The property object represented in a string. For logging/debug purposes.
     */
    virtual std::string ObjectString() const;

protected:

    void SetToUnknownValue( std::string& string ) const;
    
    void SetToUnknownValue( GuiProperty& guiProperty ) const;
    
    template <typename T> Stereotype ResolveStereotype() const { return String;  }
    template <> Stereotype ResolveStereotype<double>() const   { return Float;   }
    template <> Stereotype ResolveStereotype<int>() const      { return Integer; }

    void ClearCacheIfCachingEnabled() const;

private:

    int FetchIntoAndCacheIfEnabled( std::string& string ) const;

    std::string name_;

    bool doCache_;
    mutable std::string cachedValue_;
};

class MutableProperty : public Property
{
    typedef Property Parent;

public:

    MutableProperty( const std::string& name );

    virtual int IntroduceToGuiEnvironment( GuiEnvironment* );
    virtual bool IsMutable() const;
    int SetFrom( GuiProperty& guiProperty );
    virtual int Set( const std::string& ) = 0;
    virtual int OnGuiSetAction( GuiProperty& guiProperty );

protected:

    virtual bool IsValidValue( const std::string& ) const;
};

/**
 * \brief Represents a property whose value is set immediately on creation and will not change after that.
 */
class StaticStringProperty : public Property
{
public:

    StaticStringProperty( const std::string& name, const std::string& value );
    
    virtual Stereotype GetStereotype() const;
    virtual int FetchInto( std::string& string ) const;
    virtual std::string ObjectString() const;

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

    virtual int FetchInto( std::string& string ) const // Whatever is changed here should be replicated in BasicMutableProperty::FetchInto( ... )
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

    virtual int FetchInto( std::string& string ) const // Whatever is changed here should be replicated in BasicProperty::FetchInto( ... )
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
    
    EnumerationProperty( const std::string& name, LaserDevice* laserDevice, const std::string& getCommand, const std::string& setCommand );

    virtual int IntroduceToGuiEnvironment( GuiEnvironment* environment );
    void RegisterValidValue( const StringValueMap& validValue );
    virtual int FetchInto( std::string& string ) const;
    virtual int Set( const std::string& guiValue );

protected:

    virtual bool IsValidValue( const std::string& value ) const;

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
        const std::string& setTrueCommand, const std::string& setFalseCommand );

    virtual int Set( const std::string& value );
    virtual std::string ObjectString() const;

private:

    std::string setTrueCommand_;
    std::string setFalseCommand_;
};

class LaserPausedProperty : public BoolProperty
{
    typedef BoolProperty Parent;

public:

    LaserPausedProperty( const std::string& name, LaserDevice* laserDevice );
    
    virtual int FetchInto( std::string& string ) const;
    virtual int Set( const std::string& value );
    virtual std::string ObjectString() const;

private:

    std::string guiValue_;
};

class LaserSimulatedPausedProperty : public EnumerationProperty
{
public:

    LaserSimulatedPausedProperty( const std::string& name, LaserDevice* laserDevice );
    virtual ~LaserSimulatedPausedProperty();

    virtual int FetchInto( std::string& string ) const;
    virtual int Set( const std::string& value );
    
private:

    struct LaserState
    {
        std::string runMode;
        std::string currentSetpoint;
    };

    bool IsPaused() const;

    LaserState* savedLaserState_;
};

NAMESPACE_COBOLT_END

#endif // #ifndef __COBOLT__PROPERTY_H
