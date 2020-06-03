///////////////////////////////////////////////////////////////////////////////
// FILE:       SkyraLaser.cpp
// PROJECT:    MicroManager
// SUBSYSTEM:  DeviceAdapters
//-----------------------------------------------------------------------------
// DESCRIPTION:
// Cobolt Lasers Controller Adapter
//
// COPYRIGHT:     Cobolt AB, Stockholm, 2020
//                All rights reserved
//
// LICENSE:       MIT
//                Permission is hereby granted, free of charge, to any person obtaining a
//                copy of this software and associated documentation files( the "Software" ),
//                to deal in the Software without restriction, including without limitation the
//                rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
//                sell copies of the Software, and to permit persons to whom the Software is
//                furnished to do so, subject to the following conditions:
//                
//                The above copyright notice and this permission notice shall be included in all
//                copies or substantial portions of the Software.
//
//                THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
//                INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
//                PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
//                HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
//                OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
//                SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

// AUTHORS:
//                Lukas Kalinski / lukas.kalinski@coboltlasers.com (2020)
//

#include "SkyraLaser.h"

//#include "StaticStringProperty.h"
#include "DeviceProperty.h"
//#include "ImmutableEnumerationProperty.h"
#include "LaserStateProperty.h"
//#include "MutableDeviceProperty.h"
#include "EnumerationProperty.h"
#include "NumericProperty.h"
//#include "LaserShutterProperty.h"
//#include "NoShutterCommandLegacyFix.h"

using namespace std;
using namespace cobolt;

SkyraLaser::SkyraLaser( LaserDriver* driver ) :
    Laser( "Skyra", driver )
{
    currentUnit_ = Milliamperes;
    powerUnit_ = Milliwatts;
    
    CreateNameProperty();
    CreateModelProperty();
    CreateSerialNumberProperty();
    CreateFirmwareVersionProperty();
    CreateAdapterVersionProperty();
    CreateOperatingHoursProperty();

    //laser->CreateWavelengthProperty( wavelength );  // TODO: Per line property
    CreateKeyswitchProperty();
    //laser->CreateLaserStateProperty<ST_Skyra>();
    //laser->CreateShutterProperty();                 // TODO: Per line property
    //laser->CreateRunModeProperty<ST_Skyra>();       // TODO: Per line property
    //laser->CreatePowerSetpointProperty();           // TODO: Per line property
    //laser->CreatePowerReadingProperty();            // TODO: Per line property
    //laser->CreateCurrentSetpointProperty();         // TODO: Per line property
    //laser->CreateCurrentReadingProperty();          // TODO: Per line property
}

void SkyraLaser::CreateCurrentSetpointProperty( const int line )
{
    //std::string maxCurrentSetpointResponse;
    //if ( laserDriver_->SendCommand( "gmlc?", &maxCurrentSetpointResponse ) != return_code::ok ) {

    //    Logger::Instance()->LogError( "SkyraLaser::CreateCurrentSetpointProperty(): Failed to retrieve max current sepoint" );
    //    return;
    //}

    //const double maxCurrentSetpoint = atof( maxCurrentSetpointResponse.c_str() );

    //MutableDeviceProperty* property;
   
    //if ( IsShutterCommandSupported() || !IsInCdrhMode() ) {
    //    property = new NumericProperty<double>( "Current Setpoint [" + currentUnit_ + "]", laserDriver_, "glc?", "slc", 0.0f, maxCurrentSetpoint );
    //} else {
    //    property = new legacy::no_shutter_command::LaserCurrentProperty( "Current Setpoint [" + currentUnit_ + "]", laserDriver_, "glc?", "slc", 0.0f, maxCurrentSetpoint, this );
    //}

    //RegisterPublicProperty( property );
}

void SkyraLaser::CreateCurrentReadingProperty( const int line )
{
    DeviceProperty* property = new DeviceProperty( Property::Float, "Measured Current [" + currentUnit_ + "]", laserDriver_, "i?" );
    property->SetCaching( false );
    RegisterPublicProperty( property );
}

