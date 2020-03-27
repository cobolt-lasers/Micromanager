/**
 * \file        Laser_TestSuite.h
 *
 * \authors     Lukas Kalinski
 *
 * \copyright   Cobolt AB, 2020. All rights reserved.
 */

#include <cxxtest/TestSuite.h>

class MyTestSuite1 : public CxxTest::TestSuite
{
public:
    void testAddition( void )
    {
        TS_ASSERT( 1 + 1 > 1 );
        TS_ASSERT_EQUALS( 1 + 1, 2 );
    }
};