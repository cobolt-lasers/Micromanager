/**
 * \file        Laser.cpp
 *
 * \authors     Lukas Kalinski
 *
 * \copyright   Cobolt AB, 2020. All rights reserved. // TODO: Set proper license
 */

#include <assert.h>
#include "Laser.h"

using namespace std;
using namespace cobolt;
using namespace laser;

Laser* Laser::Create( const std::string& modelString )
{
    std::vector<std::string> modelTokens;
    DecomposeModelString( modelString, modelTokens );
    Laser* laser;
    
    if ( modelString.find( "-06-" ) != std::string::npos ) {

        laser = new Laser_06DPL();

    } else {

        laser = new Laser_Unknown();
    }

    laser->wavelength_ = modelTokens[ 0 ];

    return laser;
}

Laser::~Laser()
{
    for ( PropertyIterator it = GetPropertyIteratorBegin(); it != GetPropertyIteratorEnd(); it++ ) {
        delete it->second;
    }

    properties_.clear();
}

const std::string& Laser::GetWavelength() const
{
    return wavelength_;
}

void Laser::SetupWithLaserDevice( LaserDevice* device )
{
    assert( device != NULL );
    assert( device_ == NULL ); // Adapt must not be ran more than once.

    device_ = device;
    
    Incarnate();
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
    return properties_.begin();
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

    for ( std::string::iterator its = modelString.begin(); its < modelString.end(); its++ ) {

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

Laser::Laser( const std::string& modelName ) :
    modelName_( modelName ),
    wavelength_( "Unknown" )
{}

bool Laser::SupportsProperty( const property::symbol symbol ) const
{
    switch ( symbol ) {

        case property::model:
        case property::wavelength:
        case property::serial_number:
        case property::firmware_version:
        case property::operating_hours:
        case property::toggle:
        case property::paused:

            return true;

        default:

            return SupportsModelSpecificProperty( symbol );
    }
}

void Laser::Incarnate()
{
    using namespace property;
    
    // ###
    // Create supported properties:
    
    RegisterProperty( model,            new BasicProperty<std::string>( symbol_strings[ model ], device_, "glm?" ) );
    RegisterProperty( wavelength,       new StaticStringProperty( symbol_strings[ wavelength ], this->GetWavelength() ) );
    RegisterProperty( serial_number,    new BasicProperty<std::string>( symbol_strings[ serial_number ], device_, "gsn?" ) );
    RegisterProperty( firmware_version, new BasicProperty<std::string>( symbol_strings[ firmware_version ], device_, "gfv?" ) );
    RegisterProperty( operating_hours,  new BasicProperty<std::string>( symbol_strings[ operating_hours ], device_, "hrs?" ) );
    RegisterProperty( toggle,           new ToggleProperty( symbol_strings[ toggle ], device_, "l?", "l1", "l0" ) );
    
    RegisterPropertyIfSupported( current_setpoint,            new BasicMutableProperty<double>( symbol_strings[ current_setpoint ], device_, "glc?", "slc" ) );
    RegisterPropertyIfSupported( max_current_setpoint,        new BasicProperty<double>( symbol_strings[ max_current_setpoint ], device_, "gmlc?" ) );
    RegisterPropertyIfSupported( current_reading,             new BasicProperty<double>( symbol_strings[ current_reading ], device_, "i?" ) );
    RegisterPropertyIfSupported( power_setpoint,              new BasicMutableProperty<double>( symbol_strings[ power_setpoint ], device_, "p?", "slp" ) );
    RegisterPropertyIfSupported( max_power_setpoint,          new BasicProperty<double>( symbol_strings[ max_power_setpoint ], device_, "gmlp?" ) );
    RegisterPropertyIfSupported( power_reading,               new BasicProperty<double>( symbol_strings[ power_reading ], device_, "pa?" ) );
    RegisterPropertyIfSupported( paused,                      new LaserPausedProperty( symbol_strings[ paused ], device_ ) );
    RegisterPropertyIfSupported( run_mode_cc_cp_mod,          new BasicMutableProperty<type::run_mode::cc_cp_mod::symbol>( symbol_strings[ run_mode_cc_cp_mod ], device_, "gam?", "sam" ) );
    RegisterPropertyIfSupported( digital_modulation_flag,     new BasicMutableProperty<type::flag::symbol>( symbol_strings[ digital_modulation_flag ], device_, "gdmes?", "sdmes" ) );
    RegisterPropertyIfSupported( analog_modulation_flag,      new BasicMutableProperty<type::flag::symbol>( symbol_strings[ analog_modulation_flag ], device_, "games?", "sames" ) );
    RegisterPropertyIfSupported( modulation_power_setpoint,   new BasicMutableProperty<double>( symbol_strings[ modulation_power_setpoint ], device_, "glmp?", "slmp" ) );
    RegisterPropertyIfSupported( analog_impedance,            new BasicMutableProperty<type::analog_impedance::symbol>( symbol_strings[ analog_impedance ], device_, "galis?", "salis" ) );

    // ###
    // Attach constraints to created properties:
    
    AttachConstraintIfPropertySupported( current_setpoint,          new RangeConstraint( 0.0f, GetProperty( max_current_setpoint )->Get<double>() ) );
    AttachConstraintIfPropertySupported( power_setpoint,            new RangeConstraint( 0.0f, GetProperty( max_power_setpoint )->Get<double>() ) );
    AttachConstraintIfPropertySupported( run_mode_cc_cp_mod,        new EnumConstraint<type::run_mode::cc_cp_mod::symbol>( type::run_mode::cc_cp_mod::symbol_strings, type::run_mode::cc_cp_mod::__count__ ) );
    AttachConstraintIfPropertySupported( digital_modulation_flag,   new EnumConstraint<type::flag::symbol>( type::flag::symbol_strings, type::flag::__count__ ) );
    AttachConstraintIfPropertySupported( analog_modulation_flag,    new EnumConstraint<type::flag::symbol>( type::flag::symbol_strings, type::flag::__count__ ) );
    AttachConstraintIfPropertySupported( analog_impedance,          new EnumConstraint<type::flag::symbol>( type::analog_impedance::symbol_strings, type::analog_impedance::__count__ ) );
}

void Laser::RegisterProperty( const laser::property::symbol propertySymbol, cobolt::Property* property )
{
    properties_[ property::ToString( propertySymbol ) ] = property;
}

void Laser::RegisterPropertyIfSupported( const property::symbol propertySymbol, cobolt::Property* property )
{
    if ( !SupportsProperty( propertySymbol ) ) {
        return;
    }
    
    RegisterProperty( propertySymbol, property );
}

void Laser::AttachConstraintIfPropertySupported( const property::symbol propertySymbol, cobolt::MutableProperty::Constraint* constraint )
{
    if ( !SupportsProperty( propertySymbol ) ) {
        return;
    }

    MutableProperty* mutableProperty = dynamic_cast<MutableProperty*>( properties_[ property::ToString( propertySymbol ) ] );

    if ( mutableProperty == NULL ) {

        Logger::Instance()->Log( "Failed to attach constraint to property '" + property::ToString( propertySymbol ) + "'.", true );
        return;
    }
    
    mutableProperty->SetupWith( constraint );
}
