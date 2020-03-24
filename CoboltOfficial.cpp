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
// Commonly used strings
//
const char * g_SendTerm = "\r";
const char * g_RecvTerm = "\r\n";

const char* const g_Property_Port_None = "None";

const char* const g_Property_Unknown_Value = "Unknown";

const char * const g_Default_Str_Empty = "";
//const char * const g_Default_Str_Unknown = "Unknown";
const char * const g_Default_Str_Double_0 = "0.0";

//////////////////////////////////////////////////////////////////////////////
// Cobolt Laser constants
//
const std::string MODEL_06 = "06-laser";
const std::string MODEL_08 = "08-laser";
const std::string MODEL_SKYRA = "Skyra-laser";

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
    port_( "None" ),
    bBusy_( false )
{
    cobolt::Logger::Instance().SetupWithGateway( this );

    // TODO Auto-generated constructor stub
    assert( strlen( g_DeviceName ) < (unsigned int) MM::MaxStrLength );

    InitializeDefaultErrorMessages();

    // Map error codes to strings:
    SetErrorText( ERR_PORT_CHANGE_FORBIDDEN,                "You can't change the port after device has been initialized."          );
    SetErrorText( ERR_SERIAL_PORT_NOT_SELECTED,                "Serial port must not be undefined when initializing!"                  );
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
    if ( laser_->toggle->Fetch() != laser::toggle::on ) {
        return OPERATING_SHUTTER_WITH_LASER_OFF;
    }
    
    if ( open ) {
        laser_->paused->Set( false ); // laser_->UnpauseShining()
    } else {
        laser_->paused->Set( true ); // laser_->PauseShining()
    }

    if ( !laser_->paused->LastRequestSuccessful() ) {
        return DEVICE_ERR;
    }
    
    return DEVICE_OK;
}

/**
 * Tells whether the shutter is open or not (i.e. whether laser is shining or not).
 */
