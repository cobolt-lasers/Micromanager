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

#include "cobolt.h"
#include "Power.h"
#include "Current.h"
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
        GENERATOR( toggle,                    12,  "On Status"                 ) \
        GENERATOR( paused,                    13,  "Paused"                    ) \
        GENERATOR( run_mode_cc_cp_mod,        14,  "Run Mode"                  ) \
        GENERATOR( digital_modulation_flag,   15,  "Digital Modulation"        ) \
                                                                                 \
        GENERATOR( analog_modulation_flag,    16,  "Analog Modulation"         ) \
        GENERATOR( modulation_power_setpoint, 17,  "Modulation Power Setpoint" ) \
        GENERATOR( analog_impedance,          18,  "Analog Impedance"          )
    
    namespace property { GENERATE_ENUM_STRING_MAP( FOREACH_PROPERTY ); }

    #undef FOREACH_PROPERTY

    //namespace property
    //{
    //    const char* model                        = "Model";
    //    const char* wavelength                   = "Wavelength";
    //    const char* serial_number                = "Serial Number";
    //    const char* firmware_version             = "Firmware Version";
    //    const char* operating_hours              = "Operating Hours";

    //    const char* current_setpoint             = "Current Setpoint";
    //    const char* max_current_setpoint         = "Max Current Setpoint";
    //    const char* current_reading              = "Measured Current";
    //    const char* power_setpoint               = "Power Setpoint";
    //    const char* max_power_setpoint           = "Max Power Setpoint";

    //    const char* power_reading                = "Measured Power";
    //    const char* toggle                       = "On Status";
    //    const char* paused                       = "Paused";
    //    const char* run_mode                     = "Run Mode";
    //    const char* digital_modulation_flag      = "Digital Modulation";

    //    const char* analog_modulation_flag       = "Analog Modulation";
    //    const char* modulation_power_setpoint    = "Modulation Power Setpoint";
    //    const char* analog_impedance             = "Analog Impedance";
    //}
}

class Laser
{
public:

    typedef std::map<std::string, cobolt::Property*>::iterator PropertyIterator;

    Laser* Create( const std::string& modelString );

    virtual ~Laser();

    const std::string& Wavelength() const;

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

    void CreatePropertyIfSupported( const laser::property::symbol, cobolt::Property* property );
    void AttachConstraintIfPropertySupported( const laser::property::symbol, cobolt::MutableProperty::Constraint* );

    std::map<std::string, cobolt::Property*> properties_;
    
    const std::string modelName_;
    std::string wavelength_;
    LaserDevice* device_;
};

class Laser_06DPL : public Laser
{
public:

    Laser_06DPL() : Laser( "06 DPL" ) {}
    
    virtual bool SupportsModelSpecificProperty( laser::property::symbol symbol ) const
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

    virtual bool SupportsModelSpecificProperty( laser::property::symbol symbol ) const
    {
        return false;
    }
};

NAMESPACE_COBOLT_END

#endif // #ifndef __COBOLT__LASER_H