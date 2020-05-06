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
#include "Property.h"

NAMESPACE_COBOLT_BEGIN

class LaserDriver;
class LaserStateProperty;
class MutableDeviceProperty;

class Laser
{
public:

    typedef std::map<std::string, cobolt::Property*>::iterator PropertyIterator;

    static Laser* Create( LaserDriver* driver );

    virtual ~Laser();

    const std::string& GetId() const;
    const std::string& GetName() const;
    const std::string& GetWavelength() const;

    void SetOn( const bool );
    void SetShutterOpen( const bool );

    bool IsShutterEnabled() const;
    
    bool IsShutterOpen() const;

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

    Laser( const std::string& name, const std::string& wavelength, LaserDriver* device );

    void RegisterState( const std::string& state );


    /// ###
    /// Property Factories

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

    template<Stereotype T> void CreateLaserStateProperty();
    template<> void CreateLaserStateProperty<ST_05_Series>();
    template<> void CreateLaserStateProperty<ST_06_DPL>();
    template<> void CreateLaserStateProperty<ST_06_MLD>();

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

    bool IsShutterCommandSupported();

    void CreateGenericProperties();

    void RegisterPublicProperty( Property* );
    
    std::map<std::string, cobolt::Property*> properties_;
    
    std::string id_;
    std::string name_;
    std::string wavelength_;
    LaserDriver* laserDriver_;

    std::string currentUnit_;
    std::string powerUnit_;

    LaserStateProperty* laserStateProperty_;
    MutableDeviceProperty* laserOnOffProperty_;
    MutableDeviceProperty* shutter_;
};

NAMESPACE_COBOLT_END

#endif // #ifndef __COBOLT__LASER_H