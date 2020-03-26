/**
 * \file        CoboltOfficial.h
 *
 * \brief       Official device adapter for COBOLT lasers.
 *
 * \authors     Johan Crone, Lukas Kalinski
 *
 * \copyright   Cobolt AB, 2020. All rights reserved. // TODO: Set proper license
 */

#ifndef __COBOLT_OFFICIAL_H
#define __COBOLT_OFFICIAL_H

#include "DeviceBase.h"
#include <string>
#include "Laser.h"
#include "Logger.h"
#include "LaserDevice.h"

class CoboltOfficial : 
    public CShutterBase<CoboltOfficial>, 
    public cobolt::LaserDevice, 
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
    /// LaserDevice API

    virtual int SendCommand( const std::string& command, std::string* response = NULL );

    /// ###
    /// LoggerGateway API

    virtual void SendLogMessage( const char* message, bool debug ) const;

    /// ###
    /// GuiEnvironment API

    virtual int RegisterAllowedGuiPropertyValue( const std::string& propertyName, std::string& value );
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
    std::string port_;
};

#endif // #ifndef __COBOLT_OFFICIAL_H