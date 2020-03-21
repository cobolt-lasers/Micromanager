///////////////////////////////////////////////////////////////////////////////
// FILE:          CoboltOfficial.h
// PROJECT:       Micro-Manager
// SUBSYSTEM:     DeviceAdapters
//-----------------------------------------------------------------------------
// DESCRIPTION:   Official laser controller for the COBOLT lasers, via serial port
//                communication.
// COPYRIGHT:     COBOLT, a Hübner Group Company
// LICENSE:       ???? (LGPL) // TODO: Set proper license
// AUTHORS:       ????
//
//

#include "CoboltOfficial.h"
#include "Power.h"
#include "Current.h"

using namespace std;
using namespace cobolt;

//////////////////////////////////////////////////////////////////////////////
// Internal types.
//
typedef struct
{
    bool backupIsActive;
    std::string mode;
    double outputPowerSetting;
    double driveCurrentSetting;
    bool digitalModActive;
    bool analogModActive;
    double modPowerSetting;
    std::string analogImpValue;
} BackedUpDataStruct;

//////////////////////////////////////////////////////////////////////////////
// Device Properties strings
//
const char * g_DeviceName = "CoboltOfficial";
const char * g_DeviceDescription = "COBOLT Official Laser Controller";
const char * g_DeviceVendorName = "COBOLT: a HÜBNER Group Company";

//////////////////////////////////////////////////////////////////////////////
// Laser properties strings
//
//const char * const g_PropertyLaserModel = "01-Laser Model";
//const char * const g_PropertySerialNumber = "15-Laser Serial Number";
//const char * const g_PropertyFirmwareVersion = "16-Laser Firmware Version";
//const char * const g_PropertyWaveLength = "02-Laser Wavelength";
//const char * const g_PropertyMaxPower = "03-Laser Maximum Power (mW)";
//const char * const g_PropertyMaxCurrent = "04-Laser Maximum Current (mA)";
//const char * const g_PropertyLaserPowerSetting = "07-Laser Power Output Setting (mW)";
//const char * const g_PropertyLaserCurrentSetting = "08-Laser Current Output Setting (mA)";
//const char * const g_PropertyOperatingHours = "14-Laser Operating Hours";
//const char * const g_PropertyLaserStatus = "05-Laser Status (On/Off)";
//const char * const g_PropertyOperatingMode = "06-Laser Operation Mode";
//const char * const g_PropertyCurrentLaserOutput = "09-Read Laser Output Power";
//const char * const g_PropertyModulationPower = "13-Modulation Mode Laser Power Setting (mW)";
//const char * const g_PropertyDigitalModulationState = "10-Modulation Mode: Digital";
//const char * const g_PropertyAnalogModulationState = "11-Modulation Mode: Analog";
//const char * const g_PropertyAnalogImpedanceState = "12-Analog Impedance Setting";

//////////////////////////////////////////////////////////////////////////////
// Commonly used strings
//
const char * g_SendTerm = "\r";
const char * g_RecvTerm = "\r\n";

const char* const g_Property_Unknown_Value = "Unknown";

const char * const g_Default_Str_Empty = "";
//const char * const g_Default_Str_Unknown = "Unknown";
const char * const g_Default_Str_Double_0 = "0.0";

//const char * const g_PropertyOn = "On";
//const char * const g_PropertyOff = "Off";
//const char * const g_PropertyEnabled = "Enabled";
//const char * const g_PropertyDisabled = "Disabled";
//const char * const g_PropertyLowImp = "50 Ohm";
//const char * const g_PropertyHighImp = "1 kOhm";

//////////////////////////////////////////////////////////////////////////////
// Cobolt Laser constants
//
const std::string MODEL_06 = "06-laser";
const std::string MODEL_08 = "08-laser";
const std::string MODEL_SKYRA = "Skyra-laser";

//const std::string LASER_OFF_MODE = "Off";
//const std::string CONSTANT_POWER_MODE = "Constant Power Mode";
//const std::string CONSTANT_CURRENT_MODE = "Constant Current Mode";
//const std::string MODULATION_MODE = "Modulation Mode";

static BackedUpDataStruct setupWhenClosingShutter;

///////////////////////////////////////////////////////////////////////////////
// Exported MMDevice API
///////////////////////////////////////////////////////////////////////////////
MODULE_API void InitializeModuleData()
{
    RegisterDevice( g_DeviceName, MM::ShutterDevice, g_DeviceDescription );
}

