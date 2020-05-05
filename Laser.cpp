/**
 * \file        Laser.cpp
 *
 * \authors     Lukas Kalinski
 *
 * \copyright   Cobolt AB, 2020. All rights reserved. // TODO: Set proper license
 */

#include <assert.h>
#include "Laser.h"
#include "Logger.h"

#include "LaserDriver.h"
#include "StaticStringProperty.h"
#include "DeviceProperty.h"
#include "MutableDeviceProperty.h"
#include "EnumerationProperty.h"
#include "NumericProperty.h"
#include "LaserShutterProperty.h"
#include "NoShutterCommandLegacyFix.h"

using namespace std;
using namespace cobolt;

const std::string Laser::Milliamperes = "mA";
const std::string Laser::Amperes = "A";
const std::string Laser::Milliwatts = "mW";
const std::string Laser::Watts = "W";

const std::string Laser::EnumerationItem_On = "on";
const std::string Laser::EnumerationItem_Off = "off";
const std::string Laser::EnumerationItem_Enabled = "enabled";
const std::string Laser::EnumerationItem_Disabled = "disabled";

const std::string Laser::EnumerationItem_RunMode_ConstantCurrent = "Constant Current";
const std::string Laser::EnumerationItem_RunMode_ConstantPower = "Constant Power";
const std::string Laser::EnumerationItem_RunMode_Modulation = "Modulation";

int Laser::NextId__ = 1;

Laser* Laser::Create( LaserDriver* driver )
{
    assert( driver != NULL );
    
    std::string modelString;
    if ( driver->SendCommand( "glm?", &modelString ) != return_code::ok ) {
        return NULL;
    }
    
    std::vector<std::string> modelTokens;
    DecomposeModelString( modelString, modelTokens );
    std::string wavelength = "Unknown";
    
    if ( modelTokens.size() > 0 ) {
        wavelength = std::to_string( (_Longlong) atoi( modelTokens[ 0 ].c_str() ) );
    }

    Laser* laser;

    if ( modelString.find( "-06-91-" ) != std::string::npos ) {

        laser = new Laser( "06-DPL", wavelength, driver );

        laser->currentUnit_ = Milliamperes; 
        laser->powerUnit_ = Milliwatts;

        laser->CreateNameProperty();
        laser->CreateModelProperty();
        laser->CreateFirmwareVersionProperty();
        laser->CreateWavelengthProperty();
        laser->CreateLaserOnOffProperty();
        laser->CreateShutterProperty();
        laser->CreateRunModeProperty<ST_06_DPL>();
        laser->CreatePowerSetpointProperty();
        laser->CreatePowerReadingProperty();
        laser->CreateCurrentSetpointProperty();
        laser->CreateCurrentReadingProperty();
        laser->CreateDigitalModulationProperty();
        laser->CreateAnalogModulationFlagProperty();
        laser->CreateOperatingHoursProperty();
        laser->CreateSerialNumberProperty();

    } else if ( modelString.find( "-06-01-" ) != std::string::npos ||
                modelString.find( "-06-03-" ) != std::string::npos ) {

        laser = new Laser( "06-MLD", wavelength, driver );

        laser->currentUnit_ = Milliamperes;
        laser->powerUnit_ = Milliwatts;

        laser->CreateNameProperty();
        laser->CreateModelProperty();
        laser->CreateFirmwareVersionProperty();
        laser->CreateWavelengthProperty();
        laser->CreateLaserOnOffProperty();
        laser->CreateShutterProperty();
        laser->CreateRunModeProperty<ST_06_MLD>();
        laser->CreatePowerSetpointProperty();
        laser->CreatePowerReadingProperty();
        laser->CreateCurrentSetpointProperty();
        laser->CreateCurrentReadingProperty();
        laser->CreateDigitalModulationProperty();
        laser->CreateAnalogModulationFlagProperty();
        laser->CreateAnalogImpedanceProperty();
        laser->CreateModulationPowerSetpointProperty();
        laser->CreateOperatingHoursProperty();
        laser->CreateSerialNumberProperty();
        
    } else if ( modelString.find( "-05-" ) != std::string::npos ) {

        laser = new Laser( "Compact 05", wavelength, driver );

        laser->currentUnit_ = Amperes;
        laser->powerUnit_ = Milliwatts;

        laser->CreateNameProperty();
        laser->CreateModelProperty();
        laser->CreateFirmwareVersionProperty();
        laser->CreateWavelengthProperty();
        laser->CreateLaserOnOffProperty();
        laser->CreateShutterProperty();
        laser->CreateRunModeProperty<ST_05_Series>();
        laser->CreatePowerSetpointProperty();
        laser->CreatePowerReadingProperty();
        laser->CreateCurrentSetpointProperty();
        laser->CreateCurrentReadingProperty();
        laser->CreateOperatingHoursProperty();
        laser->CreateSerialNumberProperty();
        
    } else {

        laser = new Laser( "Unknown", wavelength, driver );
    }
    
    Logger::Instance()->LogMessage( "Created laser '" + laser->GetName() + "'", true );

    laser->SetShutterOpen( false );

    Property::ResetIdGenerator();

    return laser;
}

