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

#ifndef _COBOLTOFFICIAL_H_
#define _COBOLTOFFICIAL_H_

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
#define ERR_UNDEFINED_SERIAL_PORT                101002
#define ERR_UNKNOWN_COBOLT_LASER_MODEL           101003
#define OPERATING_SHUTTER_WITH_LASER_OFF         101004
#define ERR_LASER_OPERATING_MODE_NOT_SUPPORTED   101005
#define ERR_CANNOT_SET_MODE_OFF                  101006

class CoboltOfficial : public CShutterBase<CoboltOfficial>, public cobolt::LaserCommandDispatcher
{
public:

    CoboltOfficial();
    virtual ~CoboltOfficial();

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
    // Property Change Handlers
    //
    int OnPort(                          MM::PropertyBase*, MM::ActionType );
    int OnLaserModel(                    MM::PropertyBase*, MM::ActionType );
    int OnSerialNumber(                  MM::PropertyBase*, MM::ActionType );
    int OnFirmwareVersion(               MM::PropertyBase*, MM::ActionType );
    int OnWaveLength(                    MM::PropertyBase*, MM::ActionType );
    int OnMaxLaserPower(                 MM::PropertyBase*, MM::ActionType );
    int OnMaxLaserCurrent(               MM::PropertyBase*, MM::ActionType );
    int OnLaserPowerSetpoint(            MM::PropertyBase*, MM::ActionType );
    int OnLaserCurrentSetpoint(          MM::PropertyBase*, MM::ActionType );
    int OnHours(                         MM::PropertyBase*, MM::ActionType );
    int OnLaserOnOff(                    MM::PropertyBase*, MM::ActionType );
    int OnRunMode(                       MM::PropertyBase*, MM::ActionType );
    int OnLaserPowerReading(             MM::PropertyBase*, MM::ActionType );
    int OnLaserModulationPowerSetpoint(  MM::PropertyBase*, MM::ActionType );
    int OnDigitalModulationState(        MM::PropertyBase*, MM::ActionType );
    int OnAnalogModulationState(         MM::PropertyBase*, MM::ActionType );
    int OnAnalogImpedanceState(          MM::PropertyBase*, MM::ActionType );

private:

    std::string ToFormattedString( const cobolt::Current& ) const;
    std::string ToFormattedString( const cobolt::Power& ) const;
    std::string WavelengthToFormattedString( const int& ) const;
    std::string HoursToFormattedString( const int& ) const;

    //////////////////////////////////////////////////////////////////////////////
    // Internal, communication & help methods
    //
    int SendSerialCmd( const std::string& command, std::string& answer );
    int CheckIfPauseCmdIsSupported();
    void ExtractGlmReplyParts( std::string answer, std::vector<std::string> &svec );
    int HandleGLMCmd();
    void HandleShutter( bool openShutter );

    std::string GetSerialNumber();
    std::string GetFirmwareVersion();
    double GetLaserMaxCurrent();
    double GetLaserPowerSetting();
    int SetLaserPowerSetting( const Power& );
    std::string GetOperatingHours();
    std::string GetLaserStatus();
    int SetLaserStatus( std::string status );
    double GetLaserPowerOutput();
    int SetLaserPauseCommand( bool pauseLaserActive );
    std::string GetOperatingMode();
    int SetLaserOperatingMode( std::string mode );

    double GetLaserDriveCurrent();
    int SetLaserDriveCurrent( double mA );
    std::string GetDigitalModulationState();
    int SetDigitalModulationState( std::string state );
    std::string GetAnalogModulationState();
    int SetAnalogModulationState( std::string state );
    double GetModulationPowerSetting();
    int SetModulationPowerSetting( double power_mW );
    std::string GetAnalogImpedanceState();
    int SetAnalogImpedanceState( std::string state );

    //////////////////////////////////////////////////////////////////////////////
    // Class members
    //
    cobolt::Laser* laser_;

    bool bInitialized_;
    bool bBusy_;
    bool bLaserIsPaused_;
    bool bLaserPausCmdIsSupported_;
    std::string port_;                    /* Serial port number */
    std::string laserModel_;              /* Laser is  a Skyra, 06, 08, ... */
    std::string serialNumber_;            /* Laser's serial number */
    std::string firmwareVersion_;         /* Laser's firmware version */
    long laserWavelength_;                /* Laser's wavelength */
    double laserMaxPower_;                /* Laser's maximum allowed power [W] */
    double laserMaxCurrent_;              /* Laser's maximum allowed current [mA] */
    double laserPowerSetting_;            /* Laser Power used when laser is on [mW] */
    double laserCurrentSetting_;          /* Laser Current used when laser is on [mA] */
    std::string operatingHours_;          /* Laser's operating hours */
    std::string laserStatus_;             /* Laser is On/Off */
    double laserPowerOutput_;             /* Current laser power output [mW]*/
    double laserCurrentOutput_;           /* Current laser power output [mA]*/
    std::string laserOperatingMode_;      /* Current laser operating mode */
    std::string whichContinousOpMode_;     /* Keeps track on which continuous operating mode was last set */
    std::string digitalModulationState_;  /* Selected Modulation mode */
    std::string analogModulationState_;   /* Selected Modulation mode */
    double modulationPowerSetting_;       /* Configured power (mW) when in modulation mode */
    std::string analogImpedanceState_;    /* Selected laser impedance */

};

#endif /* _COBOLTOFFICIAL_H_ */