MODULE_API MM::Device* CreateDevice( const char* deviceName )
{
    if ( deviceName == 0 ) {
        return 0;
    } else if ( strcmp( deviceName, g_DeviceName ) == 0 ) {
        return new CoboltOfficial;
    } else {
        return 0;
    }
}

MODULE_API void DeleteDevice( MM::Device* pDevice )
{
    delete pDevice;
}

CoboltOfficial::CoboltOfficial() :
    laser_( NULL ),
    bInitialized_( false ),
    bBusy_( false )
{
    // TODO Auto-generated constructor stub
    assert( strlen( g_DeviceName ) < (unsigned int) MM::MaxStrLength );

    InitializeDefaultErrorMessages();

    // Map error codes to strings:
    SetErrorText( ERR_PORT_CHANGE_FORBIDDEN,                "You can't change the port after device has been initialized."          );
    SetErrorText( ERR_UNDEFINED_SERIAL_PORT,                "Serial port must not be undefined when initializing!"                  );
    SetErrorText( ERR_UNKNOWN_COBOLT_LASER_MODEL,           "Cobolt Laser Model is not yet supported!"                              );
    SetErrorText( OPERATING_SHUTTER_WITH_LASER_OFF,         "Cannot operate shutter while the Laser is turned off!"                 );
    SetErrorText( ERR_LASER_OPERATING_MODE_NOT_SUPPORTED,   "Laser Operating Mode not yet implemented or not supported!"            );
    SetErrorText( ERR_CANNOT_SET_MODE_OFF,                  "Cannot set Operating mode Off! Use Laser Status to turn On and Off!"   );

    /* Nothing is backed up yet */
    setupWhenClosingShutter.backupIsActive = false;

    // Create properties:
    CreateProperty( MM::g_Keyword_Name,         g_DeviceName,           MM::String, true );
    CreateProperty( "Vendor",                   g_DeviceVendorName,     MM::String, true );
    CreateProperty( MM::g_Keyword_Description,  g_DeviceDescription,    MM::String, true );
    CreateProperty( MM::g_Keyword_Port,         g_Default_Str_Unknown,  MM::String, false, new CPropertyAction( this, &CoboltOfficial::OnPropertyAction_Port ), true );
    
    UpdateStatus();
}

CoboltOfficial::~CoboltOfficial()
{
    Shutdown();
}

//////////////////////////////////////////////////////////////////////////////
// Shutter API
//
int CoboltOfficial::SetOpen( bool open )
{
    int reply = DEVICE_ERR;

    if ( laserStatus_.compare( g_PropertyOff ) == 0 ) {
        if ( open == true ) {
            /* When lasers is turned off, the pause laser output is always inactive.
             * No command is needed to be sent to laser, just log
             */
            LogMessage( "CoboltOfficial::SetOpen: Open shutter requested, while laser is OFF. Ignore request.", true );
        } else {
            /* Close shutter, i.e. turn on laser pause, is not possible when laser is turned off. Log */
            LogMessage( "CoboltOfficial::SetOpen: Close shutter requested, while laser is OFF. Nothing done.", true );
        }
        reply = OPERATING_SHUTTER_WITH_LASER_OFF;
    } else {
        HandleShutter( open );
        reply = DEVICE_OK;
    }

    return reply;
}

/**
 * Tells whether the shutter is open or not (i.e. whether laser is shining or not).
 */
int CoboltOfficial::GetOpen( bool& open )
{
    /*
     * TODO: Laser Pause Command is only supported by new products.
     *       Need to implement functionality for older products.
     */

     /* If Laser is off or if laser on and paused, the shutter is considered closed. */
    open = ( ( laserStatus_.compare( g_PropertyOff ) == 0 ) ? false : ( !bLaserIsPaused_ ) );

    return DEVICE_OK;
}

/**
 * Opens the shutter for the given duration, then closes it again.
 * Currently not implemented in any shutter adapters
 */
int CoboltOfficial::Fire( double deltaT )
{
    int reply = SetOpen( true );

    if ( reply == DEVICE_OK ) {
        CDeviceUtils::SleepMs( (long) ( deltaT + 0.5 ) );

        reply = SetOpen( false );
    }

    return reply;
}

