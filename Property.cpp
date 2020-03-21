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

NAMESPACE_COBOLT_END

NAMESPACE_COBOLT_COMPATIBILITY_BEGIN( no_pause_command )

PausedProperty::PausedProperty( Laser* laser ) :
    MutableProperty<bool>( laser->Device() ),
    laser_( laser ),
    laserState_( NULL )
{

}

bool PausedProperty::Fetch() const
{
    return ( laserState_ != NULL );
}

void PausedProperty::Set( const bool& value )
{
    if ( value ) {
        Pause();
    } else {
        Unpause();
    }
}

void PausedProperty::Pause()
{

}

void PausedProperty::Unpause()
{

}

NAMESPACE_COBOLT_COMPATIBILITY_END
