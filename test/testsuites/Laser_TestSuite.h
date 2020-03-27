/**
 * \file        Laser_TestSuite.h
 *
 * \authors     Lukas Kalinski
 *
 * \copyright   Cobolt AB, 2020. All rights reserved.
 */

#include <cxxtest/TestSuite.h>
#include "Laser.h"

#define FIRMWARE_VERSION "1.2.3"

class LaserDeviceMock : public cobolt::LaserDevice
{
public:
    
    virtual int SendCommand( const std::string& command, std::string* response = NULL )
    {
        receivedCommand = command;

        if ( command == "gfv?" ) { *response = FIRMWARE_VERSION; }
    }

    std::string receivedCommand;
};

using namespace cobolt;

class MyTestSuite1 : public CxxTest::TestSuite
{
    Laser* _someLaser;

public:

    void setUp()
    {
        _someLaser = Laser::Create( "-06-" );
    }

    void tearDown()
    {
        delete _someLaser;
    }

    void test_GetProperty_firmware()
    {
        TS_ASSERT_EQUALS( _someLaser->GetProperty( laser::property::firmware_version )->Get<std::string>(), FIRMWARE_VERSION );
    }
};