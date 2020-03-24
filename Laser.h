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
    #define FOREACH_TOGGLE_VALUE( GENERATOR ) \
        GENERATOR( off, 0,  "Off" ) \
        GENERATOR( on,  1,  "On" )

    namespace toggle { GENERATE_ENUM_STRING_MAP( FOREACH_TOGGLE_VALUE ); }

    #undef FOREACH_TOGGLE_VALUE

    #define FOREACH_FLAG_VALUE( GENERATOR ) \
        GENERATOR( disabled,    0,  "Disabled" ) \
        GENERATOR( enabled,     1,  "Enabled" )

    namespace flag { GENERATE_ENUM_STRING_MAP( FOREACH_FLAG_VALUE ); }

    #undef FOREACH_FLAG_VALUE

    #define FOREACH_ANALOG_IMPEDANCE_VALUE( GENERATOR ) \
        GENERATOR( high,    0,  "1 kOhm" ) \
        GENERATOR( low,     1,  "50 Ohm" )

    namespace analog_impedance { GENERATE_ENUM_STRING_MAP( FOREACH_ANALOG_IMPEDANCE_VALUE ); }

    #undef FOREACH_ANALOG_IMPEDANCE_VALUE
    
    #define FOREACH_RUN_MODE_VALUE( GENERATOR ) \
        GENERATOR( constant_current, 0,   "Constant Current Mode" ) \
        GENERATOR( constant_power,   1,   "Constant Power Mode" ) \
        GENERATOR( modulation,       2,   "Modulation Mode" )

    namespace run_mode { GENERATE_ENUM_STRING_MAP( FOREACH_RUN_MODE_VALUE ); }

    #undef FOREACH_RUN_MODE_VALUE

    //#define FOREACH_PROPERTY( GENERATOR ) \
    //    GENERATOR( model,                       "Model"                     ) \
    //    GENERATOR( wavelength,                  "Wavelength"                ) \
    //    GENERATOR( serial_number,               "Serial Number"             ) \
    //    GENERATOR( firmware_version,            "Firmware Version"          ) \
    //    GENERATOR( operating_hours,             "Operating Hours"           ) \
    //    GENERATOR( current_setpoint,            "Current Setpoint"          ) \
    //    GENERATOR( max_current_setpoint,        "Max Current Setpoint"      ) \
    //    GENERATOR( power_setpoint,              "Power Setpoint"            ) \
    //    GENERATOR( max_power_setpoint,          "Max Power Setpoint"        ) \
    //    GENERATOR( power_reading,               "Power Reading"             ) \
    //    GENERATOR( toggle,                      "On Status"                 ) \
    //    GENERATOR( paused,                      "Paused"                    ) \
    //    GENERATOR( run_mode,                    "Run Mode"                  ) \
    //    GENERATOR( digital_modulation_flag,     "Digital Modulation"        ) \
    //    GENERATOR( analog_modulation_flag,      "Analog Modulation"         ) \
    //    GENERATOR( modulation_power_setpoint,   "Modulation Power Setpoint" ) \
    //    GENERATOR( analog_impedance,            "Analog Impedance"          )
    //
    //namespace property { GENERATE_ENUM_STRING_MAP( FOREACH_PROPERTY ); }

    //#undef FOREACH_PROPERTY

    namespace property
    {
        const char* model                        = "Model";
        const char* wavelength                   = "Wavelength";
        const char* serial_number                = "Serial Number";
        const char* firmware_version             = "Firmware Version";
        const char* operating_hours              = "Operating Hours";

        const char* current_setpoint             = "Current Setpoint";
        const char* max_current_setpoint         = "Max Current Setpoint";
        const char* current_reading              = "Measured Current";
        const char* power_setpoint               = "Power Setpoint";
        const char* max_power_setpoint           = "Max Power Setpoint";

        const char* power_reading                = "Measured Power";
        const char* toggle                       = "On Status";
        const char* paused                       = "Paused";
        const char* run_mode                     = "Run Mode";
        const char* digital_modulation_flag      = "Digital Modulation";

        const char* analog_modulation_flag       = "Analog Modulation";
        const char* modulation_power_setpoint    = "Modulation Power Setpoint";
        const char* analog_impedance             = "Analog Impedance";
    }
}

class Laser
{
public:

    typedef std::map<std::string, cobolt::Property*>::iterator PropertyIterator;

    Laser();
    virtual ~Laser();

    void SetupWithLaserDevice( LaserDevice* );

    LaserDevice* Device();

    Property* Property( const char* name )
    {
        return properties_[ name ];
    }

    PropertyIterator PropertyIteratorBegin()
    {
        return properties_.begin();
    }
    
    PropertyIterator PropertyIteratorEnd()
    {
        return properties_.begin();
    }

private:

    void ClearProperties();

    /**
     * Adapts the laser and its properties to the detected laser device or
     * falls back on default setup.
     */
    void Adapt();

    std::map<std::string, cobolt::Property*> properties_;

    LaserDevice* device_;
};

NAMESPACE_COBOLT_END

#endif // #ifndef __COBOLT__LASER_H