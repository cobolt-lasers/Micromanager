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
        GENERATOR( on,  "On" ) \
        GENERATOR( off, "Off" )

    namespace toggle { GENERATE_ENUM_STRING( FOREACH_TOGGLE_VALUE ); }

    #undef FOREACH_TOGGLE_VALUE

    #define FOREACH_FLAG_VALUE( GENERATOR ) \
        GENERATOR( enabled,  "Enabled" ) \
        GENERATOR( disabled, "Disabled" )

    namespace flag { GENERATE_ENUM_STRING( FOREACH_FLAG_VALUE ); }

    #undef FOREACH_FLAG_VALUE

    #define FOREACH_ANALOG_IMPEDANCE_VALUE( GENERATOR ) \
        GENERATOR( low,  "50 Ohm" ) \
        GENERATOR( high, "1 kOhm" )

    namespace analog_impedance { GENERATE_ENUM_STRING( FOREACH_ANALOG_IMPEDANCE_VALUE ); }

    #undef FOREACH_ANALOG_IMPEDANCE_VALUE
    
    #define FOREACH_RUN_MODE_VALUE( GENERATOR ) \
        GENERATOR( constant_current,    "Constant Current Mode" ) \
        GENERATOR( constant_power,      "Constant Power Mode" ) \
        GENERATOR( modulation,          "Modulation Mode" )

    namespace run_mode { GENERATE_ENUM_STRING( FOREACH_RUN_MODE_VALUE ); }

    #undef FOREACH_RUN_MODE_VALUE
}

class Laser
{
public:

    Laser();

    void SetupWith( Logger* );
    void SetupWith( LaserDevice* );

    LaserDevice* Device();
    
    Property<std::string>*                   model;
    Property<int>*                           wavelength;
    Property<std::string>*                   serialNumber; // GetSerialNumber()
    Property<std::string>*                   firmwareVersion; // GetFirmwareVersion()
    Property<int>*                           operatingHours; // GetOperatingHours()
                                             
    MutableProperty<Current>*                currentSetpoint; // GetLaserDriveCurrent() / SetLaserDriveCurrent()
    Property<Current>*                       maxCurrentSetpoint; // GetLaserMaxCurrent()
                                             
    MutableProperty<Power>*                  powerSetpoint; // GetLaserPowerSetting() / SetLaserPowerSetting( ... )
    Property<Power>*                         maxPowerSetpoint;
    Property<Power>*                         powerReading; // GetLaserPowerOutput()
    
    MutableProperty<laser::toggle::type>*    on; // GetLaserStatus() / SetLaserStatus()
    MutableProperty<bool>*                   paused; // SetLaserPauseCommand()
    MutableProperty<laser::run_mode::type>*  runMode; // GetOperatingMode() / SetLaserOperatingMode()
                                             
    MutableProperty<laser::flag::type>*      digitalModulationState; // GetDigitalModulationState() / SetDigitalModulationState()
    MutableProperty<std::string>*            analogModulationState; // GetAnalogModulationState() / SetAnalogModulationState()
    MutableProperty<Power>*                  modulationPowerSetpoint; // GetModulationPowerSetting() / SetModulationPowerSetting()
    MutableProperty<std::string>*            analogImpedanceState; // GetAnalogImpedanceState() / SetAnalogImpedanceState()

private:

    /**
     * Adapts the laser and its properties to the detected laser device or
     * falls back on default setup.
     */
    void Adapt();

    Logger* logger_;
    LaserDevice* device_;
};

NAMESPACE_COBOLT_END

#endif // #ifndef __COBOLT__LASER_H