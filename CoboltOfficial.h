///////////////////////////////////////////////////////////////////////////////
// FILE:       CoboltOfficial.h
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
//                Johan Crone (2020)
//                Lukas Kalinski / lukas.kalinski@coboltlasers.com (2020)

#ifndef __COBOLT_OFFICIAL_H
#define __COBOLT_OFFICIAL_H

#include "DeviceBase.h"
#include <string>
#include "Laser.h"
#include "Logger.h"
#include "LaserDriver.h"

class CoboltOfficial : 
    public CShutterBase<CoboltOfficial>, 
    public cobolt::LaserDriver, 
    public cobolt::Logger::Gateway, 
    public cobolt::GuiEnvironment
{
public:

    CoboltOfficial();
    virtual ~CoboltOfficial();

    /// ### 
    /// MMDevice API
    
    int Initialize();
    int Shutdown();
    bool Busy();
    void GetName( char* name ) const;

    /// ###
    /// Shutter API
    
    int SetOpen( bool open = true );
    int GetOpen( bool& open );

    /**
     * Opens the shutter for the given duration, then closes it again.
     * Currently not implemented in any shutter adapters
     */
    int Fire( double duration );

    /// ###
    /// LaserDriver API

    virtual int SendCommand( const std::string& command, std::string* response = NULL );

    /// ###
    /// LoggerGateway API

    virtual void SendLogMessage( const char* message, bool debug ) const;

    /// ###
    /// GuiEnvironment API

    virtual int RegisterAllowedGuiPropertyValue( const std::string& propertyName, const std::string& value );
    virtual int RegisterAllowedGuiPropertyRange( const std::string& propertyName, double min, double max );

    /// ###
    /// Property Action Handlers

    int OnPropertyAction_Port( MM::PropertyBase*, MM::ActionType );
    int OnPropertyAction_Laser( MM::PropertyBase*, MM::ActionType );

private:

    MM::PropertyType ResolvePropertyType( const cobolt::Property::Stereotype ) const;
    int ExposeToGui( const cobolt::Property* property );
    
    cobolt::Laser* laser_;

    bool isInitialized_;
    bool isBusy_;
    std::string port_;
};

#endif // #ifndef __COBOLT_OFFICIAL_H