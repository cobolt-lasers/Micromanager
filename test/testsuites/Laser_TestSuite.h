/**
 * \file        Laser_TestSuite.h
 *
 * \authors     Lukas Kalinski
 *
 * \copyright   Cobolt AB, 2020. All rights reserved.
 */

#include <cxxtest/TestSuite.h>
#include "Laser.h"

using namespace cobolt;

class GuiPropertyMock : public GuiProperty
{
public:
    
    GuiPropertyMock( const std::string& string ) :
        value( string )
    {}

    virtual bool Set( const std::string& string ) { value = string; return true; }
    virtual bool Get( std::string& string ) const { string = value; return true; }
    std::string value;
};

class Laser_TestSuite : public CxxTest::TestSuite, public LaserDevice
{
    struct PhysicalLaserMock
    {
        PhysicalLaserMock() :
            firmwareVersion( "1.2.3" ),
            isOn( false )
        {}

        std::string firmwareVersion;
        bool isOn;
    };

    PhysicalLaserMock _physicalLaserMock;
    Laser* _someLaser;
    std::string _lastReceivedCommand;

public:

    void setUp()
    {
        _someLaser = Laser::Create( "-06-" );
        _someLaser->SetupWithLaserDevice( this );
        _lastReceivedCommand.clear();
    }

    void tearDown()
    {
        delete _someLaser;
    }

    void test_GetProperty_firmware()
    {
        TS_ASSERT_EQUALS( _someLaser->GetProperty( laser::property::firmware_version )->Get<std::string>(), _physicalLaserMock.firmwareVersion );
    }

    void test_OnGuiSetAction_toggle_on()
    {
        /// ###
        /// Setup

        _someLaser->SetOn( false );
        GuiPropertyMock guiProperty( "On" );
        
        /// ###
        /// Verify Setup

        TS_ASSERT( !_someLaser->IsOn() );
        
        /// ###
        /// Test

        _someLaser->GetProperty( laser::property::toggle )->OnGuiSetAction( guiProperty );

        /// ###
        /// Verify
        
        TS_ASSERT( _physicalLaserMock.isOn );
    }

    void test_OnGuiSetAction_toggle_off()
    {
        /// ###
        /// Setup

        _physicalLaserMock.isOn = true;
        GuiPropertyMock guiProperty( "Off" );

        /// ###
        /// Test

        _someLaser->GetProperty( laser::property::toggle )->OnGuiSetAction( guiProperty );

        /// ###
        /// Verify

        TS_ASSERT( !_physicalLaserMock.isOn );
    }

    /// ###
    /// Laser Device API

    virtual int SendCommand( const std::string& command, std::string* response = NULL )
    {
        if ( command == "gfv?" ) { *response = _physicalLaserMock.firmwareVersion; }
        if ( command == "l1" )   { _physicalLaserMock.isOn = true; }
        if ( command == "l0" )   { _physicalLaserMock.isOn = false; }
        
        _lastReceivedCommand = command;

        return cobolt::return_code::ok;
    }
};