/**
 * \file        LegacyLaserShutterProperty.h
 *
 * \authors     Lukas Kalinski
 *
 * \copyright   Cobolt AB, 2020. All rights reserved. // TODO: Set proper license
 */

#ifndef __COBOLT__LEGACY_LASER_SHUTTER_PROPERTY_H
#define __COBOLT__LEGACY_LASER_SHUTTER_PROPERTY_H

#include "EnumerationProperty.h"
#include "NumericProperty.h"

NAMESPACE_COBOLT_BEGIN

namespace legacy
{
    namespace no_shutter_command
    {

        class LaserStatePersistor
        {
        public:

            LaserStatePersistor( LaserDevice* laserDevice ) :
                laserDevice_( laserDevice )
            {}

            void PersistRunMode( const std::string& runMode )
            {
                std::string current;
                Fetch( NULL, &current );

                char valueToSave[ 32 ];
                sprintf( valueToSave, "%s:%s", runMode.c_str(), current.c_str() );
                const std::string saveCommand = "sdsn " + std::string( valueToSave );

                laserDevice_->SendCommand( saveCommand );
            }

            void PersistCurrent( const std::string& current )
            {
                std::string runMode;
                Fetch( &runMode, NULL );

                char valueToSave[ 32 ];
                sprintf( valueToSave, "%s:%s", runMode.c_str(), current );
                const std::string saveCommand = "sdsn " + std::string( valueToSave );

                laserDevice_->SendCommand( saveCommand );
            }

        private:

            void Fetch( std::string* state, std::string* current )
            {
                std::string savedValue;
                laserDevice_->SendCommand( "gdsn?", &savedValue );

                bool separatorFound = false;
                for ( int i = 0; i < savedValue.length(); i++ ) {

                    if ( savedValue[ i ] == ':' ) {
                        separatorFound = true;
                        continue;
                    }

                    if ( !separatorFound && state != NULL ) {
                        state->append( 1, savedValue[ i ] );
                    } else if ( current != NULL ) {
                        current->append( 1, savedValue[ i ] );
                    }
                }
            }

            LaserDevice* laserDevice_;
        };

        class LaserCurrentProperty : public NumericProperty<double>
        {
            typedef NumericProperty<double> Parent;

        public:

            LaserCurrentProperty( const std::string& name, LaserDevice* laserDevice, const std::string& getCommand,
                const std::string& setCommandBase, const double min, const double max ) :
                NumericProperty<double>( name, laserDevice, getCommand, setCommandBase, min, max ),
                laserStatePersistor_( laserDevice )
            {
                laserDevice_->SendCommand( "gdsn?" );
            }

            virtual int SetValue( const std::string& value )
            {
                int returnCode = Parent::SetValue( value );

                if ( returnCode == return_code::ok ) {
                    laserStatePersistor_.PersistCurrent( value );
                }

                return returnCode;
            }

        private:

            LaserStatePersistor laserStatePersistor_;
        };

        class LaserRunModeProperty : public EnumerationProperty
        {
        public:

            LaserRunModeProperty( const std::string& name, LaserDevice* laserDevice, const std::string& getCommand ) :
                EnumerationProperty( name, laserDevice, getCommand ),
                laserStatePersistor_( laserDevice )
            {}

            virtual int SetValue( const std::string& value )
            {
                int returnCode = Parent::SetValue( value );

                if ( returnCode == return_code::ok ) {
                    laserStatePersistor_.PersistRunMode( value );
                }

                return returnCode;
            }

        private:

            LaserStatePersistor laserStatePersistor_;
        };

        class LaserShutterProperty : public MutableDeviceProperty
        {
        public:

            static const std::string Value_Open;
            static const std::string Value_Closed;

            LaserShutterProperty( const std::string& name, LaserDevice* laserDevice );
            virtual ~LaserShutterProperty();

            virtual int IntroduceToGuiEnvironment( GuiEnvironment* environment );

            virtual int GetValue( std::string& string ) const;
            virtual int SetValue( const std::string& value );

        private:

            struct LaserState
            {
                std::string runMode;
                std::string currentSetpoint;
            };

            bool IsOpen() const;

            LaserState* savedLaserState_;
        };

    }
}

NAMESPACE_COBOLT_END

#endif // #ifndef __COBOLT__LEGACY_LASER_SHUTTER_PROPERTY_H
