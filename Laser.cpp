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

    properties_[ laser::property::firmware_version           ] = new DefaultProperty(           laser::property::firmware_version,            device_, "gfv?"                   );
    properties_[ laser::property::wavelength                 ] = new DefaultProperty(           laser::property::wavelength,                  device_, ""                       );
    properties_[ laser::property::serial_number              ] = new DefaultProperty(           laser::property::serial_number,               device_, "gsn?"                   );
    properties_[ laser::property::firmware_version           ] = new DefaultProperty(           laser::property::firmware_version,            device_, ""                       );
    properties_[ laser::property::operating_hours            ] = new DefaultProperty(           laser::property::operating_hours,             device_, "hrs?"                   );

    properties_[ laser::property::current_setpoint           ] = new DefaultMutableProperty(    laser::property::current_setpoint,            device_, "glc?",      "slc"       );
    properties_[ laser::property::max_current_setpoint       ] = new DefaultProperty(           laser::property::max_current_setpoint,        device_, ""                       );
    properties_[ laser::property::current_reading            ] = new DefaultProperty(           laser::property::current_reading,             device_, "i?"                     );
    properties_[ laser::property::power_setpoint             ] = new DefaultMutableProperty(    laser::property::power_setpoint,              device_, "p?",        "slp"       );
    properties_[ laser::property::max_power_setpoint         ] = new DefaultProperty(           laser::property::max_power_setpoint,          device_, ""                       );

    properties_[ laser::property::power_reading              ] = new DefaultProperty(           laser::property::power_reading,               device_, "pa?"                    );
    properties_[ laser::property::toggle                     ] = new LaserOnProperty(           laser::property::toggle,                      device_                           );
    properties_[ laser::property::paused                     ] = new LaserPausedProperty(       laser::property::paused,                      device_                           );
    properties_[ laser::property::run_mode                   ] = new LaserRunModeProperty(      laser::property::run_mode,                    device_                           );
    properties_[ laser::property::digital_modulation_flag    ] = new MutableFlagProperty(       laser::property::digital_modulation_flag,     device_, "gdmes?",    "sdmes"     );
    
    properties_[ laser::property::analog_modulation_flag     ] = new DefaultProperty(           laser::property::analog_modulation_flag,      device_, "games?",    "sames"     );
    properties_[ laser::property::modulation_power_setpoint  ] = new DefaultProperty(           laser::property::modulation_power_setpoint,   device_, ""                       );
    properties_[ laser::property::analog_impedance           ] = new DefaultProperty(           laser::property::analog_impedance,            device_, ""                       );
    

    string model;
    device_->SendCommand( "glm?", model );

    EnableProperty( laser::property::firmware_version );
}