void SkyraLaser::CreatePowerSetpointProperty( const int line )
{
    std::string maxPowerSetpointResponse;
    if ( laserDriver_->SendCommand( "gmlp?", &maxPowerSetpointResponse ) != return_code::ok ) {

        Logger::Instance()->LogError( "SkyraLaser::CreatePowerSetpointProperty(): Failed to retrieve max power sepoint" );
        return;
    }

    const double maxPowerSetpoint = atof( maxPowerSetpointResponse.c_str() );
    
    MutableDeviceProperty* property = new NumericProperty<double>( "Power Setpoint [" + powerUnit_ + "]", laserDriver_, "glp?", "slp", 0.0f, maxPowerSetpoint );
    RegisterPublicProperty( property );
}

void SkyraLaser::CreatePowerReadingProperty( const int line )
{
    DeviceProperty* property = new DeviceProperty( Property::String, "Power Reading [" + powerUnit_ + "]", laserDriver_, "pa?" );
    property->SetCaching( false );
    RegisterPublicProperty( property );
}

void SkyraLaser::CreateLaserStateProperty()
{
    if ( IsInCdrhMode() ) {

        laserStateProperty_ = new LaserStateProperty( Property::String, "Laser State", laserDriver_, "gom?" );

        laserStateProperty_->RegisterState( "0", "Off", false );
        laserStateProperty_->RegisterState( "1", "Waiting for TEC", false );
        laserStateProperty_->RegisterState( "2", "Waiting for Key", false );
        laserStateProperty_->RegisterState( "3", "Warming Up", false );
        laserStateProperty_->RegisterState( "4", "Completed", true );
        laserStateProperty_->RegisterState( "5", "Fault", false );
        laserStateProperty_->RegisterState( "6", "Aborted", false );
        laserStateProperty_->RegisterState( "7", "Waiting for Remote", false );
        laserStateProperty_->RegisterState( "8", "Standby", false );

    } else {

        laserStateProperty_ = new LaserStateProperty( Property::String, "Laser State", laserDriver_, "l?" );

        laserStateProperty_->RegisterState( "0", "Off", true );
        laserStateProperty_->RegisterState( "1", "On", true );
    }

    RegisterPublicProperty( laserStateProperty_ );
}

void SkyraLaser::CreateShutterProperty()
{
    //if ( IsShutterCommandSupported() ) {
    //    shutter_ = new LaserShutterProperty( "Emission Status", laserDriver_, this );
    //} else {

    //    if ( IsInCdrhMode() ) {
    //        shutter_ = new legacy::no_shutter_command::LaserShutterPropertyCdrh( "Emission Status", laserDriver_, this );
    //    } else {
    //        shutter_ = new legacy::no_shutter_command::LaserShutterPropertyOem( "Emission Status", laserDriver_, this );
    //    }
    //}
    //
    //RegisterPublicProperty( shutter_ );
}

void SkyraLaser::CreateRunModeProperty( const int line )
{
    //EnumerationProperty* property;
    //
    //if ( IsShutterCommandSupported() || !IsInCdrhMode() ) {
    //    property = new EnumerationProperty( "Run Mode", laserDriver_, "gam?" );
    //} else {
    //    property = new legacy::no_shutter_command::LaserRunModeProperty( "Run Mode", laserDriver_, "gam?", this );
    //}
    //
    //property->SetCaching( false );

    //property->RegisterEnumerationItem( "0", "ecc", EnumerationItem_RunMode_ConstantCurrent );
    //property->RegisterEnumerationItem( "1", "ecp", EnumerationItem_RunMode_ConstantPower );
    //property->RegisterEnumerationItem( "2", "em", EnumerationItem_RunMode_Modulation );
    //
    //RegisterPublicProperty( property );
}
