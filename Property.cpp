/**
 * \file        Property.cpp
 *
 * \authors     Lukas Kalinski
 *
 * \copyright   Cobolt AB, 2020. All rights reserved. // TODO: Set proper license
 */

#include "Property.h"
#include "Laser.h"

NAMESPACE_COBOLT_BEGIN

LaserSimulatedPausedProperty::LaserSimulatedPausedProperty( const std::string& name, LaserDevice* laserDevice ) :
    EnumerationProperty( name, laserDevice, "N/A", "N/A" ),
    savedLaserState_( NULL )
{
    RegisterValidValue( value::toggle::on );
    RegisterValidValue( value::toggle::off );
}

LaserSimulatedPausedProperty::~LaserSimulatedPausedProperty()
{
    if ( savedLaserState_ != NULL ) {
        delete savedLaserState_;
    }
}

NAMESPACE_COBOLT_END
