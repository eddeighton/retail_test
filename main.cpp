

#include "demo.hpp"

#include "spdlog/spdlog.h"

int main( int argc, const char* argv[] )
{
    try
    {
        retail::Demo demo;
        demo.run();
    }
    catch ( std::exception& ex )
    {
        SPDLOG_ERROR( "Exception: {}", ex.what() );
        return 1;
    }
    catch ( ... )
    {
        SPDLOG_ERROR( "Unknown exception" );
        return 1;
    }
    return 0;
}
