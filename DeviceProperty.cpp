///////////////////////////////////////////////////////////////////////////////
// FILE:       DeviceProperty.cpp
// PROJECT:    MicroManager
// SUBSYSTEM:  DeviceAdapters
//-----------------------------------------------------------------------------
// DESCRIPTION:
// Cobolt Lasers Controller Adapter
//
// COPYRIGHT:     Cobolt AB, Stockholm, 2020
//                All rights reserved
//
// LICENSE:       MIT
//                Permission is hereby granted, free of charge, to any person obtaining a
//                copy of this software and associated documentation files( the "Software" ),
//                to deal in the Software without restriction, including without limitation the
//                rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
//                sell copies of the Software, and to permit persons to whom the Software is
//                furnished to do so, subject to the following conditions:
//                
//                The above copyright notice and this permission notice shall be included in all
//                copies or substantial portions of the Software.
//
//                THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
//                INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
//                PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
//                HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
//                OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
//                SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

// AUTHORS:
//                Lukas Kalinski / lukas.kalinski@coboltlasers.com (2020)

#include "DeviceProperty.h"
#include "Laser.h"

NAMESPACE_COBOLT_BEGIN

DeviceProperty::DeviceProperty( Property::Stereotype stereotype, const std::string& name, LaserDriver* laserDriver, const std::string& getCommand ) :
    Property( stereotype, name ),
    laserDriver_( laserDriver ),
    getCommand_( getCommand ),
    doCache_( true )
{}

void DeviceProperty::SetCaching( const bool enabled )
{
    doCache_ = enabled;
}

std::string DeviceProperty::ObjectString() const
{
    return Property::ObjectString() + "getCommand_ = " + getCommand_ + "; ";
}

int DeviceProperty::GetValue( std::string& string ) const
{
    int returnCode = return_code::ok;

    if ( IsCacheEnabled() ) {

        if ( cachedValue_.length() == 0 ) {
            returnCode = laserDriver_->SendCommand( getCommand_, &cachedValue_ );
        }

        if ( returnCode == return_code::ok ) {
            string = cachedValue_;
        } else {
            ClearCache();
        }

    } else {

        returnCode = laserDriver_->SendCommand( getCommand_, &string );
    }

    if ( returnCode != return_code::ok ) {
        SetToUnknownValue( string );
    }

    return returnCode;
}

bool DeviceProperty::IsCacheEnabled() const
{
    return doCache_;
}

void DeviceProperty::ClearCache() const
{
    cachedValue_.clear();
}

const std::string& DeviceProperty::GetCachedValue() const
{
    return cachedValue_;
}

NAMESPACE_COBOLT_END
