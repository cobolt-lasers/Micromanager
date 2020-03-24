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

Laser::Laser()
{}

Laser::~Laser()
{
    ClearProperties();
}

void Laser::SetupWithLaserDevice( LaserDevice* device )
{
    assert( device != NULL );
    device_ = device;

    Adapt();
}

LaserDevice* Laser::Device()
{
    return device_;
}

void Laser::ClearProperties()
{
    PropertyIterator it = PropertyIteratorBegin();
    while ( it != PropertyIteratorEnd() ) {
        delete it->second;
    }
    
    properties_.clear();
}

void Laser::Adapt()
{
    // OLD CODE BEGIN (from CoboltOfficial::Initialize()) -->
    //std::string answer;
    //nRet = SendSerialCmd( "l0", answer ); /* Send all lasers off */
    //if ( nRet != DEVICE_OK ) {
    //    /* Communication failed or not supported command (possibly not a Cobolt laser) */
    //    if ( nRet == DEVICE_UNSUPPORTED_COMMAND ) {
    //        LogMessage( "CoboltOfficial::Initialize: Laser status off cmd not supported, i.e. not a Cobolt laser!" );
    //    } else {
    //        LogMessage( "CoboltOfficial::Initialize: Failed to communicate with Laser." );
    //    }

    //    return nRet;
    //}
    ///* Possible Laser Pause is cancelled when lasers turned off */
    //bLaserIsPaused_ = false;
    // <-- OLD CODE END

    if ( device_ == NULL ) {
        return;
    }

    ClearProperties();

    properties_[ property::firmware_version           ] = new DefaultProperty<std::string>                    ( property::firmware_version,            device_, "gfv?"                        );
    properties_[ property::wavelength                 ] = new DefaultProperty<std::string>                    ( property::wavelength,                  device_, ""                            );
    properties_[ property::serial_number              ] = new DefaultProperty<std::string>                    ( property::serial_number,               device_, "gsn?"                        );
    properties_[ property::firmware_version           ] = new DefaultProperty<std::string>                    ( property::firmware_version,            device_, ""                            );
    properties_[ property::operating_hours            ] = new DefaultProperty<std::string>                    ( property::operating_hours,             device_, "hrs?"                        );

    properties_[ property::current_setpoint           ] = new DefaultMutableProperty<double>                  ( property::current_setpoint,            device_, "glc?",      "slc"            );
    properties_[ property::max_current_setpoint       ] = new MaxLaserCurrentProperty<double>                 ( property::max_current_setpoint,        device_, ""                            );
    properties_[ property::current_reading            ] = new DefaultProperty<double>                         ( property::current_reading,             device_, "i?"                          );
    properties_[ property::power_setpoint             ] = new DefaultMutableProperty<double>                  ( property::power_setpoint,              device_, "p?",        "slp"            );
    properties_[ property::max_power_setpoint         ] = new DefaultProperty<double>                         ( property::max_power_setpoint,          device_, ""                            );
    
    properties_[ property::power_reading              ] = new DefaultProperty<double>                         ( property::power_reading,               device_, "pa?"                         );
    properties_[ property::toggle                     ] = new ToggleProperty                                  ( property::toggle,                      device_, "l?",        "l0",      "l1"  );
    properties_[ property::paused                     ] = new LaserPausedProperty                             ( property::paused,                      device_                                );
    properties_[ property::run_mode                   ] = new DefaultMutableProperty<run_mode::type>          ( property::run_mode,                    device_, "gam?", "sam"                 );
    properties_[ property::digital_modulation_flag    ] = new DefaultMutableProperty<flag::type>              ( property::digital_modulation_flag,     device_, "gdmes?",    "sdmes"          );
    
    properties_[ property::analog_modulation_flag     ] = new DefaultMutableProperty<flag::type>              ( property::analog_modulation_flag,      device_, "games?",    "sames"          );
    properties_[ property::modulation_power_setpoint  ] = new DefaultMutableProperty<double>                  ( property::modulation_power_setpoint,   device_, "glmp?",     "slmp"           );
    properties_[ property::analog_impedance           ] = new DefaultMutableProperty<analog_impedance::type>  ( property::analog_impedance,            device_, "galis?",    "salis"          );
    
    ( (MutableProperty*) properties_[ property::current_setpoint ] )->SetupWith( new RangeConstraint( 0.0f, properties_[ property::max_current_setpoint ]->Get<double>() ) );

    string model;
    device_->SendCommand( "glm?", model );

    EnableProperty( laser::property::firmware_version );
}