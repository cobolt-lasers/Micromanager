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

#include "base.h"
#include "Logger.h"
#include "LaserDevice.h"
#include "StaticStringProperty.h"
#include "DeviceProperty.h"
#include "MutableDeviceProperty.h"
#include "EnumerationProperty.h"
#include "NumericProperty.h"
#include "LaserShutterProperty.h"
#include "LegacyLaserShutterProperty.h"

NAMESPACE_COBOLT_BEGIN

class Laser
{
public:

    typedef std::map<std::string, cobolt::Property*>::iterator PropertyIterator;

    static Laser* Create( LaserDevice* device );

    virtual ~Laser();

    const std::string& GetId() const;
    const std::string& GetName() const;
    const std::string& GetWavelength() const;

    void SetOn( const bool );
    void SetShutterOpen( const bool );

    bool IsOn() const;
    bool IsPaused() const;

    Property* GetProperty( const std::string& name ) const;
    Property* GetProperty( const std::string& name );

    PropertyIterator GetPropertyIteratorBegin();
    PropertyIterator GetPropertyIteratorEnd();

private:

    enum Stereotype {

        ST_06_DPL,
        ST_06_MLD,
        ST_05_Series
    };

    static int NextId__;

    Laser( const std::string& name, const std::string& wavelength, LaserDevice* device );

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

    void CreateLaserOnOffProperty();
    void CreateShutterProperty();
    template <Stereotype T> void CreateRunModeProperty() {}
    template <> void CreateRunModeProperty<ST_05_Series>();
    template <> void CreateRunModeProperty<ST_06_DPL>();
    template <> void CreateRunModeProperty<ST_06_MLD>();
    void CreateDigitalModulationProperty();
    void CreateAnalogModulationFlagProperty();

    void CreateModulationPowerSetpointProperty();
    void CreateAnalogImpedanceProperty();
    
    static const std::string Milliamperes;
    static const std::string Amperes;
    static const std::string Milliwatts;
    static const std::string Watts;
                 
    static const std::string EnumerationItem_On;
    static const std::string EnumerationItem_Off;
    static const std::string EnumerationItem_Enabled;
    static const std::string EnumerationItem_Disabled;
                 
    static const std::string EnumerationItem_RunMode_ConstantCurrent;
    static const std::string EnumerationItem_RunMode_ConstantPower;
    static const std::string EnumerationItem_RunMode_Modulation;

    static void DecomposeModelString( std::string modelString, std::vector<std::string>& modelTokens );

    bool IsPauseCommandSupported();

    void CreateGenericProperties();

    void RegisterPublicProperty( Property* );
    
    std::map<std::string, cobolt::Property*> properties_;
    
    std::string id_;
    std::string name_;
    std::string wavelength_;
    LaserDevice* device_;

    std::string currentUnit_;
    std::string powerUnit_;

    MutableDeviceProperty* laserOnOffProperty;
    MutableDeviceProperty* pausedProperty_;
};

NAMESPACE_COBOLT_END

#endif // #ifndef __COBOLT__LASER_H