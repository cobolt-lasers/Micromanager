/**
 * \file        StaticStringProperty.h
 *
 * \authors     Lukas Kalinski
 *
 * \copyright   Cobolt AB, 2020. All rights reserved. // TODO: Set proper license
 */

#ifndef __COBOLT__STATIC_STRING_PROPERTY_H
#define __COBOLT__STATIC_STRING_PROPERTY_H

#include "Property.h"

NAMESPACE_COBOLT_BEGIN

/**
 * \brief Represents a property whose value is set immediately on creation and will not change after that.
 */
class StaticStringProperty : public Property
{
public:

    StaticStringProperty( const std::string& name, const std::string& value );
    
    virtual Stereotype GetStereotype() const;
    virtual int FetchInto( std::string& string ) const;
    virtual std::string ObjectString() const;

private:

    std::string value_;
};

NAMESPACE_COBOLT_END

#endif // #ifndef __COBOLT__STATIC_STRING_PROPERTY_H
