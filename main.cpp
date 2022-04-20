


#include <SDL2/SDL_config.h>
#include <SDL2/SDL.h>

#include "spdlog/spdlog.h"

#include <sstream>

int main( int argc, const char* argv[] )
{
    const int iInitResult = SDL_Init( SDL_INIT_VIDEO );              // Initialize SDL2
    if( iInitResult )
    {
        //uh oh!!
        std::ostringstream os;
        os << "Failed to initialise SDL. Error: " << SDL_GetError();
        throw std::runtime_error( os.str() );
    }
    else
    {
        SPDLOG_INFO( "SDL Initialisation successful" );
        
    
        atexit( SDL_Quit );
    }
    return 0;
}