int CoboltOfficial::Initialize()
{
    if ( bInitialized_ ) {
        return DEVICE_OK;
    }
    /* Make sure a serial port is set! */
    else if ( strcmp( port_.c_str(), g_Default_Str_Unknown ) != 0 ) {
        CPropertyAction* pAct;
        int nRet;
        std::vector<std::string> allowedValues;

        /* Verify communication and in the same time set all lasers off (safe mode) */
        {
            std::string answer;
            nRet = SendSerialCmd( "l0", answer ); /* Send all lasers off */
            if ( nRet != DEVICE_OK ) {
                /* Communication failed or not supported command (possibly not a Cobolt laser) */
                if ( nRet == DEVICE_UNSUPPORTED_COMMAND ) {
                    LogMessage( "CoboltOfficial::Initialize: Laser status off cmd not supported, i.e. not a Cobolt laser!" );
                } else {
                    LogMessage( "CoboltOfficial::Initialize: Failed to communicate with Laser." );
                }

                return nRet;
            }
            /* Possible Laser Pause is cancelled when lasers turned off */
            bLaserIsPaused_ = false;
        }

        /* Fetch Laser model, maxpower, wavelength .. */
        nRet = HandleGLMCmd();
        if ( DEVICE_OK != nRet ) {
            LogMessage( "CoboltOfficial::Initialize: Failed to extract laser info! ErrorCode: " + std::to_string( (_Longlong) nRet ), true );
            return nRet;
        }

        /* Check the Pause Laser Command is supported */
        nRet = CheckIfPauseCmdIsSupported();
        if ( DEVICE_OK != nRet ) {
            LogMessage( "CoboltOfficial::Initialize: Failed to check if Laser Pause Command is supported! ErrorCode: " + std::to_string( (_Longlong) nRet ), true );
            return nRet;
        }

        /* LASERMODEL */
        pAct = new CPropertyAction( this, &CoboltOfficial::OnPropertyAction_Model );
        nRet = CreateProperty( g_PropertyLaserModel, laserModel_.c_str(), MM::String, true, pAct );
        if ( DEVICE_OK != nRet ) {
            return nRet;
        }

        /* SERIAL NUMBER */
        serialNumber_ = GetSerialNumber();
        pAct = new CPropertyAction( this, &CoboltOfficial::OnPropertyAction_SerialNumber );
        nRet = CreateProperty( g_PropertySerialNumber, serialNumber_.c_str(), MM::String, true, pAct );
        if ( DEVICE_OK != nRet ) {
            return nRet;
        }

        /* FIRMWARE VERSION */
        firmwareVersion_ = GetFirmwareVersion();
        pAct = new CPropertyAction( this, &CoboltOfficial::OnPropertyAction_FirmwareVersion );
        nRet = CreateProperty( g_PropertyFirmwareVersion, firmwareVersion_.c_str(), MM::String, true, pAct );
        if ( DEVICE_OK != nRet ) {
            return nRet;
        }

        /* OPERATING HOURS */
        operatingHours_ = GetOperatingHours();
        pAct = new CPropertyAction( this, &CoboltOfficial::OnPropertyAction_OperatingHours );
        nRet = CreateProperty( g_PropertyOperatingHours, operatingHours_.c_str(), MM::String, true, pAct );
        if ( DEVICE_OK != nRet ) {
            return nRet;
        }

        /*** TODO: This might be the place to branch the init for single and multi laser models. From this point the properties are laser specific
         *** and not common for the connected laser models.
         ***/

         /* LASER WAVELENGTH */
        pAct = new CPropertyAction( this, &CoboltOfficial::OnPropertyAction_Wavelength );
        nRet = CreateProperty( g_PropertyWaveLength, std::to_string( (long long) laserWavelength_ ).c_str(), MM::Integer, true, pAct );
        if ( DEVICE_OK != nRet ) {
            return nRet;
        }

        /* LASER MAX POWER [mW] */
        pAct = new CPropertyAction( this, &CoboltOfficial::OnPropertyAction_MaxPowerSetpoint );
        nRet = CreateProperty( g_PropertyMaxPower, std::to_string( (long double) ( laserMaxPower_ * 1000.0 ) ).c_str(), MM::Float, true, pAct );
        if ( DEVICE_OK != nRet ) {
            return nRet;
        }

        /* LASER MAX CURRENT [mA] */
        laserMaxCurrent_ = GetLaserMaxCurrent();
        pAct = new CPropertyAction( this, &CoboltOfficial::OnPropertyAction_MaxCurrentSetpoint );
        nRet = CreateProperty( g_PropertyMaxCurrent, std::to_string( (long double) laserMaxCurrent_ ).c_str(), MM::Float, true, pAct );
        if ( DEVICE_OK != nRet ) {
            return nRet;
        }

        /* LASER POWER SETTING [mW] (Initialise to safe state, i.e. 0.0) */
        laserPowerSetting_ = 0.0;
        nRet = SetLaserPowerSetting( laserPowerSetting_ );
        if ( nRet != DEVICE_OK ) {
            /* Failed to update laser with laserpowersetting */
            return nRet;
        }
        pAct = new CPropertyAction( this, &CoboltOfficial::OnPropertyAction_PowerSetpoint );
        nRet = CreateProperty( g_PropertyLaserPowerSetting, g_Default_Str_Double_0, MM::Float, false, pAct );
        if ( DEVICE_OK != nRet ) {
            return nRet;
        }
        /* Create limits 0.0 ... Laser Max Power Setting */
        SetPropertyLimits( g_PropertyLaserPowerSetting, 0.0, ( laserMaxPower_ * 1000.0 ) );

        /* LASER CURRENT SETTING [mA] (Initialise to safe state, i.e. 0.0) */
        laserCurrentSetting_ = 0.0;
        nRet = SetLaserDriveCurrent( laserCurrentSetting_ );
        if ( nRet != DEVICE_OK ) {
            /* Failed to update laser with laserdrivecurrent */
            return nRet;
        }
        pAct = new CPropertyAction( this, &CoboltOfficial::OnPropertyAction_CurrentSetpoint );
        nRet = CreateProperty( g_PropertyLaserCurrentSetting, g_Default_Str_Double_0, MM::Float, false, pAct );
        if ( DEVICE_OK != nRet ) {
            return nRet;
        }
        /* Create limits 0.0 ... Laser Max Current Setting */
        SetPropertyLimits( g_PropertyLaserCurrentSetting, 0.0, laserMaxCurrent_ );


        /* LASER STATUS */
        laserStatus_ = GetLaserStatus();
        if ( laserStatus_.compare( g_PropertyOff ) != 0 ) {
            /* We started the initialize with turning the lasers off */
            LogMessage( "CoboltOfficial::Initialize(): Laser is not turned off as expected! Got: " + laserStatus_, true );
            return DEVICE_ERR;
        }
        pAct = new CPropertyAction( this, &CoboltOfficial::OnPropertyAction_LaserToggle );
        nRet = CreateProperty( g_PropertyLaserStatus, laserStatus_.c_str(), MM::String, false, pAct );
        if ( DEVICE_OK != nRet ) {
            return nRet;
        }
        /* Create allowed values On/Off */
        allowedValues.clear();
        allowedValues.push_back( g_PropertyOn );
        allowedValues.push_back( g_PropertyOff );
        SetAllowedValues( g_PropertyLaserStatus, allowedValues );

        /* LASER OPERATING MODE  (Initialise to Constant Power)*/
        nRet = SetLaserOperatingMode( CONSTANT_POWER_MODE );
        if ( DEVICE_OK != nRet ) {
            return nRet;
        }
        /* Note that as long LaserStatus is Off, the mode returned from laser is OFF */
        laserOperatingMode_ = LASER_OFF_MODE;
        pAct = new CPropertyAction( this, &CoboltOfficial::OnPropertyAction_RunMode );
        nRet = CreateProperty( g_PropertyOperatingMode, laserOperatingMode_.c_str(), MM::String, false, pAct );
        if ( DEVICE_OK != nRet ) {
            return nRet;
        }
        /* Create allowed values */
        allowedValues.clear();
        allowedValues.push_back( CONSTANT_POWER_MODE );
        allowedValues.push_back( CONSTANT_CURRENT_MODE );
        allowedValues.push_back( MODULATION_MODE );
        allowedValues.push_back( LASER_OFF_MODE );
        SetAllowedValues( g_PropertyOperatingMode, allowedValues );

        /* LASER OUTPUT */
        laserPowerOutput_ = GetLaserPowerOutput();
        std::string tmpString = std::to_string( (long double) laserPowerOutput_ ) + " mW";
        pAct = new CPropertyAction( this, &CoboltOfficial::OnPropertyAction_PowerReading );
        nRet = CreateProperty( g_PropertyCurrentLaserOutput, tmpString.c_str(), MM::String, true, pAct );
        if ( DEVICE_OK != nRet ) {
            return nRet;
        }

        /* MODULATION LASER POWER [mW]  (Initialise to safe state, i.e. 0.0) */
        /* TODO: If set modulation laser power command is not supported, skip the property */
        modulationPowerSetting_ = 0.0;
        nRet = SetModulationPowerSetting( modulationPowerSetting_ );
        if ( DEVICE_OK == nRet ) {
            pAct = new CPropertyAction( this, &CoboltOfficial::OnPropertyAction_ModulationPowerSetpoint );
            nRet = CreateProperty( g_PropertyModulationPower, std::to_string( (long double) modulationPowerSetting_ ).c_str(), MM::Float, false, pAct );
            if ( DEVICE_OK != nRet ) {
                return nRet;
            }
            /* TODO: Is it correct to use laser max power as max laser modulation power? */
            /* Create limits 0.0 ... Laser Max Power */
            SetPropertyLimits( g_PropertyModulationPower, 0.0, ( laserMaxPower_ * 1000.0 ) );
        }

        /* DIGITAL MODULATION STATE (Enabled/Disabled) */
        digitalModulationState_ = GetDigitalModulationState();
        pAct = new CPropertyAction( this, &CoboltOfficial::OnPropertyAction_DigitalModulationFlag );
        nRet = CreateProperty( g_PropertyDigitalModulationState, digitalModulationState_.c_str(), MM::String, false, pAct );
        if ( DEVICE_OK != nRet ) {
            return nRet;
        }
        /* Create allowed values */
        allowedValues.clear();
        allowedValues.push_back( g_PropertyEnabled );
        allowedValues.push_back( g_PropertyDisabled );
        SetAllowedValues( g_PropertyDigitalModulationState, allowedValues );

        /* ANALOG MODULATION STATE (Enabled/Disabled) */
        analogModulationState_ = GetAnalogModulationState();
        pAct = new CPropertyAction( this, &CoboltOfficial::OnPropertyAction_AnalogModulationFlag );
        nRet = CreateProperty( g_PropertyAnalogModulationState, analogModulationState_.c_str(), MM::String, false, pAct );
        if ( DEVICE_OK != nRet ) {
            return nRet;
        }
        /* Create allowed values */
        allowedValues.clear();
        allowedValues.push_back( g_PropertyEnabled );
        allowedValues.push_back( g_PropertyDisabled );
        SetAllowedValues( g_PropertyAnalogModulationState, allowedValues );

        /* ANALOG IMPEDANCE STATE (50 Ohm/1 kOhm) */
        analogImpedanceState_ = GetAnalogImpedanceState();
        pAct = new CPropertyAction( this, &CoboltOfficial::OnPropertyAction_AnalogImpedance );
        nRet = CreateProperty( g_PropertyAnalogImpedanceState, analogImpedanceState_.c_str(), MM::String, false, pAct );
        if ( DEVICE_OK != nRet ) {
            return nRet;
        }
        /* Create allowed values */
        allowedValues.clear();
        allowedValues.push_back( g_PropertyLowImp );
        allowedValues.push_back( g_PropertyHighImp );
        SetAllowedValues( g_PropertyAnalogImpedanceState, allowedValues );

        /* Laser is completely initialised */
        bInitialized_ = true;

        return DEVICE_OK;

    } else {
        /* Serial port is still undefined. Cannot initialize */
        LogMessage( "CoboltOfficial::Initialize: Serial Port not defined!", true );
        return ERR_UNDEFINED_SERIAL_PORT;
    }
}

