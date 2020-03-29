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

LaserSimulatedPausedProperty::LaserSimulatedPausedProperty( const std::string& name, Laser* laser ) :
    BasicMutableProperty<type::toggle::symbol>( name, laser->GetDevice(), "N/A", "N/A" ),
    toggle_( type::toggle::ToString( type::toggle::off ) )
{}

NAMESPACE_COBOLT_END
