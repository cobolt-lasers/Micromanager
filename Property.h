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

class Laser;

typedef MM::PropertyBase GuiProperty;

/// ###
/// Property Interfaces

/**
 * T - Internal Type
 * S - Presented Type (in GUI)
 */
template <typename T>
class Property
{
public:

    class FetchValueModifier
    {
    public:

        virtual void ApplyOn( T& ) = 0;
    };

    Property( const char* name, LaserDevice* laserDevice ) :
        laserDevice_( laserDevice ),
        name_( name )
    {}

    virtual ~Property()
    {
        if ( modifier_ != NULL ) {
            delete modifier_;
        }
    }

    /**
     * \brief Attaches a modifier that will the fetched value (e.g. if a certain model returns
     *        amperes but want milliamperes).
     *
     * \attention Takes object ownership.
     */
    void setupWith( FetchValueModifier* fetchValueModifier )
    {
        if ( fetchValueModifier_ != NULL ) {
            delete fetchValueModifier_;
        }

        fetchValueModifier_ = fetchValueModifier;
    }

    const char* Name() const
    {
        return name_;
    }

    /**
     * \brief Retrieves the value from the laser and returns it if retrieval successful.
     */
    virtual T Fetch() const
    {
        return "";
    }

    bool FetchInto( GuiProperty& guiProperty ) const
    {
        switch ( guiProperty.GetType() ) {

            case MM::PropertyType::Float:   return FetchInto<double>      ( guiProperty );
            case MM::PropertyType::Integer: return FetchInto<long>        ( guiProperty );
            case MM::PropertyType::String:  return FetchInto<std::string> ( guiProperty );
        }

        const T value = Fetch();

        if ( !LastRequestSuccessful() ) {

            guiProperty.Set( UnknownValue );
            return false;
        }

        guiProperty.Set( convert_to<S>( value ) );

        return true;
    }

    virtual bool IsSupported() const { return true; }
    bool LastRequestSuccessful() const { return lastRequestResult_; }

protected:
    
    template <typename S> static S UnknownValue()           { return (S) -1; }
    template <>           static std::string UnknownValue() { return UnknownValue<const char*>(); }
    template <>           static const char* UnknownValue() { return "Unknown"; }
    
    void MarkRequestSuccessful() { lastRequestResult_ = true; }
    void MarkRequestFailed() { lastRequestResult_ = false; }
    
    LaserDevice* const laserDevice_;
    FetchValueModifier* fetchValueModifier_;

private:

    template <typename S>
    bool FetchInto( GuiProperty& guiProperty ) const
    {
        const T value = Fetch();

        if ( !LastRequestSuccessful() ) {

            guiProperty.Set( UnknownValue );
            return false;
        }

        guiProperty.Set( convert_to<S>( value ) );

        return true;
    }

    const char* const name_;
    bool lastRequestResult_;
};

template <typename T>
class MutableProperty : public Property<T>
{
public:

    MutableProperty( const char* name, LaserDevice* laserDevice) :
        Property<T>( name, laserDevice )
    {}

    virtual void Set( const T& value ) = 0;

    bool SetFrom( const GuiProperty& guiProperty )
    {
        switch ( p.GetType() ) {

            case MM::PropertyType::Float:   return SetFrom<double>      ( guiProperty );
            case MM::PropertyType::Integer: return SetFrom<long>        ( guiProperty );
            case MM::PropertyType::String:  return SetFrom<std::string> ( guiProperty );
        }
    }

private:

    template <typename S>
    bool SetFrom( const GuiProperty& guiProperty )
    {
        S guiPropertyValue;
        guiProperty->Get( guiPropertyValue );
        
        T value = convert_to<T>( guiPropertyValue );

        Set( value );

        if ( !LastRequestSuccessful() ) {
            guiProperty->Set( UnknownValue<S>() );
            return false;
        }

        return true;
    }
};

/// ###
/// Unsupported Property Stubs

template <typename T>
class UnsupportedProperty : public Property<T>
{
public:

    static UnsupportedProperty<T>* Instance();
    virtual bool IsSupported() const { return false; }

private:

    UnsupportedProperty( const char* name ) :
        Property<T>( name, T(), NULL )
    {}
};

template <typename T>
class UnsupportedMutableProperty : public MutableProperty<T>
{
public:

    static UnsupportedMutableProperty<T>* Instance();
    virtual bool IsSupported() const { return false; }

private:

    UnsupportedMutableProperty( const char* name ) :
        MutableProperty<T>( name, T(), NULL )
    {}

    virtual void Set( const T& value ) {}
};

/// ###
/// Default Property Implementations

template <typename T>
class DefaultProperty : public Property<T>
{
public:

    DefaultProperty( LaserDevice* laserDevice, const std::string& getCommand ) :
        Property<T>( laserDevice ),
        getCommand_( getCommand )
    {}

    virtual T Fetch() const
    {
        std::string value;
        
        if ( !laserDevice_->SendCommand( getCommand_, value ) ) {
            MarkRequestFailed();
            return T();
        }

        if ( fetchValueModifier_ != NULL ) {
            fetchValueModifier_->ApplyOn( value );
        }
        
        MarkRequestSuccessful();

        return value;
    }

private:

    const std::string getCommand_;
};

template <typename T>
class DefaultMutableProperty : public MutableProperty<T>
{
public:
    
    DefaultMutableProperty( LaserDevice* laserDevice, const std::string& getCommand, const std::string& setCommand ) :
        MutableProperty( laserDevice ),
        getCommand_( getCommand ),
        setCommand_( setCommand )
    {}
    
    virtual T Fetch() const
    {
        std::string value;
        
        if ( !laserDevice_->SendCommand( getCommand_, value ) ) {
            MarkRequestFailed();
            return T();
        }

        if ( fetchValueModifier_ != NULL ) {
            fetchValueModifier_->ApplyOn( value );
        }

        MarkRequestSuccessful();

        return value;
    }
    
    virtual void Set( const T& value )
    {
        std::string response;
        std::string preparedSetCommand = setCommand_ + " " + std::to_string( value );
        
        if ( !laserDevice_->SendCommand( preparedSetCommand, response ) ) {
            MarkRequestFailed();
            return;
        }
        
        MarkRequestSuccessful();
    }

private:

    const std::string getCommand_;
    const std::string setCommand_;
};

/// ###
/// Specific Property Implementations

class MaxCurrentSetpointProperty : public Property<double>
{
public:
    bool Fetch( double& ) const;
};

NAMESPACE_COBOLT_END

NAMESPACE_COBOLT_COMPATIBILITY_BEGIN( no_pause_command )

// TODO: Continue on request
//class PausedProperty : public MutableProperty<bool>
//{
//public:
//    
//    PausedProperty( Laser* );
//    virtual bool Fetch() const;
//    virtual void Set( const bool& value );
//
//private:
//
//    struct LaserState
//    {
//        std::string mode;
//        double powerSetpoint; // outputPowerSetting
//        double currentSetpoint; // driveCurrentSetting
//        bool digitalModulationActive; // digitalModActive
//        bool analogModulationActive; // analogModActive
//        double modulationPowerSetpoint; // modPowerSetting
//        std::string analogImpedance;
//    };
//
//    void Pause();
//    void Unpause();
//
//    Laser* laser_;
//
//    /**
//     * The state of the laser that we need to preserve between pause and unpause.
//     */
//    LaserState* laserState_;
//};

NAMESPACE_COBOLT_COMPATIBILITY_END

#endif // #ifndef __COBOLT__PROPERTY_H