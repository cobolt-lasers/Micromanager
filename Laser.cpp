/**
 * \file        Laser.cpp
 *
 * \authors     Lukas Kalinski
 *
 * \copyright   Cobolt AB, 2020. All rights reserved. // TODO: Set proper license
 */

#include <assert.h>
#include "Laser.h"
#include "Property.h"

using namespace std;
using namespace cobolt;

const char* Laser::Milliamperes = "mA";
const char* Laser::Amperes = "A";
const char* Laser::Milliwatts = "mW";
const char* Laser::Watts = "W";

Laser* Laser::Create( LaserDevice* device )
{
    assert( device != NULL );
    
    std::string modelString;
    if ( device->SendCommand( "glm?", &modelString ) != return_code::ok ) {
        return NULL;
    }
    
    std::vector<std::string> modelTokens;
    DecomposeModelString( modelString, modelTokens );
    Laser* laser;
    
    if ( modelString.find( "-06-" ) != std::string::npos ) {

        laser = new Laser( "06-DPL", device );

        laser->currentUnit_ = Milliamperes; 
        laser->powerUnit_ = Milliwatts;
        laser->maxCurrentSetpoint_ = 3.0f;
        laser->maxPowerSetpoint_ = 100.0f; // TODO: Pick proper value.

        laser->CreateNameProperty();
        laser->CreateModelProperty();
        laser->CreateWavelengthProperty();
        laser->CreateSerialNumberProperty();
        laser->CreateFirmwareVersionProperty();
        laser->CreateOperatingHoursProperty();
        laser->CreateCurrentSetpointProperty();
        laser->CreateCurrentReadingProperty();
        laser->CreatePowerSetpointProperty();
        laser->CreatePowerReadingProperty();
        laser->CreateToggleProperty();
        laser->CreatePausedProperty();
        laser->CreateRunModeProperty();
        laser->CreateDigitalModulationProperty();
        laser->CreateAnalogModulationFlagProperty();
        laser->CreateModulationPowerSetpointProperty();
        laser->CreateAnalogImpedanceProperty();

    } else if ( modelString.find( "-05-" ) != std::string::npos ) {

        laser = new Laser( "Compact 05", device );

        laser->currentUnit_ = Amperes;
        laser->powerUnit_ = Milliwatts;
        laser->maxCurrentSetpoint_ = 3.0f;
        laser->maxPowerSetpoint_ = 100.0f; // TODO: Pick proper value.

        laser->CreateNameProperty();
        laser->CreateModelProperty();
        laser->CreateWavelengthProperty();
        laser->CreateSerialNumberProperty();
        laser->CreateFirmwareVersionProperty();
        laser->CreateOperatingHoursProperty();
        laser->CreateCurrentSetpointProperty();
        laser->CreateCurrentReadingProperty();
        laser->CreatePowerSetpointProperty();
        laser->CreatePowerReadingProperty();
        laser->CreateToggleProperty();
        laser->CreatePausedProperty();
        laser->CreateRunModeProperty();
        
    } else {

        laser = new Laser( "Unknown", device );
    }
    
    if ( modelTokens.size() > 0 ) {
        laser->wavelength_ = modelTokens[ 0 ];
    } else {
        laser->wavelength_ = "Unknown";
    }
    
    Logger::Instance()->Log( "Created laser '" + laser->GetName() + "'", true );

    return laser;
}

Laser::~Laser()
{
    const bool pausedPropertyIsPublic = ( pausedProperty_ != NULL && properties_.find( pausedProperty_->GetName() ) != properties_.end() );
    
    if ( !pausedPropertyIsPublic ) {
        delete pausedProperty_;
    }

    for ( PropertyIterator it = GetPropertyIteratorBegin(); it != GetPropertyIteratorEnd(); it++ ) {
        delete it->second;
    }

    properties_.clear();
}

const std::string& Laser::GetName() const
{
    return name_;
}

const std::string& Laser::GetWavelength() const
{
    return wavelength_;
}

LaserDevice* Laser::GetDevice()
{
    return device_;
}

void Laser::SetOn( const bool on )
{
    toggleProperty_->Set( type::toggle::ToString( ( on ? type::toggle::on : type::toggle::off )  ) );
}

void Laser::SetPaused( const bool paused )
{
    pausedProperty_->Set( type::toggle::ToString( ( paused ? type::toggle::on : type::toggle::off ) ) ); 
}

bool Laser::IsOn() const
{
    return ( type::toggle::FromString( toggleProperty_->Get<std::string>() ) == type::toggle::on );
}

