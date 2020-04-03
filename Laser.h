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
#include "Property.h"

NAMESPACE_COBOLT_BEGIN

class Laser
{
public:

    typedef std::map<std::string, cobolt::Property*>::iterator PropertyIterator;

    static Laser* Create( LaserDevice* device );

    virtual ~Laser();

    const std::string& GetName() const;
    const std::string& GetWavelength() const;

    void SetOn( const bool );
    void SetPaused( const bool );

    bool IsOn() const;
    bool IsPaused() const;

    Property* GetProperty( const std::string& name ) const;
    Property* GetProperty( const std::string& name );

    PropertyIterator GetPropertyIteratorBegin();
    PropertyIterator GetPropertyIteratorEnd();

protected:

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

    void CreateToggleProperty();
    void CreatePausedProperty();
    void CreateRunModeProperty( const std::vector<StringValueMap>& supportedRunModes );
    void CreateDigitalModulationProperty();
    void CreateAnalogModulationFlagProperty();

    void CreateModulationPowerSetpointProperty();
    void CreateAnalogImpedanceProperty();
    
private:

    static const char* Milliamperes;
    static const char* Amperes;
    static const char* Milliwatts;
    static const char* Watts;

    static void DecomposeModelString( std::string modelString, std::vector<std::string>& modelTokens );

    bool IsPauseCommandSupported();

    void CreateGenericProperties();

    void RegisterPublicProperty( Property* );
    
    std::map<std::string, cobolt::Property*> properties_;
    
    std::string name_;
    std::string wavelength_;
    LaserDevice* device_;

    std::string currentUnit_;
    std::string powerUnit_;

    MutableProperty* toggleProperty_;
    MutableProperty* pausedProperty_;
};

NAMESPACE_COBOLT_END

#endif // #ifndef __COBOLT__LASER_H