Laser::~Laser()
{
    const bool pausedPropertyIsPublic = ( shutter_ != NULL && properties_.find( shutter_->GetName() ) != properties_.end() );
    
    if ( !pausedPropertyIsPublic ) {
        delete shutter_;
    }

    for ( PropertyIterator it = GetPropertyIteratorBegin(); it != GetPropertyIteratorEnd(); it++ ) {
        delete it->second;
    }

    properties_.clear();
}

const std::string& Laser::GetId() const
{
    return id_;
}

const std::string& Laser::GetName() const
{
    return name_;
}

const std::string& Laser::GetWavelength() const
{
    return wavelength_;
}

void Laser::SetOn( const bool on )
{
    // Reset shutter on laser on/off:
    SetShutterOpen( false );

    laserOnOffProperty->SetValue( ( on ? EnumerationItem_On : EnumerationItem_Off ) );
}

void Laser::SetShutterOpen( const bool open )
{
    shutter_->SetValue( open ? LaserShutterProperty::Value_Open : LaserShutterProperty::Value_Closed );
}

bool Laser::IsOn() const
{
    return ( laserOnOffProperty->GetValue() ==  EnumerationItem_On );
}

bool Laser::IsShutterOpen() const
{
    return ( shutter_->GetValue() == LaserShutterProperty::Value_Open );
}

Property* Laser::GetProperty( const std::string& name ) const
{
    return properties_.at( name );
}

Property* Laser::GetProperty( const std::string& name )
{
    return properties_[ name ];
}

Laser::PropertyIterator Laser::GetPropertyIteratorBegin()
{
    return properties_.begin();
}

Laser::PropertyIterator Laser::GetPropertyIteratorEnd()
{
    return properties_.end();
}

void Laser::DecomposeModelString( std::string modelString, std::vector<std::string>& modelTokens )
{
    std::string token;

    for ( std::string::iterator character = modelString.begin(); character != modelString.end(); character++ ) {

        if ( *character == '-' || *character == '\r' ) {

            if ( token.length() > 0 ) {
                modelTokens.push_back( token );
                token.clear();
            }

        } else {

            token.push_back( *character );
        }
    }
    
    if ( token.length() > 0 ) {
        modelTokens.push_back( token );
    }
}

Laser::Laser( const std::string& name, const std::string& wavelength, LaserDriver* driver ) :
    id_( std::to_string( (long double) NextId__++ ) ),
    name_( name ),
    wavelength_( wavelength ),
    laserDriver_( driver ),
    currentUnit_( "?" ),
    powerUnit_( "?" )
{
}

void Laser::CreateNameProperty()
{
    RegisterPublicProperty( new StaticStringProperty( "Name", this->GetName() ) );
}

void Laser::CreateModelProperty()
{
    RegisterPublicProperty( new DeviceProperty( Property::String, "Model", laserDriver_, "glm?") );
}

void Laser::CreateWavelengthProperty()
{
    RegisterPublicProperty( new StaticStringProperty( "Wavelength", this->GetWavelength()) );
}