int CoboltOfficial::Shutdown()
{
    if ( bInitialized_ == true ) {
        /* TODO: Should the laser be turned off first? */
        setupWhenClosingShutter.backupIsActive = false;
        bInitialized_ = false;
    }

    return DEVICE_OK;
}

void CoboltOfficial::GetName( char* name ) const
{
    CDeviceUtils::CopyLimitedString( name, g_DeviceName );
}

bool CoboltOfficial::Busy()
{
    return bBusy_;
}

//////////////////////////////////////////////////////////////////////////////
// Action interface
//
int CoboltOfficial::OnPropertyAction_Port( MM::PropertyBase* guiProperty, MM::ActionType action )
{
    if ( action == MM::BeforeGet ) {

        guiProperty->Set( port_.c_str() );

    } else if ( action == MM::AfterSet ) {

        if ( bInitialized_ ) {
            // revert
            guiProperty->Set( port_.c_str() );
            return ERR_PORT_CHANGE_FORBIDDEN;
        }

        guiProperty->Get( port_ );
    }

    return DEVICE_OK;
}

int CoboltOfficial::OnPropertyAction_Model( MM::PropertyBase* guiProperty, MM::ActionType action )
{
    if ( action == MM::BeforeGet ) {

        if ( !laser_->model->FetchInto( *guiProperty ) ) {
            return DEVICE_ERR;
        }
    }

    return DEVICE_OK;
}

