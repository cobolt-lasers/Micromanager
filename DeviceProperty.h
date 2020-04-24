/**
 * \file        DeviceProperty.h
 *
 * \authors     Lukas Kalinski
 *
 * \copyright   Cobolt AB, 2020. All rights reserved. // TODO: Set proper license
 */

#ifndef __COBOLT__DEVICE_PROPERTY_H
#define __COBOLT__DEVICE_PROPERTY_H

#include "Property.h"

NAMESPACE_COBOLT_BEGIN

class LaserDevice;

class DeviceProperty : public Property
{
public:

    DeviceProperty( Property::Stereotype stereotype, const std::string& name, LaserDevice* laserDevice, const std::string& getCommand );

    /**
     * \brief If caching is on, then the value will remain the same until it is
     *        changed on the Micromanager side. Thus properties that can change
     *        on laser side should NOT be cached.
     */
    void SetCaching( const bool enabled );

    virtual std::string ObjectString() const;

protected:

    virtual int FetchInto( std::string& string ) const;

    void ClearCache() const;

    LaserDevice* laserDevice_;

private:

    std::string getCommand_;

    bool doCache_;
    mutable std::string cachedValue_;
};

NAMESPACE_COBOLT_END

#endif // #ifndef __COBOLT__DEVICE_PROPERTY_H
