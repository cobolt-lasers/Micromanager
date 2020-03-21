///////////////////////////////////////////////////////////////////////////////
// FILE:          CoboltOfficial.h
// PROJECT:       Micro-Manager
// SUBSYSTEM:     DeviceAdapters
//-----------------------------------------------------------------------------
// DESCRIPTION:   Official laser controller for the COBOLT lasers, via serial port
//                communication.
// COPYRIGHT:     COBOLT, a Hübner Group Company
// LICENSE:       ???? (LGPL)
// AUTHORS:       ????
//
//
#ifndef _COBOLTOFFICIAL_H_
#define _COBOLTOFFICIAL_H_

#include "DeviceBase.h"
#include <string>
#include <vector>
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

class CoboltOfficial: public CShutterBase<CoboltOfficial>
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
   void GetName(char* name) const;

   //////////////////////////////////////////////////////////////////////////////
   // Shutter API
   //
   int SetOpen(bool open = true);
   int GetOpen(bool& open);
   /**
    * Opens the shutter for the given duration, then closes it again.
    * Currently not implemented in any shutter adapters
    */
   int Fire(double deltaT);

   //////////////////////////////////////////////////////////////////////////////
   // Action interface
   //
   int OnPort(MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnLaserModel(MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnSerialNumber(MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnVersion(MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnWaveLength(MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnMaxLaserPower(MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnMaxLaserCurrent(MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnLaserPowerSetting(MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnLaserCurrentSetting(MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnHours(MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnLaserOnOff(MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnOperatingMode(MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnLaserOutput(MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnModulationLaserPower(MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnDigitalModulationState(MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnAnalogModulationState(MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnAnalogImpedanceState(MM::PropertyBase* pProp, MM::ActionType eAct);


private:
   //////////////////////////////////////////////////////////////////////////////
   // Internal, communication & help methods
   //
   int SendSerialCmd(std::string command, std::string& answer);
   int CheckIfPauseCmdIsSupported();
   void ExtractGlmReplyParts (std::string answer, std::vector<std::string> &svec);
   int HandleGLMCmd();
   void HandleShutter(bool openShutter);

   std::string GetSerialNumber();
   std::string GetFirmwareVersion();
   double GetLaserMaxCurrent();
   double GetLaserPowerSetting();
   int SetLaserPowerSetting (double power_mW);
   std::string GetOperatingHours();
   std::string GetLaserStatus();
   int SetLaserStatus (std::string status);
   double GetLaserPowerOutput();
   int SetLaserPauseCommand(bool pauseLaserActive);
   std::string GetOperatingMode();
   int SetLaserOperatingMode(std::string mode);

   double GetLaserDriveCurrent();
   int SetLaserDriveCurrent (double mA);
   std::string GetDigitalModulationState();
   int SetDigitalModulationState (std::string state);
   std::string GetAnalogModulationState();
   int SetAnalogModulationState (std::string state);
   double GetModulationPowerSetting();
   int SetModulationPowerSetting (double power_mW);
   std::string GetAnalogImpedanceState();
   int SetAnalogImpedanceState (std::string state);

   //////////////////////////////////////////////////////////////////////////////
   // Class members
   //
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