int CoboltOfficial::GetOpen( bool& open )
{
    open = ( laser_->toggle->Fetch() == laser::toggle::on && !laser_->paused->Fetch() );

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

int CoboltOfficial::ExposeToGui( const Property* property )
{
    CPropertyAction* action = new CPropertyAction( this, &CoboltOfficial::OnLaserPropertyAction );
    return CreateProperty( property->Name(), property->FetchAsString().c_str(), property->TypeInGui(), !property->MutableInGui(), action );
}

int CoboltOfficial::Initialize() // TODO NOW: implement this then make laser model adaptation work
{
    if ( bInitialized_ ) {
        return DEVICE_OK;
    }

    if ( port_ == g_Property_Port_None ) {
        
        LogMessage( "CoboltOfficial::Initialize(): Serial port not selected", true );
        return ERR_SERIAL_PORT_NOT_SELECTED;
    }

    laser_->SetupWithLaserDevice( this );

    int result;

    Laser::PropertyIterator it = laser_->PropertyIteratorBegin();
    while ( it != laser_->PropertyIteratorEnd() ) {
        ExposeToGui( it->second );
        it->second->IntroduceToGuiEnvironment( this );
    }
    

    /* LASER POWER SETTING [mW] (Initialise to safe state, i.e. 0.0) */
    laserPowerSetting_ = 0.0;
    result = SetLaserPowerSetting( laserPowerSetting_ );
    if ( result != DEVICE_OK ) {
        /* Failed to update laser with laserpowersetting */
        return result;
    }
    pAct = new CPropertyAction( this, &CoboltOfficial::OnPropertyAction_PowerSetpoint );
    result = CreateProperty( g_PropertyLaserPowerSetting, g_Default_Str_Double_0, MM::Float, false, pAct );
    if ( DEVICE_OK != result ) {
        return result;
    }
    /* Create limits 0.0 ... Laser Max Power Setting */
    SetPropertyLimits( g_PropertyLaserPowerSetting, 0.0, ( laserMaxPower_ * 1000.0 ) );

    /* LASER CURRENT SETTING [mA] (Initialise to safe state, i.e. 0.0) */
    laserCurrentSetting_ = 0.0;
    result = SetLaserDriveCurrent( laserCurrentSetting_ );
    if ( result != DEVICE_OK ) {
        /* Failed to update laser with laserdrivecurrent */
        return result;
    }
    pAct = new CPropertyAction( this, &CoboltOfficial::OnPropertyAction_CurrentSetpoint );
    result = CreateProperty( g_PropertyLaserCurrentSetting, g_Default_Str_Double_0, MM::Float, false, pAct );
    if ( DEVICE_OK != result ) {
        return result;
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
    result = CreateProperty( g_PropertyLaserStatus, laserStatus_.c_str(), MM::String, false, pAct );
    if ( DEVICE_OK != result ) {
        return result;
    }
    /* Create allowed values On/Off */
    allowedValues.clear();
    allowedValues.push_back( g_PropertyOn );
    allowedValues.push_back( g_PropertyOff );
    SetAllowedValues( g_PropertyLaserStatus, allowedValues );

    /* LASER OPERATING MODE  (Initialise to Constant Power)*/
    result = SetLaserOperatingMode( CONSTANT_POWER_MODE );
    if ( DEVICE_OK != result ) {
        return result;
    }
    /* Note that as long LaserStatus is Off, the mode returned from laser is OFF */
    laserOperatingMode_ = LASER_OFF_MODE;
    pAct = new CPropertyAction( this, &CoboltOfficial::OnPropertyAction_RunMode );
    result = CreateProperty( g_PropertyOperatingMode, laserOperatingMode_.c_str(), MM::String, false, pAct );
    if ( DEVICE_OK != result ) {
        return result;
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
    result = CreateProperty( g_PropertyCurrentLaserOutput, tmpString.c_str(), MM::String, true, pAct );
    if ( DEVICE_OK != result ) {
        return result;
    }

    /* MODULATION LASER POWER [mW]  (Initialise to safe state, i.e. 0.0) */
    /* TODO: If set modulation laser power command is not supported, skip the property */
    modulationPowerSetting_ = 0.0;
    result = SetModulationPowerSetting( modulationPowerSetting_ );
    if ( DEVICE_OK == result ) {
        pAct = new CPropertyAction( this, &CoboltOfficial::OnPropertyAction_ModulationPowerSetpoint );
        result = CreateProperty( g_PropertyModulationPower, std::to_string( (long double) modulationPowerSetting_ ).c_str(), MM::Float, false, pAct );
        if ( DEVICE_OK != result ) {
            return result;
        }
        /* TODO: Is it correct to use laser max power as max laser modulation power? */
        /* Create limits 0.0 ... Laser Max Power */
        SetPropertyLimits( g_PropertyModulationPower, 0.0, ( laserMaxPower_ * 1000.0 ) );
    }

    /* DIGITAL MODULATION STATE (Enabled/Disabled) */
    digitalModulationState_ = GetDigitalModulationState();
    pAct = new CPropertyAction( this, &CoboltOfficial::OnPropertyAction_DigitalModulationFlag );
    result = CreateProperty( g_PropertyDigitalModulationState, digitalModulationState_.c_str(), MM::String, false, pAct );
    if ( DEVICE_OK != result ) {
        return result;
    }
    /* Create allowed values */
    allowedValues.clear();
    allowedValues.push_back( g_PropertyEnabled );
    allowedValues.push_back( g_PropertyDisabled );
    SetAllowedValues( g_PropertyDigitalModulationState, allowedValues );

    /* ANALOG MODULATION STATE (Enabled/Disabled) */
    analogModulationState_ = GetAnalogModulationState();
    pAct = new CPropertyAction( this, &CoboltOfficial::OnPropertyAction_AnalogModulationFlag );
    result = CreateProperty( g_PropertyAnalogModulationState, analogModulationState_.c_str(), MM::String, false, pAct );
    if ( DEVICE_OK != result ) {
        return result;
    }
    /* Create allowed values */
    allowedValues.clear();
    allowedValues.push_back( g_PropertyEnabled );
    allowedValues.push_back( g_PropertyDisabled );
    SetAllowedValues( g_PropertyAnalogModulationState, allowedValues );

    /* ANALOG IMPEDANCE STATE (50 Ohm/1 kOhm) */
    analogImpedanceState_ = GetAnalogImpedanceState();
    pAct = new CPropertyAction( this, &CoboltOfficial::OnPropertyAction_AnalogImpedance );
    result = CreateProperty( g_PropertyAnalogImpedanceState, analogImpedanceState_.c_str(), MM::String, false, pAct );
    if ( DEVICE_OK != result ) {
        return result;
    }
    /* Create allowed values */
    allowedValues.clear();
    allowedValues.push_back( g_PropertyLowImp );
    allowedValues.push_back( g_PropertyHighImp );
    SetAllowedValues( g_PropertyAnalogImpedanceState, allowedValues );

    /* Laser is completely initialised */
    bInitialized_ = true;

    return DEVICE_OK;
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

bool CoboltOfficial::Busy()
{
    return bBusy_;
}

void CoboltOfficial::GetName( char* name ) const
{
    CDeviceUtils::CopyLimitedString( name, g_DeviceName );
}

//////////////////////////////////////////////////////////////////////////////
// Action interface
//

int CoboltOfficial::OnLaserPropertyAction( MM::PropertyBase* guiProperty, MM::ActionType action )
{
    laser_->Property( guiProperty->GetName() )->OnGuiAction( guiProperty, action );
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
