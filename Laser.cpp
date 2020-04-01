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
using namespace laser;
using namespace laser::property;

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

        laser->currentUnitPrefix_ = Milli; // TODO: Set correct value.
        laser->powerUnitPrefix_ = Milli; // TODO: Set correct value.
        laser->maxCurrentSetpoint_ = 3.0f;
        laser->maxPowerSetpoint_ = 0.1f; // TODO: Pick proper value.

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

        laser->currentUnitPrefix_ = Milli; // TODO: Set correct value.
        laser->powerUnitPrefix_ = Milli; // TODO: Set correct value.
        laser->maxCurrentSetpoint_ = 3.0f;
        laser->maxPowerSetpoint_ = 0.1f; // TODO: Pick proper value.

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
    MutableProperty* p = dynamic_cast<MutableProperty*>( GetProperty( property::toggle ) );
    p->Set( type::toggle::ToString( ( on ? type::toggle::on : type::toggle::off )  ) );
}

void Laser::SetPaused( const bool paused )
{
    MutableProperty* p = dynamic_cast<MutableProperty*>( GetProperty( property::paused ) );
    p->Set( type::toggle::ToString( ( paused ? type::toggle::on : type::toggle::off ) ) ); 
}

bool Laser::IsOn() const
{
    return ( type::toggle::FromString( GetProperty( property::toggle )->Get<std::string>() ) == type::toggle::on );
}

bool Laser::IsPaused() const
{
    return ( type::toggle::FromString( GetProperty( property::paused )->Get<std::string>() ) == type::toggle::on );
}

Property* Laser::GetProperty( const std::string& name ) const
{
    return properties_.at( name );
}

Property* Laser::GetProperty( const std::string& name )
{
    return properties_[ name ];
}

Property* Laser::GetProperty( const laser::property::symbol propertySymbol ) const
{
    return properties_.at( laser::property::ToString( propertySymbol ) );
}

Property* Laser::GetProperty( const laser::property::symbol propertySymbol )
{
    return properties_[ laser::property::ToString( propertySymbol ) ];
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
    currentUnitPrefix_( Milli ),
    powerUnitPrefix_( Milli )
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
    RegisterProperty( new StaticStringProperty( symbol_strings[ name ], this->GetName() ) );
}

void Laser::CreateModelProperty()
{
    RegisterProperty( new BasicProperty<std::string>( symbol_strings[ model ], device_, "glm?") );
}

void Laser::CreateWavelengthProperty()
{
    RegisterProperty( new StaticStringProperty( symbol_strings[ wavelength ], this->GetWavelength()) );
}

void Laser::CreateSerialNumberProperty()
{
    RegisterProperty( new BasicProperty<std::string>( symbol_strings[ serial_number ], device_, "gsn?") );
}

void Laser::CreateFirmwareVersionProperty()
{
    RegisterProperty( new BasicProperty<std::string>( symbol_strings[ firmware_version ], device_, "gfv?") );
}

void Laser::CreateOperatingHoursProperty()
{
    RegisterProperty( new BasicProperty<std::string>( symbol_strings[ operating_hours ], device_, "hrs?") );
}

void Laser::CreateCurrentSetpointProperty()
{
    MutableProperty* property = new BasicMutableProperty<double>( symbol_strings[ current_setpoint ], device_, "glc?", "slc" );
    property->SetupWith( new RangeConstraint( 0.0f, maxCurrentSetpoint_ ) );
    RegisterProperty( property );
}

void Laser::CreateCurrentReadingProperty()
{
    RegisterProperty( new BasicProperty<double>( symbol_strings[ current_reading ], device_, "i?") );
}

void Laser::CreatePowerSetpointProperty()
{
    MutableProperty* property = new BasicMutableProperty<double>( symbol_strings[ power_setpoint ], device_, "p?", "slp" );
    property->SetupWith( new RangeConstraint( 0.0f, maxPowerSetpoint_ ) );
    RegisterProperty( property );
}

void Laser::CreatePowerReadingProperty()
{
    RegisterProperty( new BasicProperty<double>( symbol_strings[ power_reading ], device_, "pa?") );
}

void Laser::CreateToggleProperty()
{
    RegisterProperty( new ToggleProperty( symbol_strings[ toggle ], device_, "l?", "l1", "l0") );
}

void Laser::CreatePausedProperty()
{
    MutableProperty* property;

    if ( IsPauseCommandSupported() ) {
        property = new LaserPausedProperty( symbol_strings[ paused ], device_ );
    } else {
        property = new LaserSimulatedPausedProperty( symbol_strings[ paused ], this );
    }
    
    property->SetupWith( new EnumConstraint<type::toggle::symbol>( type::toggle::symbol_strings, type::toggle::__count__ ) );
    RegisterProperty( property );
}

void Laser::CreateRunModeProperty()
{
    MutableProperty* property = new BasicMutableProperty<type::run_mode::cc_cp_mod::symbol>( symbol_strings[ run_mode_cc_cp_mod ], device_, "gam?", "sam" );
    property->SetupWith( new EnumConstraint<type::run_mode::cc_cp_mod::symbol>( type::run_mode::cc_cp_mod::symbol_strings, type::run_mode::cc_cp_mod::__count__ ) );
    RegisterProperty( property );
}

void Laser::CreateDigitalModulationProperty()
{
    MutableProperty* property = new BasicMutableProperty<type::flag::symbol>( symbol_strings[ digital_modulation_flag ], device_, "gdmes?", "sdmes" );
    property->SetupWith( new EnumConstraint<type::flag::symbol>( type::flag::symbol_strings, type::flag::__count__ ) );
    RegisterProperty( property );
}

void Laser::CreateAnalogModulationFlagProperty()
{
    MutableProperty* property = new BasicMutableProperty<type::flag::symbol>( symbol_strings[ analog_modulation_flag ], device_, "games?", "sames" );
    property->SetupWith( new EnumConstraint<type::flag::symbol>( type::flag::symbol_strings, type::flag::__count__ ) );
    RegisterProperty( property );
}

void Laser::CreateModulationPowerSetpointProperty()
{
    RegisterProperty( new BasicMutableProperty<double>( symbol_strings[ modulation_power_setpoint ], device_, "glmp?", "slmp") );
}

void Laser::CreateAnalogImpedanceProperty()
{
    MutableProperty* property = new BasicMutableProperty<type::analog_impedance::symbol>( symbol_strings[ analog_impedance ], device_, "galis?", "salis" );
    property->SetupWith( new EnumConstraint<type::flag::symbol>( type::analog_impedance::symbol_strings, type::analog_impedance::__count__ ) );
    RegisterProperty( property );
}

bool Laser::HasProperty( const laser::property::symbol propertySymbol ) const
{
    return ( properties_.find( property::ToString( propertySymbol ) ) == properties_.end() );
}

bool Laser::IsPauseCommandSupported()
{
    std::string response;
    device_->SendCommand( "l0r", &response );
    
    return ( response.find( "Syntax error" ) == std::string::npos );
}

void Laser::RegisterProperty( Property* property )
{
    assert( property != NULL );
    properties_[ property->GetName() ] = property;
}
