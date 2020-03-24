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
    void setupWith( FetchValueModifier* fetchValueModifier )
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
void FormatString( std::string& string );

template <> void FormatString<laser::flag::type>( std::string& string )
{
    if ( string == "1" ) {
        string = "O";
    }
}

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

        FormatString<T>( string );

        return result;
    }
    
private:

    const LaserDevice* laserDevice_;
    const std::string getCommand_;
};

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
    
    virtual void ExportConstraintToGui( GuiEnvironment* environment ) const
    {
        environment->RegisterAllowedGuiPropertyRange( min_, max_ );
    }

private:

    double min_;
    double max_;
};

NAMESPACE_COBOLT_END

#if 0

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

        return false;
    }

    bool LastRequestSuccessful() const { return lastRequestSuccessful_; }

protected:
    
    template <typename S> static S UnknownValue()           { return (S) -1; }
    template <>           static std::string UnknownValue() { return UnknownValue<const char*>(); }
    template <>           static const char* UnknownValue() { return "Unknown"; }
    
    void MarkRequestSuccessful() const { lastRequestSuccessful_ = true; }
    void MarkRequestFailed() const { lastRequestSuccessful_ = false; }
    
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
    mutable bool lastRequestSuccessful_;
};

template <typename T>
class MutableProperty : public Property<T>
{
public:

    MutableProperty( const char* name, LaserDevice* laserDevice ) :
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

class PausedProperty : public MutableProperty<bool>
{
public:

    PausedProperty( const char* name, LaserDevice* laserDevice ) :
        MutableProperty<bool>( name, laserDevice ),
        paused_( false )
    {}

    bool Fetch() const
    {
        MarkRequestSuccessful();
        return paused_;
    }

    virtual void Set( const bool& doPause )
    {
        std::string response;

        if ( doPause ) {
            laserDevice_->SendCommand( "l0r", response );
        } else {
            laserDevice_->SendCommand( "l1r", response );
        }
        
        if ( response == "OK" ) {
            MarkRequestSuccessful();
            paused_ = doPause;
        } else {
            MarkRequestFailed();
        }
    }

private:

    bool paused_;
};

class MaxCurrentSetpointProperty : public Property<double>
{
public:
    bool Fetch( double& ) const;
};

NAMESPACE_COBOLT_END

#endif

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