int CoboltOfficial::OnPropertyAction_SerialNumber( MM::PropertyBase* guiProperty, MM::ActionType action )
{
    if ( action == MM::BeforeGet ) {

        if ( !laser_->serialNumber->FetchInto( *guiProperty ) ) {
            return DEVICE_ERR;
        }
    }

    return DEVICE_OK;
}

int CoboltOfficial::OnPropertyAction_FirmwareVersion( MM::PropertyBase* guiProperty, MM::ActionType action )
{
    if ( action == MM::BeforeGet ) {

        if ( !laser_->firmwareVersion->FetchInto( *guiProperty ) ) {
            return DEVICE_ERR;
        }
    }

    return DEVICE_OK;
}

int CoboltOfficial::OnPropertyAction_Wavelength( MM::PropertyBase* guiProperty, MM::ActionType action )
{
    if ( action == MM::BeforeGet ) {

        if ( !laser_->wavelength->FetchInto( *guiProperty ) ) {
            return DEVICE_ERR;
        }
    }

    return DEVICE_OK;
}

int CoboltOfficial::OnPropertyAction_MaxPowerSetpoint( MM::PropertyBase* guiProperty, MM::ActionType action )
{
    if ( action == MM::BeforeGet ) {
        
        if ( !laser_->maxPowerSetpoint->FetchInto( *guiProperty ) ) {
            return DEVICE_ERR;
        }
    }

    return DEVICE_OK;
}

