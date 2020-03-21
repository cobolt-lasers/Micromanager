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

Laser::Laser() :

    serialNumber      ( NULL ), // TODO: Order
    firmwareVersion   ( NULL ),
    maxCurrentSetpoint( NULL ),
    powerSetpoint     ( NULL ),
    operatingHours    ( NULL ),
    on                ( NULL ),
    powerReading      ( NULL ),
    paused            ( NULL ),
    runMode           ( NULL ),
    currentSetpoint   ( NULL )
{
    Adapt();
}

void Laser::SetupWith( Logger* logger )
{
    assert( logger != NULL );
    logger_ = logger;
}

void Laser::SetupWith( LaserDevice* device )
{
    assert( device != NULL );
    device_ = device;
}

LaserDevice* Laser::Device()
{
    return device_;
}

void Laser::Adapt()
{
    if ( device_ == NULL ) {
        
        serialNumber         = UnsupportedProperty<string>::Instance(); // TODO: Order
        firmwareVersion      = UnsupportedProperty<string>::Instance();
        maxCurrentSetpoint   = UnsupportedProperty<Current>::Instance();
        powerSetpoint        = UnsupportedMutableProperty<Power>::Instance();
        operatingHours       = UnsupportedProperty<string>::Instance();
        on                   = UnsupportedMutableProperty<bool>::Instance();
        powerReading         = UnsupportedProperty<Power>::Instance();
        paused               = UnsupportedMutableProperty<bool>::Instance();
        runMode              = UnsupportedProperty<string>::Instance();
        currentSetpoint      = UnsupportedMutableProperty<double>::Instance();

        return;
    }

    string model;
    device_->SendCommand( "glm?", model );


}