void Laser::CreateSerialNumberProperty()
{
    RegisterPublicProperty( new DeviceProperty( Property::String, "Serial Number", laserDriver_, "gsn?") );
}

void Laser::CreateFirmwareVersionProperty()
{
    RegisterPublicProperty( new DeviceProperty( Property::String, "Firmware Version", laserDriver_, "gfv?") );
}

void Laser::CreateOperatingHoursProperty()
{
    RegisterPublicProperty( new DeviceProperty( Property::String, "Operating Hours", laserDriver_, "hrs?") );
}

void Laser::CreateCurrentSetpointProperty()
{
    std::string maxCurrentSetpointResponse;
    if ( laserDriver_->SendCommand( "gmlc?", &maxCurrentSetpointResponse ) != return_code::ok ) {

        Logger::Instance()->LogError( "Laser::CreateCurrentSetpointProperty(): Failed to retrieve max current sepoint" );
        return;
    }

    const double maxCurrentSetpoint = atof( maxCurrentSetpointResponse.c_str() );

    MutableDeviceProperty* property;
   
    if ( IsShutterCommandSupported() ) {
        property = new NumericProperty<double>( "Current Setpoint [" + currentUnit_ + "]", laserDriver_, "glc?", "slc", 0.0f, maxCurrentSetpoint );
    } else {
        property = new legacy::no_shutter_command::LaserCurrentProperty( "Current Setpoint [" + currentUnit_ + "]", laserDriver_, "glc?", "slc", 0.0f, maxCurrentSetpoint, this );
    }

    RegisterPublicProperty( property );
}

void Laser::CreateCurrentReadingProperty()
{
    DeviceProperty* property = new DeviceProperty( Property::Float, "Measured Current [" + currentUnit_ + "]", laserDriver_, "i?" );
    property->SetCaching( false );
    RegisterPublicProperty( property );
}

void Laser::CreatePowerSetpointProperty()
{
    std::string maxPowerSetpointResponse;
    if ( laserDriver_->SendCommand( "gmlp?", &maxPowerSetpointResponse ) != return_code::ok ) {

        Logger::Instance()->LogError( "Laser::CreatePowerSetpointProperty(): Failed to retrieve max power sepoint" );
        return;
    }

    const double maxPowerSetpoint = atof( maxPowerSetpointResponse.c_str() );
    
    MutableDeviceProperty* property = new NumericProperty<double>( "Power Setpoint [" + powerUnit_ + "]", laserDriver_, "glp?", "slp", 0.0f, maxPowerSetpoint );
    RegisterPublicProperty( property );
}

void Laser::CreatePowerReadingProperty()
{
    DeviceProperty* property = new DeviceProperty( Property::String, "Power Reading [" + powerUnit_ + "]", laserDriver_, "pa?" );
    property->SetCaching( false );
    RegisterPublicProperty( property );
}

void Laser::CreateLaserOnOffProperty()
{
    EnumerationProperty* property = new EnumerationProperty( "Laser Status", laserDriver_, "l?" );

    property->RegisterEnumerationItem( "0", "l0", EnumerationItem_Off );
    property->RegisterEnumerationItem( "1", "l1", EnumerationItem_On );
    
    RegisterPublicProperty( property );
    laserOnOffProperty = property;
}

void Laser::CreateShutterProperty()
{
    if ( IsShutterCommandSupported() ) {
        shutter_ = new LaserShutterProperty( "Emission Status", laserDriver_, this );
    } else {
        shutter_ = new legacy::no_shutter_command::LaserShutterProperty( "Emission Status", laserDriver_, this );
    }
    
    RegisterPublicProperty( shutter_ );
}

template <> void Laser::CreateRunModeProperty<Laser::ST_05_Series>()
{
    EnumerationProperty* property;
    
    if ( IsShutterCommandSupported() ) {
        property = new EnumerationProperty( "Run Mode", laserDriver_, "gam?" );
    } else {
        property = new legacy::no_shutter_command::LaserRunModeProperty( "Run Mode", laserDriver_, "gam?", this );
    }

    property->SetCaching( false );

    property->RegisterEnumerationItem( "0", "sam 0", EnumerationItem_RunMode_ConstantCurrent );
    property->RegisterEnumerationItem( "1", "sam 1", EnumerationItem_RunMode_ConstantPower );

    RegisterPublicProperty( property );
}

