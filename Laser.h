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
        GENERATOR( current_reading,           7,   "Measured Current"          ) \
        GENERATOR( power_setpoint,            8,   "Power Setpoint"            ) \
        GENERATOR( power_reading,             9,   "Measured Power"            ) \
        GENERATOR( toggle,                    10,  "Status"                    ) \
                                                                                 \
        GENERATOR( paused,                    11,  "Paused"                    ) \
        GENERATOR( run_mode_cc_cp_mod,        12,  "Run Mode"                  ) \
        GENERATOR( digital_modulation_flag,   13,  "Digital Modulation"        ) \
        GENERATOR( analog_modulation_flag,    14,  "Analog Modulation"         ) \
        GENERATOR( modulation_power_setpoint, 15,  "Modulation Power Setpoint" ) \
                                                                                 \
        GENERATOR( analog_impedance,          16,  "Analog Impedance"          )
    
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

    virtual void CreateSpecificProperties() = 0;

    /// ###
    /// Overrideable Property Factories

    virtual void CreateModelProperty();
    virtual void CreateWavelengthProperty();
    virtual void CreateSerialNumberProperty();
    virtual void CreateFirmwareVersionProperty();
    virtual void CreateOperatingHoursProperty();

    virtual void CreateCurrentSetpointProperty();
    virtual void CreateCurrentReadingProperty();
    virtual void CreatePowerSetpointProperty();
    virtual void CreatePowerReadingProperty();
    virtual void CreateToggleProperty();

    virtual void CreatePausedProperty();
    virtual void CreateRunModeProperty();
    virtual void CreateDigitalModulationProperty();
    virtual void CreateAnalogModulationFlagProperty();
    virtual void CreateModulationPowerSetpointProperty();

    virtual void CreateAnalogImpedanceProperty();
    
private:

    enum unit_prefix_t { NoPrefix, Milli };

    static void DecomposeModelString( std::string modelString, std::vector<std::string>& modelTokens );

    void Initialize();

    bool HasProperty( const laser::property::symbol ) const;
    bool IsPauseCommandSupported();

    void CreateGenericProperties();

    void RegisterProperty( Property* );
    
    std::map<std::string, cobolt::Property*> properties_;
    
    std::string modelName_;
    std::string wavelength_;
    LaserDevice* device_;

    double maxCurrentSetpoint_;
    double maxPowerSetpoint_;

    unit_prefix_t currentUnitPrefix_;
    unit_prefix_t powerUnitPrefix_;
};

class Laser_06DPL : public Laser
{
public:

    Laser_06DPL() : Laser( "06 DPL" ) {}
    
    virtual void CreateSpecificProperties()
    {
        CreateModelProperty();
        CreateWavelengthProperty();
        CreateSerialNumberProperty();
        CreateFirmwareVersionProperty();
        CreateOperatingHoursProperty();
        CreateCurrentSetpointProperty();
        CreateCurrentReadingProperty();
        CreatePowerSetpointProperty();
        CreatePowerReadingProperty();
        CreateToggleProperty();
        CreatePausedProperty();
        CreateRunModeProperty();
        CreateDigitalModulationProperty();
        CreateAnalogModulationFlagProperty();
        CreateModulationPowerSetpointProperty();
        CreateAnalogImpedanceProperty();
    }
};

class Laser_Unknown : public Laser
{
public:

    Laser_Unknown() : Laser( "Unknown" ) {}
    
    virtual void CreateSpecificProperties() {}
};

NAMESPACE_COBOLT_END

#endif // #ifndef __COBOLT__LASER_H