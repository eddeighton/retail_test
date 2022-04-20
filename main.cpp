


#include "application.hpp"

#include "common/assert_verify.hpp"

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
        
        SDL_SetHint( SDL_HINT_RENDER_OPENGL_SHADERS, "0" );
        SDL_SetHint( SDL_HINT_FRAMEBUFFER_ACCELERATION, "vulkan" );
        SDL_SetHint( SDL_HINT_RENDER_SCALE_QUALITY, "1" );
        
        //int major, minor;
        //VERIFY_RTE( SDL_GL_GetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, &major ) == 0 );
        //VERIFY_RTE( SDL_GL_GetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, &minor ) == 0 );
        
        //SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
        //SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
        //SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);

        retail::Application application;

        application.run();
    
        atexit( SDL_Quit );
    }
    return 0;
}