template <> void Laser::CreateRunModeProperty<Laser::ST_06_DPL>()
{
    EnumerationProperty* property;
    
    if ( IsShutterCommandSupported() ) {
        property = new EnumerationProperty( "Run Mode", laserDriver_, "gam?" );
    } else {
        property = new legacy::no_shutter_command::LaserRunModeProperty( "Run Mode", laserDriver_, "gam?", this );
    }
    
    property->SetCaching( false );

    property->RegisterEnumerationItem( "0", "sam 0", EnumerationItem_RunMode_ConstantCurrent );
    property->RegisterEnumerationItem( "1", "sam 1", EnumerationItem_RunMode_ConstantPower );
    property->RegisterEnumerationItem( "2", "sam 2", EnumerationItem_RunMode_Modulation );
    
    RegisterPublicProperty( property );
}

template <> void Laser::CreateRunModeProperty<Laser::ST_06_MLD>()
{
    EnumerationProperty* property;

    if ( IsShutterCommandSupported() ) {
        property = new EnumerationProperty( "Run Mode", laserDriver_, "gam?" );
    } else {
        property = new legacy::no_shutter_command::LaserRunModeProperty( "Run Mode", laserDriver_, "gam?", this );
    }
    
    property->SetCaching( false );

    property->RegisterEnumerationItem( "0", "sam 0", EnumerationItem_RunMode_ConstantCurrent );
    property->RegisterEnumerationItem( "1", "sam 1", EnumerationItem_RunMode_ConstantPower );
    property->RegisterEnumerationItem( "2", "sam 2", EnumerationItem_RunMode_Modulation );

    RegisterPublicProperty( property );
}

void Laser::CreateDigitalModulationProperty()
{
    EnumerationProperty* property = new EnumerationProperty( "Digital Modulation", laserDriver_, "gdmes?" );
    property->RegisterEnumerationItem( "0", "sdmes 0", EnumerationItem_Disabled );
    property->RegisterEnumerationItem( "1", "sdmes 1", EnumerationItem_Enabled );
    RegisterPublicProperty( property );
}

void Laser::CreateAnalogModulationFlagProperty()
{
    EnumerationProperty* property = new EnumerationProperty( "Analog Modulation", laserDriver_,  "games?" );
    property->RegisterEnumerationItem( "0", "sames 0", EnumerationItem_Disabled );
    property->RegisterEnumerationItem( "1", "sames 1", EnumerationItem_Enabled );
    RegisterPublicProperty( property );
}

void Laser::CreateModulationPowerSetpointProperty()
{
    std::string maxModulationPowerSetpointResponse;
    if ( laserDriver_->SendCommand( "gmlp?", &maxModulationPowerSetpointResponse ) != return_code::ok ) {

        Logger::Instance()->LogError( "Laser::CreatePowerSetpointProperty(): Failed to retrieve max power sepoint" );
        return;
    }
    
    const double maxModulationPowerSetpoint = atof( maxModulationPowerSetpointResponse.c_str() );
    
    RegisterPublicProperty( new NumericProperty<double>( "Modulation Power Setpoint", laserDriver_, "glmp?", "slmp", 0, maxModulationPowerSetpoint ) );
}

void Laser::CreateAnalogImpedanceProperty()
{
    EnumerationProperty* property = new EnumerationProperty( "Analog Impedance", laserDriver_, "galis?" );
    
    property->RegisterEnumerationItem( "0", "salis 0", "1 kOhm" );
    property->RegisterEnumerationItem( "1", "salis 1", "50 Ohm" );

    RegisterPublicProperty( property );
}

bool Laser::IsShutterCommandSupported()
{
    std::string response;
    laserDriver_->SendCommand( "l0r", &response );
    
    return ( response.find( "OK" ) != std::string::npos );
}

void Laser::RegisterPublicProperty( Property* property )
{
    assert( property != NULL );
    properties_[ property->GetName() ] = property;
}
