/**
 * \file        CoboltOfficial.cpp
 *
 * \brief       Official device adapter for COBOLT lasers.
 *
 * \authors     Johan Crone, Lukas Kalinski
 *
 * \copyright   Cobolt AB, 2020. All rights reserved. // TODO: Set proper license
 */

#include "CoboltOfficial.h"
#include "Power.h"
#include "Current.h"

using namespace std;
using namespace cobolt;

//////////////////////////////////////////////////////////////////////////////
// Device Properties strings
//
const char * g_DeviceName = "CoboltOfficial";
const char * g_DeviceDescription = "COBOLT Official Laser Controller";
const char * g_DeviceVendorName = "COBOLT: a HÜBNER Group Company";

//////////////////////////////////////////////////////////////////////////////
// Commonly used strings
//

const char* const g_Property_Port_None = "None";

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

class GuiPropertyAdapter : public cobolt::GuiProperty
{
public:

    GuiPropertyAdapter( MM::PropertyBase* mm_property ) : mm_property_( mm_property ) {}
    virtual bool Set( const std::string& value ) { return mm_property_->Set( value.c_str() ); }
    virtual bool Get( std::string& value ) const { return mm_property_->Get( value ); }
    
private:

    MM::PropertyBase* mm_property_;
};

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
    SetErrorText( ERR_SERIAL_PORT_NOT_SELECTED,             "Serial port must not be undefined when initializing!"                  );
    SetErrorText( OPERATING_SHUTTER_WITH_LASER_OFF,         "Cannot operate shutter while the Laser is turned off!"                 );

    // Create properties:
    CreateProperty( MM::g_Keyword_Name,         g_DeviceName,               MM::String, true );
    CreateProperty( "Vendor",                   g_DeviceVendorName,         MM::String, true );
    CreateProperty( MM::g_Keyword_Description,  g_DeviceDescription,        MM::String, true );
    CreateProperty( MM::g_Keyword_Port,         g_Property_Port_None,       MM::String, false, new CPropertyAction( this, &CoboltOfficial::OnPropertyAction_Port ), true );
    
    UpdateStatus();
}

CoboltOfficial::~CoboltOfficial()
{
    Shutdown();
}

int CoboltOfficial::Initialize()
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
int CoboltOfficial::SendCommand( const std::string& command, std::string* response )
{
    int reply = SendSerialCommand( port_.c_str(), command.c_str(), "\r" );

    if ( reply != DEVICE_OK ) {

        LogMessage( "CoboltOfficial::SendSerialCmd: SendSerialCommand Failed: " + std::to_string( (_Longlong) reply ), true );

    } else if ( response != NULL ) {

        reply = GetSerialAnswer( port_.c_str(), "\r\n", *response );

        if ( reply != DEVICE_OK ) {

            LogMessage( "CoboltOfficial::SendSerialCmd: GetSerialAnswer Failed: " + std::to_string( (_Longlong) reply ), true );

        } else if ( response->find( "error" ) != std::string::npos ) { // TODO: make find case insensitive

            LogMessage( "CoboltOfficial::SendSerialCmd: Sent: " + command + " Reply received: " + response, true );
            reply = DEVICE_UNSUPPORTED_COMMAND;
        }
    }

    return reply;
}

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

int CoboltOfficial::OnPropertyAction_Laser( MM::PropertyBase* mm_property, MM::ActionType action )
{
    if ( action == MM::BeforeGet ) {
        laser_->GetProperty( mm_property->GetName() )->OnGuiGetAction( GuiPropertyAdapter( mm_property ) );
    } else if ( action == MM::AfterSet ) {
        laser_->GetProperty( mm_property->GetName() )->OnGuiSetAction( GuiPropertyAdapter( mm_property ) );
    }
}

MM::PropertyType CoboltOfficial::ResolvePropertyType( const cobolt::Property::Stereotype stereotype ) const
{
    switch ( stereotype ) {

        case Property::Float:   return MM::PropertyType::Float;
        case Property::Integer: return MM::PropertyType::Integer;
        case Property::String:  return MM::PropertyType::String;
    }
}

int CoboltOfficial::ExposeToGui( const Property* property )
{
    CPropertyAction* action = new CPropertyAction( this, &CoboltOfficial::OnPropertyAction_Laser );
    return CreateProperty(
        property->Name().c_str(),
        property->Get<std::string>().c_str(),
        ResolvePropertyType( property->GetStereotype() ),
        !property->MutableInGui(),
        action );
}
