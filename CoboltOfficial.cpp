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
    cobolt::Logger::Instance()->SetupWithGateway( this );

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
    CreateProperty( MM::g_Keyword_Name,         g_DeviceName,               MM::String, true );
    CreateProperty( "Vendor",                   g_DeviceVendorName,         MM::String, true );
    CreateProperty( MM::g_Keyword_Description,  g_DeviceDescription,        MM::String, true );
    CreateProperty( MM::g_Keyword_Port,         g_Property_Unknown_Value,   MM::String, false, new CPropertyAction( this, &CoboltOfficial::OnPropertyAction_Port ), true );
    
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
    if ( !laser_->IsOn() ) {
        return OPERATING_SHUTTER_WITH_LASER_OFF;
    }
    
    if ( open ) {
        laser_->SetPaused( false );
    } else {
        laser_->SetPaused( true );
    }
    
    return DEVICE_OK;
}

/**
 * Tells whether the shutter is open or not (i.e. whether laser is shining or not).
 */
int CoboltOfficial::GetOpen( bool& open )
{
    open = ( laser_->IsOn() && !laser_->IsPaused() );

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
        CDeviceUtils::SleepMs( (long) ( deltaT + 0.5f ) );

        reply = SetOpen( false );
    }

    return reply;
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
    
    for ( Laser::PropertyIterator it = laser_->GetPropertyIteratorBegin(); it != laser_->GetPropertyIteratorEnd(); it++ ) {
        ExposeToGui( it->second );
        it->second->IntroduceToGuiEnvironment( this );
    }

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

int CoboltOfficial::OnPropertyAction_Port( MM::PropertyBase* guiProperty, MM::ActionType action )
{
    if ( action == MM::BeforeGet ) {

        guiProperty->Set( port_.c_str() );

    } else if ( action == MM::AfterSet ) {

        if ( bInitialized_ ) {
            
            // Port change after initialization not allowed, thus reset port value:
            guiProperty->Set( port_.c_str() );
            
            return ERR_PORT_CHANGE_FORBIDDEN;
        }

        guiProperty->Get( port_ );
    }

    return DEVICE_OK;
}

int CoboltOfficial::OnPropertyAction_Laser( MM::PropertyBase* guiProperty, MM::ActionType action )
{
    laser_->GetProperty( guiProperty->GetName() )->OnGuiAction( guiProperty, action );
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

int CoboltOfficial::ExposeToGui( const Property* property )
{
    CPropertyAction* action = new CPropertyAction( this, &CoboltOfficial::OnPropertyAction_Laser );
    return CreateProperty( property->Name(), property->Get<std::string>().c_str(), property->TypeInGui(), !property->MutableInGui(), action );
}
