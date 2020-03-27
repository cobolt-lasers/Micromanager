/**
 * \file        Laser.h
 *
 * \authors     Lukas Kalinski
 *
 * \copyright   Cobolt AB, 2020. All rights reserved. // TODO: Set proper license
 */

#ifndef __COBOLT__LASER_H
#define __COBOLT__LASER_H

#include <string>
#include <set>
#include <map>
#include <vector>

#include "cobolt.h"
#include "Logger.h"
#include "LaserDevice.h"
#include "Property.h"

NAMESPACE_COBOLT_BEGIN

namespace laser
{
    #define FOREACH_PROPERTY( GENERATOR )                                        \
        GENERATOR( model,                     1,   "Model"                     ) \
        GENERATOR( wavelength,                2,   "Wavelength"                ) \
        GENERATOR( serial_number,             3,   "Serial Number"             ) \
        GENERATOR( firmware_version,          4,   "Firmware Version"          ) \
        GENERATOR( operating_hours,           5,   "Operating Hours"           ) \
                                                                                 \
        GENERATOR( current_setpoint,          6,   "Current Setpoint"          ) \
        GENERATOR( max_current_setpoint,      7,   "Max Current Setpoint"      ) \
        GENERATOR( current_reading,           8,   "Measured Current"          ) \
        GENERATOR( power_setpoint,            9,   "Power Setpoint"            ) \
        GENERATOR( max_power_setpoint,        10,  "Max Power Setpoint"        ) \
                                                                                 \
        GENERATOR( power_reading,             11,  "Measured Power"            ) \
        GENERATOR( toggle,                    12,  "Status"                    ) \
        GENERATOR( paused,                    13,  "Paused"                    ) \
        GENERATOR( run_mode_cc_cp_mod,        14,  "Run Mode"                  ) \
        GENERATOR( digital_modulation_flag,   15,  "Digital Modulation"        ) \
                                                                                 \
        GENERATOR( analog_modulation_flag,    16,  "Analog Modulation"         ) \
        GENERATOR( modulation_power_setpoint, 17,  "Modulation Power Setpoint" ) \
        GENERATOR( analog_impedance,          18,  "Analog Impedance"          )
    
    namespace property { GENERATE_ENUM_STRING_MAP( FOREACH_PROPERTY ); }

    #undef FOREACH_PROPERTY
}

class Laser
{
public:

    typedef std::map<std::string, cobolt::Property*>::iterator PropertyIterator;

    static Laser* Create( const std::string& modelString );

    virtual ~Laser();

    const std::string& GetWavelength() const;

    void SetupWithLaserDevice( LaserDevice* );

    LaserDevice* GetDevice();

    void SetOn( const bool );
    void SetPaused( const bool );

    bool IsOn() const;
    bool IsPaused() const;

    Property* GetProperty( const std::string& name ) const;
    Property* GetProperty( const std::string& name );
    Property* GetProperty( const laser::property::symbol ) const;
    Property* GetProperty( const laser::property::symbol );

    PropertyIterator GetPropertyIteratorBegin();
    PropertyIterator GetPropertyIteratorEnd();

protected:

    Laser( const std::string& modelName );

    virtual bool SupportsModelSpecificProperty( const laser::property::symbol ) const = 0;
    
private:

    static void DecomposeModelString( std::string modelString, std::vector<std::string>& modelTokens );

    bool SupportsProperty( const laser::property::symbol ) const;

    void Incarnate();

    void RegisterProperty( const laser::property::symbol, cobolt::Property* property );
    void RegisterPropertyIfSupported( const laser::property::symbol, cobolt::Property* property );
    void AttachConstraintIfPropertySupported( const laser::property::symbol, cobolt::MutableProperty::Constraint* );

    std::map<std::string, cobolt::Property*> properties_;
    
    std::string modelName_;
    std::string wavelength_;
    LaserDevice* device_;
};

class Laser_06DPL : public Laser
{
public:

    Laser_06DPL() : Laser( "06 DPL" ) {}
    
    virtual bool SupportsModelSpecificProperty( const laser::property::symbol symbol ) const
    {
        switch ( symbol ) {

            case laser::property::model:
            case laser::property::wavelength:
            case laser::property::serial_number:
            case laser::property::firmware_version:
            case laser::property::operating_hours:
            case laser::property::current_setpoint:
            case laser::property::max_current_setpoint:
            case laser::property::current_reading:
            case laser::property::power_setpoint:
            case laser::property::max_power_setpoint:
            case laser::property::power_reading:
            case laser::property::toggle:
            case laser::property::paused:
            case laser::property::run_mode_cc_cp_mod:
            case laser::property::digital_modulation_flag:
            case laser::property::analog_modulation_flag:
            case laser::property::modulation_power_setpoint:
            case laser::property::analog_impedance:

                return true;

            default:

                return false;
        }
    }
};

class Laser_Unknown : public Laser
{
public:

    Laser_Unknown() : Laser( "Unknown" ) {}

    virtual bool SupportsModelSpecificProperty( const laser::property::symbol ) const
    {
        return false;
    }
};

NAMESPACE_COBOLT_END

#endif // #ifndef __COBOLT__LASER_H