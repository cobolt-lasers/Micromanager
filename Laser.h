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
        GENERATOR( name,                      1,   "Name"                      ) \
        GENERATOR( model,                     2,   "Model"                     ) \
        GENERATOR( wavelength,                3,   "Wavelength"                ) \
        GENERATOR( serial_number,             4,   "Serial Number"             ) \
        GENERATOR( firmware_version,          5,   "Firmware Version"          ) \
                                                                                 \
        GENERATOR( operating_hours,           6,   "Operating Hours"           ) \
        GENERATOR( current_setpoint,          7,   "Current Setpoint"          ) \
        GENERATOR( current_reading,           8,   "Measured Current"          ) \
        GENERATOR( power_setpoint,            9,   "Power Setpoint"            ) \
        GENERATOR( power_reading,             10,  "Measured Power"            ) \
                                                                                 \
        GENERATOR( toggle,                    11,  "Status"                    ) \
        GENERATOR( paused,                    12,  "Paused"                    ) \
        GENERATOR( run_mode_cc_cp_mod,        13,  "Run Mode"                  ) \
        GENERATOR( digital_modulation_flag,   14,  "Digital Modulation"        ) \
        GENERATOR( analog_modulation_flag,    15,  "Analog Modulation"         ) \
                                                                                 \
        GENERATOR( modulation_power_setpoint, 16,  "Modulation Power Setpoint" ) \
        GENERATOR( analog_impedance,          17,  "Analog Impedance"          )
    
    namespace property { GENERATE_ENUM_STRING_MAP( FOREACH_PROPERTY ); }

    #undef FOREACH_PROPERTY
}

class Laser
{
public:

    typedef std::map<std::string, cobolt::Property*>::iterator PropertyIterator;

    static Laser* Create( LaserDevice* device );

    virtual ~Laser();

    const std::string& GetName() const;
    const std::string& GetWavelength() const;
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

    Laser( const std::string& name, LaserDevice* device );

    /// ###
    /// Overrideable Property Factories

    void CreateNameProperty();
    void CreateModelProperty();
    void CreateWavelengthProperty();
    void CreateSerialNumberProperty();
    void CreateFirmwareVersionProperty();

    void CreateOperatingHoursProperty();
    void CreateCurrentSetpointProperty();
    void CreateCurrentReadingProperty();
    void CreatePowerSetpointProperty();
    void CreatePowerReadingProperty();

    void CreateToggleProperty();
    void CreatePausedProperty();
    void CreateRunModeProperty();
    void CreateDigitalModulationProperty();
    void CreateAnalogModulationFlagProperty();

    void CreateModulationPowerSetpointProperty();
    void CreateAnalogImpedanceProperty();
    
private:

    enum unit_prefix_t { NoPrefix, Milli };

    static void DecomposeModelString( std::string modelString, std::vector<std::string>& modelTokens );

    bool HasProperty( const laser::property::symbol ) const;
    bool IsPauseCommandSupported();

    void CreateGenericProperties();

    void RegisterProperty( Property* );
    
    std::map<std::string, cobolt::Property*> properties_;
    
    std::string name_;
    std::string wavelength_;
    LaserDevice* device_;

    double maxCurrentSetpoint_;
    double maxPowerSetpoint_;

    unit_prefix_t currentUnitPrefix_;
    unit_prefix_t powerUnitPrefix_;
};

NAMESPACE_COBOLT_END

#endif // #ifndef __COBOLT__LASER_H