bool Laser::IsPaused() const
{
    return ( type::toggle::FromString( pausedProperty_->Get<std::string>() ) == type::toggle::on );
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

/**
 * \brief Extracts the string parts from the glm? command and put them one by one in a vector.
 *        expects a string where the parts are separated with the character '-'
 *
 * \example The model string could have a format similar to 'WWWW-06-XX-PPPP-CCC'.
 */
void Laser::DecomposeModelString( std::string modelString, std::vector<std::string>& modelTokens )
{
    std::string token;

    for ( std::string::iterator its = modelString.begin(); its != modelString.end(); its++ ) {

        if ( *its != '-' ) {

            token.push_back( *its );

        } else if ( *its == '\r' ) {

            modelTokens.push_back( token );
            break;

        } else {

            modelTokens.push_back( token );
            token.clear();
        }
    }
}

Laser::Laser( const std::string& name, LaserDevice* device ) :
    name_( name ),
    wavelength_( "Unknown" ),
    device_( device ),
    maxCurrentSetpoint_( 0.0f ),
    maxPowerSetpoint_( 0.0f ),
    currentUnit_( "?" ),
    powerUnit_( "?" )
{
    CreateNameProperty();
    CreateModelProperty();
    CreateWavelengthProperty();
    CreateSerialNumberProperty();
    CreateFirmwareVersionProperty();
    CreateOperatingHoursProperty();
    CreatePausedProperty();
    CreateToggleProperty();
}

void Laser::CreateNameProperty()
{
    RegisterPublicProperty( new StaticStringProperty( "Name", this->GetName() ) );
}

void Laser::CreateModelProperty()
{
    RegisterPublicProperty( new BasicProperty<std::string>( "Model", device_, "glm?") );
}

void Laser::CreateWavelengthProperty()
{
    RegisterPublicProperty( new StaticStringProperty( "Wavelength", this->GetWavelength()) );
}

void Laser::CreateSerialNumberProperty()
{
    RegisterPublicProperty( new BasicProperty<std::string>( "Serial Number", device_, "gsn?") );
}

void Laser::CreateFirmwareVersionProperty()
{
    RegisterPublicProperty( new BasicProperty<std::string>( "Firmware Version", device_, "gfv?") );
}

void Laser::CreateOperatingHoursProperty()
{
    RegisterPublicProperty( new BasicProperty<std::string>( "Operating Hours", device_, "hrs?") );
}

void Laser::CreateCurrentSetpointProperty()
{
    MutableProperty* property = new BasicMutableProperty<double>( "Current Setpoint [" + currentUnit_ + "]", device_, "glc?", "slc" );
    property->SetupWith( new RangeConstraint( 0.0f, maxCurrentSetpoint_ ) );
    RegisterPublicProperty( property );
}

void Laser::CreateCurrentReadingProperty()
{
    RegisterPublicProperty( new BasicProperty<double>( "Measured Current [" + currentUnit_ + "]", device_, "i?") );
}

void Laser::CreatePowerSetpointProperty()
{
    MutableProperty* property = new BasicMutableProperty<double>( "Power Setpoint [" + powerUnit_ + "]", device_, "glp?", "slp" );
    property->SetupWith( new RangeConstraint( 0.0f, maxPowerSetpoint_ ) );
    RegisterPublicProperty( property );
}

void Laser::CreatePowerReadingProperty()
{
    RegisterPublicProperty( new BasicProperty<double>( "Power Reading [" + powerUnit_ + "]", device_, "pa?") );
}

void Laser::CreateToggleProperty()
{
    toggleProperty_ = new ToggleProperty( "On-Off Switch", device_, "l?", "l1", "l0" );
    toggleProperty_->SetupWith( new EnumConstraint<type::toggle::symbol>( type::toggle::symbol_strings, type::toggle::__count__ ) );
    RegisterPublicProperty( toggleProperty_ );
}

void Laser::CreatePausedProperty()
{
    if ( IsPauseCommandSupported() ) {
        pausedProperty_ = new LaserPausedProperty( "Paused", device_ );
    } else {
        pausedProperty_ = new LaserSimulatedPausedProperty( "Paused", this );
    }
    
    pausedProperty_->SetupWith( new EnumConstraint<type::toggle::symbol>( type::toggle::symbol_strings, type::toggle::__count__ ) );
    RegisterPublicProperty( pausedProperty_ ); // TODO: Consider removing as this property should not be public.
}

void Laser::CreateRunModeProperty()
{
    MutableProperty* property = new BasicMutableProperty<type::run_mode::cc_cp_mod::symbol>( "Run Mode", device_, "gam?", "sam" );
    property->SetupWith( new EnumConstraint<type::run_mode::cc_cp_mod::symbol>( type::run_mode::cc_cp_mod::symbol_strings, type::run_mode::cc_cp_mod::__count__ ) );
    RegisterPublicProperty( property );
}

void Laser::CreateDigitalModulationProperty()
{
    MutableProperty* property = new BasicMutableProperty<type::flag::symbol>( "Digital Modulation", device_, "gdmes?", "sdmes" );
    property->SetupWith( new EnumConstraint<type::flag::symbol>( type::flag::symbol_strings, type::flag::__count__ ) );
    RegisterPublicProperty( property );
}

void Laser::CreateAnalogModulationFlagProperty()
{
    MutableProperty* property = new BasicMutableProperty<type::flag::symbol>( "Analog Modulation", device_, "games?", "sames" );
    property->SetupWith( new EnumConstraint<type::flag::symbol>( type::flag::symbol_strings, type::flag::__count__ ) );
    RegisterPublicProperty( property );
}

void Laser::CreateModulationPowerSetpointProperty()
{
    RegisterPublicProperty( new BasicMutableProperty<double>( "Modulation Power Setpoint", device_, "glmp?", "slmp") );
}

void Laser::CreateAnalogImpedanceProperty()
{
    MutableProperty* property = new BasicMutableProperty<type::analog_impedance::symbol>( "Analog Impedance", device_, "galis?", "salis" );
    property->SetupWith( new EnumConstraint<type::flag::symbol>( type::analog_impedance::symbol_strings, type::analog_impedance::__count__ ) );
    RegisterPublicProperty( property );
}

bool Laser::IsPauseCommandSupported()
{
    std::string response;
    device_->SendCommand( "l0r", &response );
    
    return ( response.find( "Syntax error" ) == std::string::npos );
}

void Laser::RegisterPublicProperty( Property* property )
{
    assert( property != NULL );
    properties_[ property->GetName() ] = property;
}
