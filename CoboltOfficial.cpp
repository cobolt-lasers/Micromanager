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
    bBusy_( false ),
    bLaserIsPaused_( false ),
    bLaserPausCmdIsSupported_( false ),
    port_( g_Default_Str_Unknown ),
    laserModel_( g_Default_Str_Unknown ),
    serialNumber_( g_Default_Str_Unknown ),
    firmwareVersion_( g_Default_Str_Unknown ),
    laserWavelength_( 0 ),
    laserMaxPower_( 0.0 ),
    laserMaxCurrent_( 0.0 ),
    laserPowerSetting_( 0.0 ),
    laserCurrentSetting_( 0.0 ),
    operatingHours_( g_Default_Str_Unknown ),
    laserStatus_( g_Default_Str_Unknown ),
    laserPowerOutput_( 0.0 ),
    laserCurrentOutput_( 0.0 ),
    laserOperatingMode_( g_Default_Str_Unknown ),
    whichContinousOpMode_( g_Default_Str_Unknown ),
    digitalModulationState_( g_Default_Str_Unknown ),
    analogModulationState_( g_Default_Str_Unknown ),
    modulationPowerSetting_( 0.0 ),
    analogImpedanceState_( g_Default_Str_Unknown )
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
    CreateProperty( MM::g_Keyword_Port,         g_Default_Str_Unknown,  MM::String, false, new CPropertyAction( this, &CoboltOfficial::OnPort ), true );

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
        pAct = new CPropertyAction( this, &CoboltOfficial::OnLaserModel );
        nRet = CreateProperty( g_PropertyLaserModel, laserModel_.c_str(), MM::String, true, pAct );
        if ( DEVICE_OK != nRet ) {
            return nRet;
        }

        /* SERIAL NUMBER */
        serialNumber_ = GetSerialNumber();
        pAct = new CPropertyAction( this, &CoboltOfficial::OnSerialNumber );
        nRet = CreateProperty( g_PropertySerialNumber, serialNumber_.c_str(), MM::String, true, pAct );
        if ( DEVICE_OK != nRet ) {
            return nRet;
        }

        /* FIRMWARE VERSION */
        firmwareVersion_ = GetFirmwareVersion();
        pAct = new CPropertyAction( this, &CoboltOfficial::OnFirmwareVersion );
        nRet = CreateProperty( g_PropertyFirmwareVersion, firmwareVersion_.c_str(), MM::String, true, pAct );
        if ( DEVICE_OK != nRet ) {
            return nRet;
        }

        /* OPERATING HOURS */
        operatingHours_ = GetOperatingHours();
        pAct = new CPropertyAction( this, &CoboltOfficial::OnHours );
        nRet = CreateProperty( g_PropertyOperatingHours, operatingHours_.c_str(), MM::String, true, pAct );
        if ( DEVICE_OK != nRet ) {
            return nRet;
        }

        /*** TODO: This might be the place to branch the init for single and multi laser models. From this point the properties are laser specific
         *** and not common for the connected laser models.
         ***/

         /* LASER WAVELENGTH */
        pAct = new CPropertyAction( this, &CoboltOfficial::OnWaveLength );
        nRet = CreateProperty( g_PropertyWaveLength, std::to_string( (long long) laserWavelength_ ).c_str(), MM::Integer, true, pAct );
        if ( DEVICE_OK != nRet ) {
            return nRet;
        }

        /* LASER MAX POWER [mW] */
        pAct = new CPropertyAction( this, &CoboltOfficial::OnMaxLaserPower );
        nRet = CreateProperty( g_PropertyMaxPower, std::to_string( (long double) ( laserMaxPower_ * 1000.0 ) ).c_str(), MM::Float, true, pAct );
        if ( DEVICE_OK != nRet ) {
            return nRet;
        }

        /* LASER MAX CURRENT [mA] */
        laserMaxCurrent_ = GetLaserMaxCurrent();
        pAct = new CPropertyAction( this, &CoboltOfficial::OnMaxLaserCurrent );
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
        pAct = new CPropertyAction( this, &CoboltOfficial::OnLaserPowerSetpoint );
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
        pAct = new CPropertyAction( this, &CoboltOfficial::OnLaserCurrentSetpoint );
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
        pAct = new CPropertyAction( this, &CoboltOfficial::OnLaserOnOff );
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
        pAct = new CPropertyAction( this, &CoboltOfficial::OnRunMode );
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
        pAct = new CPropertyAction( this, &CoboltOfficial::OnLaserPowerReading );
        nRet = CreateProperty( g_PropertyCurrentLaserOutput, tmpString.c_str(), MM::String, true, pAct );
        if ( DEVICE_OK != nRet ) {
            return nRet;
        }

        /* MODULATION LASER POWER [mW]  (Initialise to safe state, i.e. 0.0) */
        /* TODO: If set modulation laser power command is not supported, skip the property */
        modulationPowerSetting_ = 0.0;
        nRet = SetModulationPowerSetting( modulationPowerSetting_ );
        if ( DEVICE_OK == nRet ) {
            pAct = new CPropertyAction( this, &CoboltOfficial::OnLaserModulationPowerSetpoint );
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
        pAct = new CPropertyAction( this, &CoboltOfficial::OnDigitalModulationState );
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
        pAct = new CPropertyAction( this, &CoboltOfficial::OnAnalogModulationState );
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
        pAct = new CPropertyAction( this, &CoboltOfficial::OnAnalogImpedanceState );
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
int CoboltOfficial::OnPort( MM::PropertyBase* pProp, MM::ActionType eAct )
{
    if ( eAct == MM::BeforeGet ) {
        pProp->Set( port_.c_str() );
    } else if ( eAct == MM::AfterSet ) {
        if ( bInitialized_ ) {
            // revert
            pProp->Set( port_.c_str() );
            return ERR_PORT_CHANGE_FORBIDDEN;
        }

        pProp->Get( port_ );
    }

    return DEVICE_OK;
}

int CoboltOfficial::OnLaserModel( MM::PropertyBase* pProp, MM::ActionType eAct )
{
    if ( eAct == MM::BeforeGet ) {

        const std::string model = laser_->model->Fetch();

        if ( laser_->model->LastRequestSuccessful() ) {

            pProp->Set( model.c_str() );

        } else {
            
            pProp->Set( g_Property_Unknown_Value );
            return DEVICE_ERR;
        }
    }

    return DEVICE_OK;

    //if ( eAct == MM::BeforeGet ) {
    //    /* Read Only, i.e. always set property according to member value. */
    //    pProp->Set( laserModel_.c_str() );
    //}

    //return DEVICE_OK;
}

int CoboltOfficial::OnSerialNumber( MM::PropertyBase* pProp, MM::ActionType eAct )
{
    if ( eAct == MM::BeforeGet ) {

        if ( !laser_->serialNumber->FetchInto( *pProp ) ) {
            return DEVICE_ERR;
        }
    }

    return DEVICE_OK;

    //if ( eAct == MM::BeforeGet ) {
    //    /* Read Only, i.e. always set property according to member value. */
    //    pProp->Set( serialNumber_.c_str() );
    //}
    //return DEVICE_OK;
}

int CoboltOfficial::OnFirmwareVersion( MM::PropertyBase* pProp, MM::ActionType eAct )
{
    if ( eAct == MM::BeforeGet ) {

        if ( !laser_->firmwareVersion->FetchInto( *pProp ) ) {
            return DEVICE_ERR;
        }
    }

    return DEVICE_OK;

    //if ( eAct == MM::BeforeGet ) {
    //    /* Read Only, i.e. always set property according to member value. */
    //    pProp->Set( firmwareVersion_.c_str() );
    //}

    //return DEVICE_OK;
}

int CoboltOfficial::OnWaveLength( MM::PropertyBase* pProp, MM::ActionType eAct )
{
    if ( eAct == MM::BeforeGet ) {

        if ( !laser_->wavelength->FetchInto( *pProp ) ) {
            return DEVICE_ERR;
        }
    }

    return DEVICE_OK;

    //if ( eAct == MM::BeforeGet ) {
    //    /* Read Only, i.e. always set property according to member value. */
    //    pProp->Set( laserWavelength_ );
    //}

    //return DEVICE_OK;
}

int CoboltOfficial::OnMaxLaserPower( MM::PropertyBase* pProp, MM::ActionType eAct )
{
    if ( eAct == MM::BeforeGet ) {
        
        if ( !laser_->maxPowerSetpoint->FetchInto( *pProp ) ) {
            return DEVICE_ERR;
        }
    }

    return DEVICE_OK;

    //if ( eAct == MM::BeforeGet ) {
    //    /* Read Only, i.e. always set property according to member value. */
    //    pProp->Set( laserMaxPower_ * 1000.0 );
    //}

    //return DEVICE_OK;
}

int CoboltOfficial::OnMaxLaserCurrent( MM::PropertyBase* pProp, MM::ActionType eAct )
{
    if ( eAct == MM::BeforeGet ) {
     
        if ( !laser_->maxCurrentSetpoint->FetchInto( *pProp ) ) {
            return DEVICE_ERR;
        }
    }

    return DEVICE_OK;

    //if ( eAct == MM::BeforeGet ) {
    //    /* Read Only, i.e. always set property according to member value. */
    //    pProp->Set( laserMaxCurrent_ );
    //}

    //return DEVICE_OK;
}

int CoboltOfficial::OnLaserPowerSetpoint( MM::PropertyBase* pProp, MM::ActionType eAct )
{
    if ( eAct == MM::BeforeGet ) {

        if ( !laser_->powerSetpoint->FetchInto( *pProp ) ) {
            return DEVICE_ERR;
        }

    } else if ( eAct == MM::AfterSet ) {

        if ( !laser_->powerSetpoint->SetFrom( *pProp ) ) {
            return DEVICE_ERR;
        }
    }

    return DEVICE_OK;

    //double value;
    //int reply = DEVICE_OK;

    //if ( eAct == MM::BeforeGet ) {
    //    pProp->Set( laserPowerSetting_ );

    //} else if ( eAct == MM::AfterSet ) {
    //    pProp->Get( value );
    //    reply = SetLaserPowerSetting( Power::mW( value ) );
    //    if ( reply != DEVICE_OK ) {
    //        /* Failed to change laser power. Fetch current power and update property */
    //        pProp->Set( laserPowerSetting_ );
    //    }
    //}

    //return reply;
}

int CoboltOfficial::OnLaserCurrentSetpoint( MM::PropertyBase* pProp, MM::ActionType eAct )
{
    if ( eAct == MM::BeforeGet ) {
        
        if ( !laser_->currentSetpoint->FetchInto( *pProp ) ) {
            return DEVICE_ERR;
        }

    } else if ( eAct == MM::AfterSet ) {
    
        if ( !laser_->currentSetpoint->SetFrom( *pProp ) ) {
            return DEVICE_ERR;
        }
    }

    return DEVICE_OK;

    //double value;
    //int reply = DEVICE_OK;

    //if ( eAct == MM::BeforeGet ) {
    //    pProp->Set( laserCurrentSetting_ );

    //} else if ( eAct == MM::AfterSet ) {
    //    pProp->Get( value );
    //    reply = SetLaserDriveCurrent( value );
    //    if ( reply != DEVICE_OK ) {
    //        /* Failed to change laser current. Fetch used current and update property */
    //        pProp->Set( laserCurrentSetting_ );
    //    }
    //}

    //return reply;
}

int CoboltOfficial::OnHours( MM::PropertyBase* pProp, MM::ActionType eAct )
{
    if ( eAct == MM::BeforeGet ) {

        if ( !laser_->operatingHours->FetchInto( *pProp ) ) {
            return DEVICE_ERR;
        }
    }

    return DEVICE_OK;

    //if ( eAct == MM::BeforeGet ) {
    //    /* Read Only, i.e. always set property according to member value. */
    //    operatingHours_ = GetOperatingHours();
    //    pProp->Set( operatingHours_.c_str() );
    //}

    //return DEVICE_OK;
}

int CoboltOfficial::OnLaserOnOff( MM::PropertyBase* pProp, MM::ActionType eAct ) // TODO NOW: moving property handling into cobolt::Property
{
    if ( eAct == MM::BeforeGet ) {
        
        if ( !laser_->on->FetchInto( *pProp ) ) {
            return DEVICE_ERR;
        }

    } else if ( eAct == MM::AfterSet ) {

        if ( !laser_->on->SetFrom( *pProp ) ) {
            return DEVICE_ERR;
        }
    }

    return DEVICE_OK;

    //if ( eAct == MM::BeforeGet ) {
    //    laserStatus_ = GetLaserStatus();
    //    pProp->Set( laserStatus_.c_str() );
    //} else if ( eAct == MM::AfterSet ) {
    //    std::string answer;
    //    int reply = DEVICE_ERR;

    //    pProp->Get( answer );
    //    if ( answer.compare( g_PropertyOn ) == 0 ) {
    //        reply = SetLaserStatus( g_PropertyOn );
    //    } else if ( answer.compare( g_PropertyOff ) == 0 ) {
    //        reply = SetLaserStatus( g_PropertyOff );
    //    } else {
    //        /* TODO: Error handling for not supported status */
    //        pProp->Set( laserStatus_.c_str() ); /* Restore property to current Status */
    //        LogMessage( "CoboltOfficial::OnLaserOnOff: Invalid LaserStatus. Exp: On/Off Got: " + answer, true );
    //        return DEVICE_INVALID_INPUT_PARAM;
    //    }

    //    if ( reply != DEVICE_OK ) {
    //        /* Failed to set the new status. Update Property with current status */
    //        pProp->Set( laserStatus_.c_str() );
    //    }
    //}

    //return DEVICE_OK;
}

int CoboltOfficial::OnRunMode( MM::PropertyBase* pProp, MM::ActionType eAct )
{
    if ( eAct == MM::BeforeGet ) {

        pProp->Set( laser::run_mode::string[ laser_->runMode->Fetch() ] );

    } else if ( eAct == MM::AfterSet ) {

        std::string runModeStr;
        
        pProp->Get( runModeStr );

        laser::run_mode::type runMode = laser::run_mode::FromString( runModeStr );

        if ( runMode == laser::run_mode::__undefined__ ) {
            LogMessage( "CoboltOfficial::OnRunMode(): Invalid value" );
            return DEVICE_ERR;
        }

        laser_->runMode->Set( runMode );

        // Fall back on current value on failure:
        if ( !laser_->runMode->LastRequestSuccessful() ) {
            pProp->Set( laser::run_mode::string[ laser_->runMode->Fetch() ] );
        }
    }

    return DEVICE_OK;

    //if ( eAct == MM::BeforeGet ) {
    //    laserOperatingMode_ = GetOperatingMode();
    //    pProp->Set( laserOperatingMode_.c_str() );
    //} else if ( eAct == MM::AfterSet ) {
    //    std::string answer;
    //    int reply = DEVICE_ERR;

    //    pProp->Get( answer );
    //    if ( answer.compare( CONSTANT_CURRENT_MODE ) == 0 ) {
    //        reply = SetLaserOperatingMode( CONSTANT_CURRENT_MODE );
    //    } else if ( answer.compare( CONSTANT_POWER_MODE ) == 0 ) {
    //        reply = SetLaserOperatingMode( CONSTANT_POWER_MODE );
    //    } else if ( answer.compare( MODULATION_MODE ) == 0 ) {
    //        reply = SetLaserOperatingMode( MODULATION_MODE );
    //    } else if ( answer.compare( LASER_OFF_MODE ) == 0 ) {
    //        reply = SetLaserOperatingMode( LASER_OFF_MODE );
    //    } else {
    //        /* TODO: Error handling for not supported mode */
    //        pProp->Set( laserOperatingMode_.c_str() ); /* Restore property to current mode */
    //        LogMessage( "CoboltOfficial::OnRunMode: Invalid LaserOperating Mode. Got: " + answer, true );
    //        return DEVICE_INVALID_INPUT_PARAM;
    //    }

    //    if ( reply != DEVICE_OK ) {
    //        /* Failed to set the new status. Update Property with current mode */
    //        pProp->Set( laserOperatingMode_.c_str() );
    //    }
    //}

    //return DEVICE_OK;
}

int CoboltOfficial::OnLaserPowerReading( MM::PropertyBase* pProp, MM::ActionType eAct )
{
    if ( eAct == MM::BeforeGet ) {

        const std::string formattedWattageStr = ToFormattedString( laser_->powerReading->Fetch() );
        
        if ( laser_->powerReading->LastRequestSuccessful() ) {
            pProp->Set( formattedWattageStr.c_str() );
        } else {
            pProp->Set( g_Property_Unknown_Value );
            return DEVICE_ERR;
        }
    }

    return DEVICE_OK;

    //std::string tmpString;

    ///* Read Only. Only update property */

    //if ( eAct == MM::BeforeGet ) {
    //    laserPowerOutput_ = GetLaserPowerOutput();
    //    tmpString = std::to_string( (long double) laserPowerOutput_ ) + " mW";
    //    pProp->Set( tmpString.c_str() );

    //}

    //return DEVICE_OK;
}

int CoboltOfficial::OnLaserModulationPowerSetpoint( MM::PropertyBase* pProp, MM::ActionType eAct )
{
    if ( eAct == MM::BeforeGet ) {

        const std::string formattedWattage = ToFormattedString( laser_->modulationPowerSetpoint->Fetch() );

        if ( laser_->modulationPowerSetpoint->LastRequestSuccessful() ) {

            pProp->Set( formattedWattage.c_str() );

        } else {

            pProp->Set( g_Property_Unknown_Value );
            return DEVICE_ERR;
        }

    } else if ( eAct == MM::AfterSet ) {

        double mW;
        pProp->Get( mW );

        laser_->modulationPowerSetpoint->Set( Power::mW( mW ) );

        if ( laser_->modulationPowerSetpoint->LastRequestSuccessful() ) {

            pProp->Set( ToFormattedString( laser_->modulationPowerSetpoint->Fetch() ).c_str() );

        } else {

            pProp->Set( g_Property_Unknown_Value );
            return DEVICE_ERR;
        }
    }
    
    return DEVICE_OK;
}

// TODO NOW: Implement the 3 functions below, then revise everything start making it work

int CoboltOfficial::OnDigitalModulationState( MM::PropertyBase* pProp, MM::ActionType eAct )
{
    if ( eAct == MM::BeforeGet ) {

        const std::string flag = laser::flag::string[ laser_->digitalModulationState->Fetch() ];

        if ( laser_->digitalModulationState->LastRequestSuccessful() ) {

            pProp->Set( flag.c_str() );

        } else {

            pProp->Set( g_Property_Unknown_Value );
            return DEVICE_ERR;
        }

    } else if ( eAct == MM::AfterSet ) {

        std::string valueStr;

        pProp->Get( valueStr );
        
        const laser::flag::type flag = laser::flag::FromString( valueStr );

        if ( flag == laser::flag::__undefined__ ) {

            pProp->Set( g_Property_Unknown_Value );
            LogMessage( "CoboltOfficial::OnDigitalModulationState(): Invalid value" );
            return DEVICE_ERR;
        }

        laser_->digitalModulationState->Set( flag );

        if ( !laser_->digitalModulationState->LastRequestSuccessful() ) {

            pProp->Set( g_Property_Unknown_Value );
            return DEVICE_ERR;
        }
    }

    return DEVICE_OK;

    //if ( eAct == MM::BeforeGet ) {
    //    digitalModulationState_ = GetDigitalModulationState();
    //    pProp->Set( digitalModulationState_.c_str() );
    //} else if ( eAct == MM::AfterSet ) {
    //    std::string answer;
    //    int reply = DEVICE_ERR;

    //    pProp->Get( answer );
    //    if ( answer.compare( g_PropertyEnabled ) == 0 ) {
    //        reply = SetDigitalModulationState( g_PropertyEnabled );
    //    } else if ( answer.compare( g_PropertyDisabled ) == 0 ) {
    //        reply = SetDigitalModulationState( g_PropertyDisabled );
    //    } else {
    //        /* TODO: Error handling for not supported state */
    //        pProp->Set( digitalModulationState_.c_str() ); /* Restore property to current State */
    //        LogMessage( "CoboltOfficial::OnDigitalModulationState: Invalid Digital Modulation State. Exp: Enabled/Disabled Got: " + answer, true );
    //        return DEVICE_INVALID_INPUT_PARAM;
    //    }

    //    if ( reply != DEVICE_OK ) {
    //        /* Failed to set the new state. Update Property with current state */
    //        pProp->Set( digitalModulationState_.c_str() );
    //    }
    //}

    //return DEVICE_OK;
}

int CoboltOfficial::OnAnalogModulationState( MM::PropertyBase* pProp, MM::ActionType eAct )
{
    if ( eAct == MM::BeforeGet ) {
        analogModulationState_ = GetAnalogModulationState();
        pProp->Set( analogModulationState_.c_str() );
    } else if ( eAct == MM::AfterSet ) {
        std::string answer;
        int reply = DEVICE_ERR;

        pProp->Get( answer );
        if ( answer.compare( g_PropertyEnabled ) == 0 ) {
            reply = SetAnalogModulationState( g_PropertyEnabled );
        } else if ( answer.compare( g_PropertyDisabled ) == 0 ) {
            reply = SetAnalogModulationState( g_PropertyDisabled );
        } else {
            /* TODO: Error handling for not supported state */
            pProp->Set( analogModulationState_.c_str() ); /* Restore property to current State */
            LogMessage( "CoboltOfficial::OnAnalogModulationState: Invalid Analog Modulation State. Exp: Enabled/Disabled Got: " + answer, true );
            return DEVICE_INVALID_INPUT_PARAM;
        }

        if ( reply != DEVICE_OK ) {
            /* Failed to set the new state. Update Property with current state */
            pProp->Set( analogModulationState_.c_str() );
        }
    }

    return DEVICE_OK;
}

int CoboltOfficial::OnAnalogImpedanceState( MM::PropertyBase* pProp, MM::ActionType eAct )
{
    if ( eAct == MM::BeforeGet ) {
        analogImpedanceState_ = GetAnalogImpedanceState();
        pProp->Set( analogImpedanceState_.c_str() );
    } else if ( eAct == MM::AfterSet ) {
        std::string answer;
        int reply = DEVICE_ERR;

        pProp->Get( answer );
        if ( answer.compare( g_PropertyLowImp ) == 0 ) {
            reply = SetAnalogImpedanceState( g_PropertyLowImp );
        } else if ( answer.compare( g_PropertyHighImp ) == 0 ) {
            reply = SetAnalogImpedanceState( g_PropertyHighImp );
        } else {
            /* TODO: Error handling for not supported state */
            pProp->Set( analogImpedanceState_.c_str() ); /* Restore property to current State */
            LogMessage( "CoboltOfficial::OnAnalogImpedanceState: Invalid Analog Impedance State. Exp: 50Ohm/1kOhm Got: " + answer, true );
            return DEVICE_INVALID_INPUT_PARAM;
        }

        if ( reply != DEVICE_OK ) {
            /* Failed to set the new state. Update Property with current state */
            pProp->Set( analogImpedanceState_.c_str() );
        }
    }

    return DEVICE_OK;
}

std::string CoboltOfficial::ToFormattedString( const cobolt::Current& c ) const // TODO: Deprecated, use ToString/FromString + provide unit in property name
{
    return ( std::to_string( (long double) c.mA() ) + " mA" );
}

std::string CoboltOfficial::ToFormattedString( const cobolt::Power& p ) const // TODO: Deprecated, use ToString/FromString + provide unit in property name
{
    return ( std::to_string( (long double) p.mW() ) + " mW" );
}

std::string CoboltOfficial::WavelengthToFormattedString( const int& wavelength ) const // TODO: Deprecated, use ToString/FromString + provide unit in property name
{
    return std::to_string( (_Longlong) wavelength ) + " nm";
}

std::string CoboltOfficial::HoursToFormattedString( const int& hours ) const // TODO: Deprecated, use ToString/FromString + provide unit in property name
{
    return std::to_string( (_Longlong) hours ) + " h";
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

int CoboltOfficial::CheckIfPauseCmdIsSupported()
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
void CoboltOfficial::ExtractGlmReplyParts( std::string answer, std::vector<std::string> &svec )
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

int CoboltOfficial::HandleGLMCmd()
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
 *           and set output current to 0.0 mA
 *  If Open Shutter:
 *           Pause Cmd supported: Send Laser Pause Command "disable pause"
 *           Else use the stored settings and modes to return from "closed shutter"
 */
void CoboltOfficial::HandleShutter( bool openShutter )
{
    const bool closeShutter = !openShutter;
    laser_->paused->Set( closeShutter );
    
    //if ( bLaserPausCmdIsSupported_ == true ) {

    //    /* Send Pause if request is close shutter, i.e. openShutter = false */
    //    SetLaserPauseCommand( !openShutter );
    //    bLaserIsPaused_ = !openShutter;

    //} else {

    //    /* If close shutter, goto constant current and set current = 0,
    //     * else restore current user setup.
    //     */
    //    if ( openShutter == true ) {
    //        /*** OPEN SHUTTER ***/

    //        if ( setupWhenClosingShutter.backupIsActive == true ) {
    //            /* Restore settings to before shutter closed, not needed if never "closed the shutter in the first place" */
    //            SetLaserPowerSetting( setupWhenClosingShutter.outputPowerSetting );
    //            SetLaserDriveCurrent( setupWhenClosingShutter.driveCurrentSetting );
    //            std::string state = ( setupWhenClosingShutter.digitalModActive ? g_PropertyEnabled : g_PropertyDisabled );
    //            SetDigitalModulationState( state );
    //            state.clear();
    //            state = ( setupWhenClosingShutter.analogModActive ? g_PropertyEnabled : g_PropertyDisabled );
    //            SetAnalogModulationState( state );
    //            SetAnalogImpedanceState( setupWhenClosingShutter.analogImpValue );
    //            SetModulationPowerSetting( setupWhenClosingShutter.modPowerSetting );
    //            SetAnalogImpedanceState( setupWhenClosingShutter.analogImpValue );
    //            SetLaserOperatingMode( setupWhenClosingShutter.mode );
    //        }

    //        /* Clear the memory variable */
    //        setupWhenClosingShutter.mode.clear();
    //        setupWhenClosingShutter.outputPowerSetting = 0.0;
    //        setupWhenClosingShutter.driveCurrentSetting = 0.0;
    //        setupWhenClosingShutter.digitalModActive = false;
    //        setupWhenClosingShutter.analogModActive = false;
    //        setupWhenClosingShutter.modPowerSetting = 0.0;
    //        setupWhenClosingShutter.analogImpValue.clear();
    //        setupWhenClosingShutter.backupIsActive = false;

    //        bLaserIsPaused_ = false; /* Used to indicate open shutter in GetOpen */

    //    } else {

    //        /*** CLOSE SHUTTER ***/

    //        /* Backup current setup and settings */
    //        setupWhenClosingShutter.mode = laserOperatingMode_;
    //        setupWhenClosingShutter.outputPowerSetting = laserPowerSetting_;
    //        setupWhenClosingShutter.driveCurrentSetting = laserCurrentSetting_;
    //        setupWhenClosingShutter.digitalModActive = ( digitalModulationState_ == g_PropertyEnabled );
    //        setupWhenClosingShutter.analogModActive = ( analogModulationState_ == g_PropertyEnabled );
    //        setupWhenClosingShutter.modPowerSetting = modulationPowerSetting_;
    //        setupWhenClosingShutter.analogImpValue = analogModulationState_;
    //        setupWhenClosingShutter.backupIsActive = true;

    //        /* Set 0.0A and goto constant current */
    //        SetLaserDriveCurrent( 0.0 );
    //        SetLaserOperatingMode( CONSTANT_CURRENT_MODE );

    //        bLaserIsPaused_ = true; /* Used to indicate closed shutter in GetOpen */
    //    }
    //}
}

std::string CoboltOfficial::GetSerialNumber()
{
    return laser_->serialNumber->Get();

    //std::string answer;
    //int reply;

    //reply = SendSerialCmd( "gsn?", answer );
    //if ( reply != DEVICE_OK ) {
    //    /* TODO: Error handling for failed send serial command. */
    //    LogMessage( "CoboltOfficial::GetSerialNumber: gsn? command failed. Errorcode: " + std::to_string( (_Longlong) reply ), true );
    //    answer = g_Default_Str_Unknown;
    //}

    //return answer;
}

std::string CoboltOfficial::GetFirmwareVersion()
{
    return laser_->firmwareVersion->Get();

    //std::string answer;
    //int reply;

    //reply = SendSerialCmd( "ver?", answer );
    //if ( reply != DEVICE_OK ) {
    //    /* TODO: Error handling for failed send serial command. */
    //    LogMessage( "CoboltOfficial::GetFirmwareVersion: Failed to send ver? Errorcode: " + std::to_string( (_Longlong) reply ), true );
    //    answer = g_Default_Str_Unknown;
    //}

    //return answer;
}

double CoboltOfficial::GetLaserMaxCurrent()
{
    return laser_->maxCurrentSetpoint->Get();

    //double lastSet_mW = 0.0; /* Contains the last successfully set Drive Current or 0.0 mw. */

    //if ( bInitialized_ == true ) {
    //    /* Already found out the Maximum Drive Current */
    //    return laserMaxCurrent_;
    //} else {
    //    /* Remember current Drive Current Setting */
    //    double storedLaserCurrentSetting = laserCurrentSetting_;
    //    /* Make sure laser is turned off in order to not do any damage */
    //    SetLaserStatus( g_PropertyOff );

    //    /* Start the drive current check */
    //    bool Done = false;
    //    double mW = 10.0;
    //    double mW_Steps = 10.0;
    //    int nRet = DEVICE_OK;

    //    while ( Done == false ) {
    //        nRet = SetLaserDriveCurrent( mW );

    //        if ( nRet == DEVICE_UNSUPPORTED_COMMAND ) {
    //            /* Not supported drive current. */
    //            if ( mW_Steps == 1.0 ) {
    //                /* Max Drive Current is Found */
    //                laserMaxCurrent_ = lastSet_mW;
    //                SetLaserDriveCurrent( storedLaserCurrentSetting );
    //                Done = true;
    //            } else {
    //                /* Go back to last allowed drive current and increase with half the steps */
    //                mW_Steps = ( ( ( mW_Steps / 2.0 ) > 1.0 ) ? ( mW_Steps / 2.0 ) : 1.0 );
    //                mW = lastSet_mW + mW_Steps;
    //            }
    //        } else if ( nRet == DEVICE_OK ) {
    //            /* Requested Drive Current was allowed */
    //            lastSet_mW = mW;
    //            mW += mW_Steps;
    //        } else {
    //            /* Something went wrong, use last working drive current */
    //            Done = true;
    //        }
    //    }


    //}

    //return lastSet_mW;
}

double CoboltOfficial::GetLaserPowerSetting()
{
    // TODO NOW: Problem: how to determine mW vs W? (and mA vs A?)
    //laser_->Power

    std::string answer;
    double power_mW;
    int reply;

    reply = SendSerialCmd( "p?", answer ); /* Read output power (W) */
    if ( reply != DEVICE_OK ) {
        /* TODO: Error handling for failed send serial command. */
        LogMessage( "CoboltOfficial::GetLaserPowerSetting: Failed Serial Command: " + std::to_string( (_Longlong) reply ), true );
        power_mW = 0.0; /* TODO: What to do when unknown power??? */
    } else {
        /* Convert from W to mW */
        power_mW = std::stod( answer ) * 1000.0;
    }

    return power_mW;
}

int CoboltOfficial::SetLaserPowerSetting( const Power& power )
{
    //std::string answer;
    //int reply;
    //std::string cmd;

    //cmd = "p " + std::to_string( (long double) ( power_mW / 1000.0 ) ); /* Command takes W not mW */

    //reply = SendSerialCmd( cmd, answer );

    //if ( reply == DEVICE_OK ) {
    //    /* Laser has been set to power. Update class member laserPower_ */
    //    laserPowerSetting_ = power_mW;
    //} else {
    //    /* TODO: Error handling for failed send serial command. */
    //    LogMessage( "CoboltOfficial::SetLaserPowerSetting: Failed to set power. Errorcode: " + std::to_string( (_Longlong) reply ), true );
    //}

    //return reply;

}

std::string CoboltOfficial::GetOperatingHours()
{
    //std::string answer;
    //int reply;

    //reply = SendSerialCmd( "hrs?", answer );
    //if ( reply != DEVICE_OK ) {
    //    /* TODO: Error handling for failed send serial command. */
    //    LogMessage( "CoboltOfficial::GetOperatingHours: Failed to get operating hours. Errorcode: " + std::to_string( (_Longlong) reply ), true );
    //    answer = g_Default_Str_Unknown;
    //}

    //return answer;
}

std::string CoboltOfficial::GetLaserStatus() // Whether laser is on or off
{
    //std::string answer;
    //int reply;

    //reply = SendSerialCmd( "l?", answer );

    //if ( reply != DEVICE_OK ) {
    //    /* TODO: Error handling for failed send serial command. */
    //    LogMessage( "CoboltOfficial::GetLaserStatus: Failed to get laser status. Errorcode: " + std::to_string( (_Longlong) reply ), true );
    //    answer.clear();
    //    answer = g_Default_Str_Unknown;
    //} else if ( answer.compare( "0" ) == 0 ) {
    //    answer.clear();
    //    answer = g_PropertyOff;
    //} else if ( answer.compare( "1" ) == 0 ) {
    //    answer.clear();
    //    answer = g_PropertyOn;
    //} else {
    //    /* TODO: Error handling of unknown reply */
    //    LogMessage( "CoboltOfficial::GetLaserStatus: Received unknown string: " + answer, true );
    //    answer.clear();
    //    answer = g_Default_Str_Unknown;
    //}

    //return answer;
}

int CoboltOfficial::SetLaserStatus( std::string status )
{
    //std::string answer;
    //int reply = DEVICE_ERR;

    //if ( status.compare( g_PropertyOn ) == 0 ) {
    //    reply = SendSerialCmd( "@cob1", answer );
    //} else if ( status.compare( g_PropertyOff ) == 0 ) {
    //    reply = SendSerialCmd( "@cob0", answer );
    //} else {
    //    /* TODO: Error handling when unknown status */
    //    LogMessage( "CoboltOfficial::SetLaserStatus: The LaserStatus is not valid. Exp: On/Off Got:" + status, true );
    //    return DEVICE_INVALID_INPUT_PARAM;
    //}

    //if ( reply == DEVICE_OK ) {
    //    /* Laser has been set to status. Update class member laserStatus_ */
    //    laserStatus_ = status;
    //    if ( status.compare( g_PropertyOff ) == 0 ) {
    //        /* Lasers was turned off, thus cannot Pause mode be active */
    //        bLaserIsPaused_ = false;
    //    }
    //} else {
    //    /* TODO: Error handling for failed send serial command. */
    //    LogMessage( "CoboltOfficial::SetLaserStatus: Failed to set laser status. Errorcode: " + std::to_string( (_Longlong) reply ), true );
    //    answer = g_Default_Str_Unknown;
    //}

    //return reply;
}

double CoboltOfficial::GetLaserPowerOutput()
{
    std::string answer;
    double power_mW = 0.0;

    int reply = SendSerialCmd( "pa?", answer ); /* Read output power (W) */
    if ( reply != DEVICE_OK ) {
        /* TODO: Error handling for failed send serial command. */
        LogMessage( "CoboltOfficial::GetLaserPowerOutput: Failed to fetch laser power output. Errorcode: " + std::to_string( (_Longlong) reply ), true );
        answer = g_Default_Str_Double_0;
    } else {
        power_mW = std::stod( answer ) * 1000.0;
    }
    return power_mW;
}

int CoboltOfficial::SetLaserPauseCommand( bool pauseLaserActive )
{
    std::string answer;
    std::string cmd = ( pauseLaserActive == true ? "l0r" : "l1r" );
    int reply = SendSerialCmd( cmd, answer );

    if ( reply != DEVICE_OK ) {
        LogMessage( "CoboltOfficial::SetLaserPauseCommand: Failed to send Laser Pause Command (" + cmd +
            ") Errorcode: " + std::to_string( (_Longlong) reply ), true );
    } else {
        /* Command successful. Update internal status member correspondingly */
        bLaserIsPaused_ = ( pauseLaserActive == true ? true : false );
    }

    return reply;
}

/* TODO: This method is not yet fully implemented!!! */
std::string CoboltOfficial::GetOperatingMode()
{
    std::string answer;
    std::string mode( g_Default_Str_Unknown );

    int reply = SendSerialCmd( "gom?", answer );
    if ( reply != DEVICE_OK ) {
        /* TODO: Error handling for failed send serial command. */
        LogMessage( "CoboltOfficial::GetOperatingMode: Get Operating Mode cmd failed. Errcode: " + std::to_string( (_Longlong) reply ), true );
    } else {
        /* Decode operating mode */
        if ( answer.compare( "0" ) == 0 ) {
            mode = LASER_OFF_MODE;
        } else if ( answer.compare( "2" ) == 0 ) {
            /* Continuous mode. Set operating mode to the last set continous op mode
             * (Constant power or constant current).
             */
            mode = whichContinousOpMode_;
        } else if ( answer.compare( "4" ) == 0 ) {
            /* Modulation */
            mode = MODULATION_MODE;
        } else {
            /* Currently not implemented Laser Operating Mode. Log */
            LogMessage( "CoboltOfficial::GetOperatingMode: Received operating mode not implemented/supported. Got: " + answer, true );
            mode = g_Default_Str_Unknown;
        }
    }

    return mode;
}

int CoboltOfficial::SetLaserOperatingMode( std::string mode )
{
    std::string answer;
    int reply = DEVICE_ERR;

    if ( mode.compare( CONSTANT_POWER_MODE ) == 0 ) {
        reply = SendSerialCmd( "cp", answer );
    } else if ( mode.compare( CONSTANT_CURRENT_MODE ) == 0 ) {
        reply = SendSerialCmd( "ci", answer );
    } else if ( mode.compare( MODULATION_MODE ) == 0 ) {
        reply = SendSerialCmd( "em", answer );
    } else if ( mode.compare( LASER_OFF_MODE ) == 0 ) {
        return ERR_CANNOT_SET_MODE_OFF;
    } else {
        /* Mode unknown or not supported */
        return ERR_LASER_OPERATING_MODE_NOT_SUPPORTED;
    }

    if ( reply == DEVICE_OK ) {
        /* Update Laser operating mode member */
        laserOperatingMode_ = mode;
        if ( ( mode.compare( CONSTANT_POWER_MODE ) == 0 ) ||
            ( mode.compare( CONSTANT_CURRENT_MODE ) == 0 ) ) {
            /* Need to keep track which continuous mode is selected */
            whichContinousOpMode_ = mode;
        }
    } else {
        /* TODO: Error handling when failed to set supported Laser operating mode */
        LogMessage( "CoboltOfficial::SetLaserOperatingMode: Failed to set supported Laser Operating Mode (" +
            mode + ") Errcode:" + std::to_string( (_Longlong) reply ), true );
    }

    return reply;
}

double CoboltOfficial::GetLaserDriveCurrent()
{
    //std::string answer;
    //int reply = SendSerialCmd( "i?", answer ); /* Read drive current (mA) */ // TODO: glc? not i? (i? returns measured current)

    //if ( reply != DEVICE_OK ) {
    //    /* TODO: Error handling for failed send serial command. */
    //    LogMessage( "CoboltOfficial::GetLaserDriveCurrent: Failed to set drive current. Errorcode: " + std::to_string( (_Longlong) reply ), true );
    //    answer = g_Default_Str_Double_0; /* TODO: What to do when unknown current??? */
    //}

    //return std::stod( answer );
}

int CoboltOfficial::SetLaserDriveCurrent( double mA )
{
    //std::string answer;
    //int reply;

    //std::string cmd = "slc " + std::to_string( (long double) mA );

    //reply = SendSerialCmd( cmd, answer );

    //if ( reply == DEVICE_OK ) {
    //    /* Laser has been set to power. Update class member laserPower_ */
    //    laserCurrentSetting_ = mA;
    //} else {
    //    /* TODO: Error handling for failed send serial command. */
    //    LogMessage( "CoboltOfficial::SetLaserDriveCurrent: Failed to set drive current. Errorcode: " + std::to_string( (_Longlong) reply ), true );
    //}

    //return reply;
}

std::string CoboltOfficial::GetDigitalModulationState()
{
    std::string answer;
    std::string state;

    int reply = SendSerialCmd( "gdmes?", answer );

    if ( reply != DEVICE_OK ) {
        /* TODO: Error handling for failed send serial command. */
        LogMessage( "CoboltOfficial::GetDigitalModulationStatus: Failed to fetch digital modulation state. Errorcode: " + std::to_string( (_Longlong) reply ), true );
        state = digitalModulationState_;
    } else {
        if ( answer.compare( "0" ) == 0 ) {
            state = g_PropertyDisabled;
        } else if ( answer.compare( "1" ) == 0 ) {
            state = g_PropertyEnabled;
        } else {
            /* TODO: Error handling of unknown digital modulation state */
            LogMessage( "CoboltOfficial::GetDigitalModulationStatus: Unknown Digital modulation state. Exp: 0 or 1 Got: " + answer, true );
            state = digitalModulationState_;
        }
    }

    return state;

}

int CoboltOfficial::SetDigitalModulationState( std::string state )
{
    std::string answer;
    int reply = DEVICE_ERR;

    if ( state.compare( g_PropertyEnabled ) == 0 ) {
        reply = SendSerialCmd( "sdmes 1", answer );
    } else if ( state.compare( g_PropertyDisabled ) == 0 ) {
        reply = SendSerialCmd( "sdmes 0", answer );
    } else {
        /* TODO: Error handling for unknown state */
        LogMessage( "CoboltOfficial::SetDigitalModulationState: Unknown state. Exp: Enabled/Disabled Got:" + state, true );
        return DEVICE_INVALID_INPUT_PARAM;
    }

    if ( reply == DEVICE_OK ) {
        /* Digital modulation status has been updated. */
        digitalModulationState_ = state;
    } else {
        /* TODO: Error handling for failed send serial command. */
        LogMessage( "CoboltOfficial::SetDigitalModulationState: Failed to set digital modulation state. ErrorCode: " + std::to_string( (_Longlong) reply ), true );
    }

    return reply;

}

std::string CoboltOfficial::GetAnalogModulationState()
{
    std::string answer;
    std::string state;

    int reply = SendSerialCmd( "games?", answer );

    if ( reply != DEVICE_OK ) {
        /* TODO: Error handling for failed send serial command. */
        LogMessage( "CoboltOfficial::GetAnalogModulationState: Failed to fetch analog modulation state. Errorcode: " + std::to_string( (_Longlong) reply ), true );
        state = analogModulationState_;
    } else {
        if ( answer.compare( "0" ) == 0 ) {
            state = g_PropertyDisabled;
        } else if ( answer.compare( "1" ) == 0 ) {
            state = g_PropertyEnabled;
        } else {
            /* TODO: Error handling of unknown digital modulation state */
            LogMessage( "CoboltOfficial::GetAnalogModulationState: Unknown Analog modulation state. Exp: 0 or 1 Got: " + answer, true );
            state = analogModulationState_;
        }
    }

    return state;

}

int CoboltOfficial::SetAnalogModulationState( std::string state )
{
    std::string answer;
    int reply = DEVICE_ERR;

    if ( state.compare( g_PropertyEnabled ) == 0 ) {
        reply = SendSerialCmd( "sames 1", answer );
    } else if ( state.compare( g_PropertyDisabled ) == 0 ) {
        reply = SendSerialCmd( "sames 0", answer );
    } else {
        /* TODO: Error handling for unknown state */
        LogMessage( "CoboltOfficial::SetAnalogModulationState: Unknown state. Exp: Enabled/Disabled Got:" + state, true );
        return DEVICE_INVALID_INPUT_PARAM;
    }

    if ( reply == DEVICE_OK ) {
        /* Analog modulation status has been updated. */
        analogModulationState_ = state;
    } else {
        /* TODO: Error handling for failed send serial command. */
        LogMessage( "CoboltOfficial::SetAnalogModulationState: Failed to set analog modulation state. ErrorCode: " + std::to_string( (_Longlong) reply ), true );
    }

    return reply;

}

double CoboltOfficial::GetModulationPowerSetting()
{
    std::string answer;

    int reply = SendSerialCmd( "glmp?", answer ); /* Read output power (mW) */
    if ( reply != DEVICE_OK ) {
        /* TODO: Error handling for failed send serial command. */
        LogMessage( "CoboltOfficial::GetModulationPowerSetting: Failed to fetch modulation power setting. Errorcode: " + std::to_string( (_Longlong) reply ), true );
        answer = g_Default_Str_Double_0; /* FIX: What to do when unknown power??? */
    }

    return std::stod( answer );
}

int CoboltOfficial::SetModulationPowerSetting( double power_mW )
{
    std::string answer;
    int reply;

    std::string cmd = "slmp " + std::to_string( (long double) power_mW );

    reply = SendSerialCmd( cmd, answer );

    if ( reply == DEVICE_OK ) {
        /* Laser has been set to power. Update class member laserPower_ */
        modulationPowerSetting_ = power_mW;
    } else {
        /* TODO: Error handling for failed send serial command. */
        LogMessage( "CoboltOfficial::SetModulationPowerSetting: Failed to set modulation power setting. Errorcode: " + std::to_string( (_Longlong) reply ), true );
    }

    return reply;

}

std::string CoboltOfficial::GetAnalogImpedanceState()
{
    std::string answer;
    std::string state;

    int reply = SendSerialCmd( "galis?", answer );

    if ( reply != DEVICE_OK ) {
        /* TODO: Error handling for failed send serial command. */
        LogMessage( "CoboltOfficial::GetAnalogImpedanceState: Failed to fetch analog impedance state. Errorcode: " + std::to_string( (_Longlong) reply ), true );
        state = analogImpedanceState_;
    } else {
        if ( answer.compare( "0" ) == 0 ) {
            /* Analog Low Impedance state is disabled (1 kOhm)*/
            state = g_PropertyHighImp;
        } else if ( answer.compare( "1" ) == 0 ) {
            /* Analog Low Impedance state is enabled (50 Ohm)*/
            state = g_PropertyLowImp;
        } else {
            /* TODO: Error handling of unknown analog impedance state */
            LogMessage( "CoboltOfficial::GetAnalogImpedanceState: Unknown Analog Impedance state. Exp: 0 or 1 Got: " + answer, true );
            state = analogImpedanceState_;
        }
    }

    return state;

}

int CoboltOfficial::SetAnalogImpedanceState( std::string state )
{
    std::string answer;
    int reply = DEVICE_ERR;

    if ( state.compare( g_PropertyLowImp ) == 0 ) {
        /* Analog impedance state = Low */
        reply = SendSerialCmd( "salis 1", answer );
    } else if ( state.compare( g_PropertyHighImp ) == 0 ) {
        /* Analog impedance state = High */
        reply = SendSerialCmd( "salis 0", answer );
    } else {
        /* TODO: Error handling for unknown state */
        LogMessage( "CoboltOfficial::SetAnalogImpedanceState: Unknown impedance state. Exp: 50 Ohm or 1 kOhm Got:" + state, true );
        return DEVICE_INVALID_INPUT_PARAM;
    }

    if ( reply == DEVICE_OK ) {
        /* Analog impedance state has been updated. */
        analogImpedanceState_ = state;
    } else {
        /* TODO: Error handling for failed send serial command. */
        LogMessage( "CoboltOfficial::SetAnalogImpedanceState: Failed to set analog impedance state. Errorcode: " + std::to_string( (_Longlong) reply ), true );
    }

    return reply;

}

