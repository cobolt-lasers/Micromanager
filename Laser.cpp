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
    wavelength_( "Unknown" ),
    device_( NULL )
{}

void Laser::CreateModelProperty()                     { RegisterProperty( new BasicProperty<std::string>( symbol_strings[ model ], device_, "glm?") );                                                    }
void Laser::CreateWavelengthProperty()                { RegisterProperty( new StaticStringProperty( symbol_strings[ wavelength ], this->GetWavelength()) );                                               }
void Laser::CreateSerialNumberProperty()              { RegisterProperty( new BasicProperty<std::string>( symbol_strings[ serial_number ], device_, "gsn?") );                                            }
void Laser::CreateFirmwareVersionProperty()           { RegisterProperty( new BasicProperty<std::string>( symbol_strings[ firmware_version ], device_, "gfv?") );                                         }
void Laser::CreateOperatingHoursProperty()            { RegisterProperty( new BasicProperty<std::string>( symbol_strings[ operating_hours ], device_, "hrs?") );                                          }
void Laser::CreateCurrentSetpointProperty()           { RegisterProperty( new BasicMutableProperty<double>( symbol_strings[ current_setpoint ], device_, "glc?", "slc") );                                }
void Laser::CreateMaxCurrentSetpointProperty()        { RegisterProperty( new BasicProperty<double>( symbol_strings[ max_current_setpoint ], device_, "gmlc?") );                                         }
void Laser::CreateCurrentReadingProperty()            { RegisterProperty( new BasicProperty<double>( symbol_strings[ current_reading ], device_, "i?") );                                                 }
void Laser::CreatePowerSetpointProperty()             { RegisterProperty( new BasicMutableProperty<double>( symbol_strings[ power_setpoint ], device_, "p?", "slp") );                                    }
void Laser::CreateMaxPowerSetpointProperty()          { RegisterProperty( new BasicProperty<double>( symbol_strings[ max_power_setpoint ], device_, "gmlp?") );                                           }
void Laser::CreatePowerReadingProperty()              { RegisterProperty( new BasicProperty<double>( symbol_strings[ power_reading ], device_, "pa?") );                                                  }
void Laser::CreateToggleProperty()                    { RegisterProperty( new ToggleProperty( symbol_strings[ toggle ], device_, "l?", "l1", "l0") );                                                     }
void Laser::CreatePausedProperty()                    { RegisterProperty( IsPauseCommandSupported() ? new LaserPausedProperty( symbol_strings[ paused ], device_) : new LaserSimulatedPausedProperty( symbol_strings[ paused ], this ) );                                                                  }
void Laser::CreateRunModeProperty()                   { RegisterProperty( new BasicMutableProperty<type::run_mode::cc_cp_mod::symbol>( symbol_strings[ run_mode_cc_cp_mod ], device_, "gam?", "sam") );   }
void Laser::CreateDigitalModulationProperty()         { RegisterProperty( new BasicMutableProperty<type::flag::symbol>( symbol_strings[ digital_modulation_flag ], device_, "gdmes?", "sdmes") );         }
void Laser::CreateAnalogModulationFlagProperty()      { RegisterProperty( new BasicMutableProperty<type::flag::symbol>( symbol_strings[ analog_modulation_flag ], device_, "games?", "sames") );          }
void Laser::CreateModulationPowerSetpointProperty()   { RegisterProperty( new BasicMutableProperty<double>( symbol_strings[ modulation_power_setpoint ], device_, "glmp?", "slmp") );                     }
void Laser::CreateAnalogImpedanceProperty()           { RegisterProperty( new BasicMutableProperty<type::analog_impedance::symbol>( symbol_strings[ analog_impedance ], device_, "galis?", "salis") );    }

bool Laser::IsPauseCommandSupported()
{
    std::string response;
    device_->SendCommand( "l0r", &response );
    
    return ( response.find( "Syntax error" ) == std::string::npos );
}

void Laser::CreateGenericProperties()
{
    CreateModelProperty();
    CreateWavelengthProperty();
    CreateSerialNumberProperty();
    CreateFirmwareVersionProperty();
    CreateOperatingHoursProperty();
    CreatePausedProperty();
    CreateToggleProperty();
}

void Laser::Incarnate()
{
    CreateGenericProperties();
    CreateSpecificProperties();

    // ###
    // Attach constraints to created properties:
    
    // TODO: If max current laser type specific, then place these in the CreateXXXXXProperty() functions as we no longer need to rely on other properties.
    AttachConstraintIfPropertySupported( current_setpoint,          new RangeConstraint( 0.0f, GetProperty( max_current_setpoint )->Get<double>() ) );
    AttachConstraintIfPropertySupported( power_setpoint,            new RangeConstraint( 0.0f, GetProperty( max_power_setpoint )->Get<double>() ) );
    AttachConstraintIfPropertySupported( run_mode_cc_cp_mod,        new EnumConstraint<type::run_mode::cc_cp_mod::symbol>( type::run_mode::cc_cp_mod::symbol_strings, type::run_mode::cc_cp_mod::__count__ ) );
    AttachConstraintIfPropertySupported( digital_modulation_flag,   new EnumConstraint<type::flag::symbol>( type::flag::symbol_strings, type::flag::__count__ ) );
    AttachConstraintIfPropertySupported( analog_modulation_flag,    new EnumConstraint<type::flag::symbol>( type::flag::symbol_strings, type::flag::__count__ ) );
    AttachConstraintIfPropertySupported( analog_impedance,          new EnumConstraint<type::flag::symbol>( type::analog_impedance::symbol_strings, type::analog_impedance::__count__ ) );
}

void Laser::RegisterProperty( Property* property )
{
    assert( property != NULL );
    properties_[ property->GetName() ] = property;
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