int CoboltOfficial::OnPropertyAction_MaxCurrentSetpoint( MM::PropertyBase* guiProperty, MM::ActionType action )
{
    if ( action == MM::BeforeGet ) {
     
        if ( !laser_->maxCurrentSetpoint->FetchInto( *guiProperty ) ) {
            return DEVICE_ERR;
        }
    }

    return DEVICE_OK;
}

int CoboltOfficial::OnPropertyAction_PowerSetpoint( MM::PropertyBase* guiProperty, MM::ActionType action )
{
    if ( action == MM::BeforeGet ) {

        if ( !laser_->powerSetpoint->FetchInto( *guiProperty ) ) {
            return DEVICE_ERR;
        }

    } else if ( action == MM::AfterSet ) {

        if ( !laser_->powerSetpoint->SetFrom( *guiProperty ) ) {
            return DEVICE_ERR;
        }
    }

    return DEVICE_OK;
}

int CoboltOfficial::OnPropertyAction_CurrentSetpoint( MM::PropertyBase* guiProperty, MM::ActionType action )
{
    if ( action == MM::BeforeGet ) {
        
        if ( !laser_->currentSetpoint->FetchInto( *guiProperty ) ) {
            return DEVICE_ERR;
        }

    } else if ( action == MM::AfterSet ) {
    
        if ( !laser_->currentSetpoint->SetFrom( *guiProperty ) ) {
            return DEVICE_ERR;
        }
    }

    return DEVICE_OK;
}

int CoboltOfficial::OnPropertyAction_OperatingHours( MM::PropertyBase* guiProperty, MM::ActionType action )
{
    if ( action == MM::BeforeGet ) {

        if ( !laser_->operatingHours->FetchInto( *guiProperty ) ) {
            return DEVICE_ERR;
        }
    }

    return DEVICE_OK;
}

int CoboltOfficial::OnPropertyAction_LaserToggle( MM::PropertyBase* guiProperty, MM::ActionType action ) // TODO NOW: moving property handling into cobolt::Property
{
    if ( action == MM::BeforeGet ) {
        
        if ( !laser_->toggle->FetchInto( *guiProperty ) ) {
            return DEVICE_ERR;
        }

    } else if ( action == MM::AfterSet ) {

        if ( !laser_->toggle->SetFrom( *guiProperty ) ) {
            return DEVICE_ERR;
        }

        // Begin in paused state (only unpausing when shutter open):
        if ( laser_->toggle->Fetch() == laser::toggle::on ) {
            laser_->paused->Set( true );
        }
    }

    return DEVICE_OK;
}

int CoboltOfficial::OnPropertyAction_RunMode( MM::PropertyBase* guiProperty, MM::ActionType action )
{
    if ( action == MM::BeforeGet ) {

        if ( !laser_->runMode->FetchInto( *guiProperty ) ) {
            return DEVICE_ERR;
        }

    } else if ( action == MM::AfterSet ) {

        if ( !laser_->runMode->SetFrom( *guiProperty ) ) {
            return DEVICE_ERR;
        }
    }

    return DEVICE_OK;
}

