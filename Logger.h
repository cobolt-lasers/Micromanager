/**
 * \file        Logger.h
 *
 * \authors     Lukas Kalinski
 *
 * \copyright   Cobolt AB, 2020. All rights reserved. // TODO: Set proper license
 */

#ifndef __COBOLT__LOGGER
#define __COBOLT__LOGGER

NAMESPACE_COBOLT_BEGIN

class Logger
{
public:

    class Gateway
    {
    public:

        virtual void SendLogMessage( const char* message, bool debug ) const = 0;
    };

    static Logger* Instance()
    {
        static Logger instance;
        return &instance;
    }

    void SetupWithGateway( const Gateway* gateway )
    {
        gateway_ = gateway;
    }
    
    virtual void LogMessage( const std::string& message, bool debug ) const
    {
        if ( gateway_ == NULL ) {
            return;
        }
        
        gateway_->SendLogMessage( ("" + message).c_str(), debug );
    }

    virtual void LogError( const std::string& message ) const
    {
        if ( gateway_ == NULL ) {
            return;
        }

        gateway_->SendLogMessage( ( "ERROR: " + message ).c_str(), false );
    }

private:

    Logger() {}

    const Gateway* gateway_;
};

NAMESPACE_COBOLT_END

#endif // #ifndef __COBOLT__LOGGER