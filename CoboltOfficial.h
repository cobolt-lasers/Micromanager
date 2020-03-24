///////////////////////////////////////////////////////////////////////////////
// FILE:          CoboltOfficial.h
// PROJECT:       Micro-Manager
// SUBSYSTEM:     DeviceAdapters
//-----------------------------------------------------------------------------
// DESCRIPTION:   Official laser controller for the COBOLT lasers, via serial port
//                communication.
// COPYRIGHT:     COBOLT, a Hübner Group Company
// LICENSE:       ???? (LGPL) // TODO: Set proper license
// AUTHORS:       Johan Crone, Lukas Kalinski
//
//

#ifndef __COBOLT_OFFICIAL_H
#define __COBOLT_OFFICIAL_H

#include "DeviceBase.h"
#include <string>
#include <vector>
#include "Laser.h"
#include "Logger.h"
#include "LaserDevice.h"

using namespace std;

//////////////////////////////////////////////////////////////////////////////
// Error codes
//
#define ERR_PORT_CHANGE_FORBIDDEN                101001
#define ERR_SERIAL_PORT_NOT_SELECTED                101002
#define ERR_UNKNOWN_COBOLT_LASER_MODEL           101003
#define OPERATING_SHUTTER_WITH_LASER_OFF         101004
#define ERR_LASER_OPERATING_MODE_NOT_SUPPORTED   101005
#define ERR_CANNOT_SET_MODE_OFF                  101006

class CoboltOfficial : public CShutterBase<CoboltOfficial>, public cobolt::LaserDevice, public cobolt::Logger::Gateway, public cobolt::GuiEnvironment
{
public:

    CoboltOfficial();
    virtual ~CoboltOfficial();

    int ExposeToGui( const Property* property );

    //////////////////////////////////////////////////////////////////////////////
    // MMDevice API
    // ------------
    int Initialize();
    int Shutdown();
    bool Busy();
    void GetName( char* name ) const;

    //////////////////////////////////////////////////////////////////////////////
    // Shutter API
    //
    int SetOpen( bool open = true );
    int GetOpen( bool& open );

    /**
     * Opens the shutter for the given duration, then closes it again.
     * Currently not implemented in any shutter adapters
     */
    int Fire( double duration );

    //////////////////////////////////////////////////////////////////////////////
    // Property Action Handlers
    //
    int OnLaserPropertyAction( MM::PropertyBase*, MM::ActionType );

    int OnPropertyAction_Port                       ( MM::PropertyBase*, MM::ActionType );
    int OnPropertyAction_Model                      ( MM::PropertyBase*, MM::ActionType );
    int OnPropertyAction_SerialNumber               ( MM::PropertyBase*, MM::ActionType );
    int OnPropertyAction_FirmwareVersion            ( MM::PropertyBase*, MM::ActionType );
    int OnPropertyAction_Wavelength                 ( MM::PropertyBase*, MM::ActionType );
    int OnPropertyAction_MaxPowerSetpoint           ( MM::PropertyBase*, MM::ActionType );
    int OnPropertyAction_MaxCurrentSetpoint         ( MM::PropertyBase*, MM::ActionType );
    int OnPropertyAction_PowerSetpoint              ( MM::PropertyBase*, MM::ActionType );
    int OnPropertyAction_CurrentSetpoint            ( MM::PropertyBase*, MM::ActionType );
    int OnPropertyAction_OperatingHours             ( MM::PropertyBase*, MM::ActionType );
    int OnPropertyAction_LaserToggle                ( MM::PropertyBase*, MM::ActionType );
    int OnPropertyAction_RunMode                    ( MM::PropertyBase*, MM::ActionType );
    int OnPropertyAction_PowerReading               ( MM::PropertyBase*, MM::ActionType );
    int OnPropertyAction_ModulationPowerSetpoint    ( MM::PropertyBase*, MM::ActionType );
    int OnPropertyAction_DigitalModulationFlag      ( MM::PropertyBase*, MM::ActionType );
    int OnPropertyAction_AnalogModulationFlag       ( MM::PropertyBase*, MM::ActionType );
    int OnPropertyAction_AnalogImpedance            ( MM::PropertyBase*, MM::ActionType );

private:

    int SendSerialCmd( const std::string& command, std::string& answer );
    int CheckIfPauseCmdIsSupported();
    void ExtractGlmReplyParts( std::string answer, std::vector<std::string> &svec );
    int HandleGLMCmd();
    
    cobolt::Laser* laser_;

    bool bInitialized_;
    std::string port_;
    bool bBusy_;
};

#endif // #ifndef __COBOLT_OFFICIAL_H