int CoboltOfficial::OnPropertyAction_PowerReading( MM::PropertyBase* guiProperty, MM::ActionType action )
{
    if ( action == MM::BeforeGet ) {

        if ( !laser_->powerReading->FetchInto( *guiProperty ) ) {
            return DEVICE_ERR;
        }
    }

    return DEVICE_OK;
}

int CoboltOfficial::OnPropertyAction_ModulationPowerSetpoint( MM::PropertyBase* guiProperty, MM::ActionType action )
{
    if ( action == MM::BeforeGet ) {

        if ( !laser_->modulationPowerSetpoint->FetchInto( *guiProperty ) ) {
            return DEVICE_ERR;
        }

    } else if ( action == MM::AfterSet ) {

        if ( !laser_->modulationPowerSetpoint->SetFrom( *guiProperty ) ) {
            return DEVICE_ERR;
        }
    }
    
    return DEVICE_OK;
}

int CoboltOfficial::OnPropertyAction_DigitalModulationFlag( MM::PropertyBase* guiProperty, MM::ActionType action )
{
    if ( action == MM::BeforeGet ) {

        if ( !laser_->digitalModulationFlag->FetchInto( *guiProperty ) ) {
            return DEVICE_ERR;
        }

    } else if ( action == MM::AfterSet ) {

        if ( !laser_->digitalModulationFlag->SetFrom( *guiProperty ) ) {
            return DEVICE_ERR;
        }
    }

    return DEVICE_OK;
}

int CoboltOfficial::OnPropertyAction_AnalogModulationFlag( MM::PropertyBase* guiProperty, MM::ActionType action )
{
    if ( action == MM::BeforeGet ) {

        if ( !laser_->analogModulationFlag->FetchInto( *guiProperty ) ) {
            return DEVICE_ERR;
        }

    } else if ( action == MM::AfterSet ) {

        if ( !laser_->analogModulationFlag->SetFrom( *guiProperty ) ) {
            return DEVICE_ERR;
        }
    }

    return DEVICE_OK;
}

int CoboltOfficial::OnPropertyAction_AnalogImpedance( MM::PropertyBase* guiProperty, MM::ActionType action )
{
    if ( action == MM::BeforeGet ) {

        if ( !laser_->analogImpedance->FetchInto( *guiProperty ) ) {
            return DEVICE_ERR;
        }

    } else if ( action == MM::AfterSet ) {

        if ( !laser_->analogImpedance->SetFrom( *guiProperty ) ) {
            return DEVICE_ERR;
        }
    }

    return DEVICE_OK;
}

//////////////////////////////////////////////////////////////////////////////
// Private methods: Communication, laser command handling & help methods
//

/******************************************************************************
 * Description:
 * Adds some Cobolt laser serial communication handling on top of the Micro-manager
 * serial communication class' handling.
 *
 * Sends the command, fetches the laser reply and detects unsupported laser commands.
 *
 * Input:
 *    std::string  command - The command to send to the laser.
 *    std::string& answer  - contains the laser's reply.
 *
 * Returns:
 *    int - DEVICE_OK if successful,
 *          DEVICE_UNSUPPORTED_COMMAND if illegal or unsupported laser command
 *          The error code received from the Micro-Manager serial communication class.
 */
int CoboltOfficial::SendSerialCmd( const std::string& command, std::string& answer )
{
    int reply = SendSerialCommand( port_.c_str(), command.c_str(), g_SendTerm );

    if ( reply != DEVICE_OK ) {
        
        LogMessage( "CoboltOfficial::SendSerialCmd: SendSerialCommand Failed: " + std::to_string( (_Longlong) reply ), true );
    
    } else {

        reply = GetSerialAnswer( port_.c_str(), g_RecvTerm, answer );
        
        if ( reply != DEVICE_OK ) {
            
            LogMessage( "CoboltOfficial::SendSerialCmd: GetSerialAnswer Failed: " + std::to_string( (_Longlong) reply ), true );
        
        } else if ( answer.find( "error" ) != std::string::npos ) { // TODO: make find case insensitive
        
            LogMessage( "CoboltOfficial::SendSerialCmd: Sent: " + command + " Reply received: " + answer, true );
            reply = DEVICE_UNSUPPORTED_COMMAND;
        }
    }

    return reply;
}

