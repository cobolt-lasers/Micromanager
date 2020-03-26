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

#define ERR_PORT_CHANGE_FORBIDDEN                101001
#define ERR_SERIAL_PORT_NOT_SELECTED             101002
#define OPERATING_SHUTTER_WITH_LASER_OFF         101003

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
    /// Property Action Handlers

    int OnPropertyAction_Port( MM::PropertyBase*, MM::ActionType );
    int OnPropertyAction_Laser( MM::PropertyBase*, MM::ActionType );

private:

    MM::PropertyType ResolvePropertyType( const cobolt::Property::Stereotype ) const;
    int ExposeToGui( const cobolt::Property* property );
    
    cobolt::Laser* laser_;

    bool bInitialized_;
    std::string port_;
    bool bBusy_;
};

#endif // #ifndef __COBOLT_OFFICIAL_H