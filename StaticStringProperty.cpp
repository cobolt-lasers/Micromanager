/**
 * \file        StaticStringProperty.cpp
 *
 * \authors     Lukas Kalinski
 *
 * \copyright   Cobolt AB, 2020. All rights reserved. // TODO: Set proper license
 */

#include "StaticStringProperty.h"

NAMESPACE_COBOLT_BEGIN

StaticStringProperty::StaticStringProperty( const std::string& name, const std::string& value ) :
    Property( Property::String, name ),
    value_( value )
{}

Property::Stereotype StaticStringProperty::GetStereotype() const
{
    return String;
}

int StaticStringProperty::GetValue( std::string& string ) const
{
    string = value_;
    return return_code::ok;
}

std::string StaticStringProperty::ObjectString() const
{
    return Property::ObjectString() + ( "value_ = " + value_ + "; " );
}

NAMESPACE_COBOLT_END