int CoboltOfficial::CheckIfPauseCmdIsSupported() // TODO: reuse later?
{
    std::string answer;
    int reply = SendSerialCmd( "l0r", answer );

    if ( reply == DEVICE_UNSUPPORTED_COMMAND ) {

        /* The Laser Pause Command is not supported */
        bLaserPausCmdIsSupported_ = false;
        reply = DEVICE_OK;

    } else if ( reply == DEVICE_OK ) {

        /* The Laser Pause Command is supported */
        bLaserPausCmdIsSupported_ = true;

    } else {

        /* TODO: Error handling. Some other problem with the sent pause command
         * Assume not supported.
         */
        bLaserPausCmdIsSupported_ = false;
    }

    return reply;
}

/**
 * Extracts the string parts from the glm? command and put them one by one in a vector.
 * expects a string where the parts are separated with the character '-'
 *
 * For example WWWW-06-XX-PPPP-CCC
 */
void CoboltOfficial::ExtractGlmReplyParts( std::string answer, std::vector<std::string> &svec ) // TODO: same as below
{
    std::string tmpstring;

    tmpstring.clear();

    for ( std::string::iterator its = answer.begin(); its < answer.end(); its++ ) {
        if ( *its != '-' ) {
            tmpstring.push_back( *its );
        } else if ( *its == '\r' ) {
            svec.push_back( tmpstring );
            tmpstring.clear();
            /* The entire string is handled, make sure no more iterations. */
            its = answer.end();
        } else {
            svec.push_back( tmpstring );
            tmpstring.clear();
        }
    }
}

int CoboltOfficial::HandleGLMCmd() // TODO: move parts of the implementation to the right place, get rid of rest
{
    //return laser_->

    /* First element is always laser model in vector */
    std::string answer;
    int reply;
    std::vector<std::string> svec;

    reply = SendSerialCmd( "glm?", answer );

    if ( reply == DEVICE_OK ) {
        /* Detect Laser model */
        if ( answer.find( "-06-" ) != std::string::npos ) {
            /* 06-Series found
             * WWWW-06-XX-PPPP-CCC
             */
            svec.push_back( MODEL_06 );
            LogMessage( "CoboltOfficial::HandleGLMCmd: Model = " + MODEL_06, true );

            ExtractGlmReplyParts( answer, svec );

            laserModel_ = svec[ 0 ];
            laserWavelength_ = std::stol( svec[ 1 ] );
            laserMaxPower_ = ( std::stod( svec[ 4 ] ) / 1000.0 ) + 0.005; // [W]
        } else if ( answer.find( "-08-" ) != std::string::npos ) {
            /* 08-Series found
             * WWWW-08-XX-Y-PPPP-CCC
             */
            svec.push_back( MODEL_08 );

            ExtractGlmReplyParts( answer, svec );

            laserModel_ = svec[ 0 ];
            laserWavelength_ = std::stol( svec[ 1 ] );
            laserMaxPower_ = ( std::stod( svec[ 5 ] ) / 1000.0 ) + 0.005;

        } else if ( ( answer.find( "ML-" ) != std::string::npos ) || ( answer.find( "MF-" ) != std::string::npos ) ) {
            /* Skyra-Series found
             * ML-AAA-BBB-CCC-DDD-XXX-YYY-ZZZ-QQQ-WWW
             */
            svec.push_back( MODEL_SKYRA );

            ExtractGlmReplyParts( answer, svec );

            /* Until implemented */
            return ERR_UNKNOWN_COBOLT_LASER_MODEL;

            /* TODO: Extract all laser properties from the vector */
        } else {
            /* TODO: Error handling for unknown laser model */
            return ERR_UNKNOWN_COBOLT_LASER_MODEL;
        }
    } else {
        /* TODO: Error handling for failed send serial command. */
        LogMessage( "CoboltOfficial::HandleGLMCmd: Failed to send glm?. Errorcode: " + std::to_string( (_Longlong) reply ), true );
        return reply;
    }

    return reply;
}

/******************************************************************************
 * Description:
 *  If Close Shutter:
 *           Pause Cmd supported: Send Laser Pause Command "enable pause"
 *           Else store all important settings and modes before going to Constant Current
 *           and set output current to 0.0 mA - // TODO: implement on request
 *  If Open Shutter:
 *           Pause Cmd supported: Send Laser Pause Command "disable pause"
 *           Else use the stored settings and modes to return from "closed shutter" - // TODO: implement on request
 */
void CoboltOfficial::HandleShutter( bool openShutter )
{
    const bool closeShutter = !openShutter;
    laser_->paused->Set( closeShutter );
}
