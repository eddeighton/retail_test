
#include "application.hpp"
#include "window.hpp"

#include "common/assert_verify.hpp"

#include "spdlog/spdlog.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_config.h>

namespace retail
{

Application::Application()
    :   m_bContinue( true ),
        m_mainWindow( Window::Config{} )
{
    const int iInitResult = SDL_Init( SDL_INIT_VIDEO ); // Initialize SDL2
    if ( iInitResult )
    {
        // uh oh!!
        THROW_RTE( "Failed to initialise SDL. Error: " << SDL_GetError() );
    }
    
    SPDLOG_INFO( "SDL Initialisation successful" );

    //SDL_SetHint( SDL_HINT_RENDER_OPENGL_SHADERS, "0" );
    SDL_SetHint( SDL_HINT_FRAMEBUFFER_ACCELERATION, "vulkan" );
    SDL_SetHint( SDL_HINT_RENDER_SCALE_QUALITY, "1" );

    atexit( SDL_Quit );
}

Application::~Application()
{

}

void Application::run()
{
    while( m_bContinue )
    {
        frame();

        SDL_Event ev;
        while( SDL_PollEvent( &ev ) )
        {
            onSDLEvent( ev );
        }
    }
}

void Application::onSDLEvent( const SDL_Event& ev )
{
    switch( ev.type )
    {
        case SDL_FIRSTEVENT:     //< Unused (do not remove) 
            break;  

        // Application events 
        case SDL_QUIT: //< User-requested quit 
            m_bContinue = false;
            break;  

        // These application events have special meaning on iOS, see README-ios.txt for details 
        case SDL_APP_TERMINATING:        //< The application is being terminated by the OS
                                            //   Called on iOS in applicationWillTerminate()
                                            //   Called on Android in onDestroy()

        case SDL_APP_LOWMEMORY:         //< The application is low on memory, free memory if possible.
                                        //    Called on iOS in applicationDidReceiveMemoryWarning()
                                        //    Called on Android in onLowMemory()
        case SDL_APP_WILLENTERBACKGROUND: //< The application is about to enter the background
                                            // Called on iOS in applicationWillResignActive()
                                            // Called on Android in onPause()
        case SDL_APP_DIDENTERBACKGROUND: //< The application did enter the background and may not get CPU for some time
                                            //  Called on iOS in applicationDidEnterBackground()
                                            //  Called on Android in onPause()
        case SDL_APP_WILLENTERFOREGROUND: //< The application is about to enter the foreground
                                            //Called on iOS in applicationWillEnterForeground()
                                            //Called on Android in onResume()
        case SDL_APP_DIDENTERFOREGROUND: //< The application is now interactive
                                            //Called on iOS in applicationDidBecomeActive()
                                            //Called on Android in onResume()
            break;
            
        // Window events 
        case SDL_WINDOWEVENT:   //< Window event data 
        {
            //if( Window* pWindow = findWindow( ev.window.windowID ) )
            {
                switch ( ev.window.event ) 
                {
                    case SDL_WINDOWEVENT_SHOWN:
                        //SDL_Log("Window %d shown", event->window.windowID);
                        break;
                    case SDL_WINDOWEVENT_HIDDEN:
                        //SDL_Log("Window %d hidden", event->window.windowID);
                        break;
                    case SDL_WINDOWEVENT_EXPOSED:
                        //SDL_Log("Window %d exposed", event->window.windowID);
                        break;
                    case SDL_WINDOWEVENT_MOVED:
                        //SDL_Log("Window %d moved to %d,%d",event->window.windowID, event->window.data1,event->window.data2);
                        break;
                    case SDL_WINDOWEVENT_RESIZED:
                        //pWindow->on_resized( ev.window.data1, ev.window.data2 );
                        break;
                    case SDL_WINDOWEVENT_MINIMIZED:
                        //SDL_Log("Window %d minimized", event->window.windowID);
                        break;
                    case SDL_WINDOWEVENT_MAXIMIZED:
                        //SDL_Log("Window %d maximized", event->window.windowID);
                        break;
                    case SDL_WINDOWEVENT_RESTORED:
                        //SDL_Log("Window %d restored", event->window.windowID);
                        break;
                    case SDL_WINDOWEVENT_ENTER:
                        //SDL_Log("Mouse entered window %d", event->window.windowID);
                        break;
                    case SDL_WINDOWEVENT_LEAVE:
                        //SDL_Log("Mouse left window %d", event->window.windowID);
                        break;
                    case SDL_WINDOWEVENT_FOCUS_GAINED:
                        //SDL_Log("Window %d gained keyboard focus",event->window.windowID);
                        break;
                    case SDL_WINDOWEVENT_FOCUS_LOST:
                        //SDL_Log("Window %d lost keyboard focus", event->window.windowID);
                        break;
                    case SDL_WINDOWEVENT_CLOSE:
                        //SDL_Log("Window %d closed", event->window.windowID);
                        m_bContinue = false;
                        break;
                    default:
                        //SDL_Log("Window %d got unknown event %d",event->window.windowID, event->window.event);
                        break;
                }
            }
        }
            break;
        case SDL_SYSWMEVENT:
            //m_applicationEvents.push_back( ev );
            break;
        case SDL_KEYDOWN: //< Key pressed
        case SDL_KEYUP:   //< Key released
            //if( Window* pWindow = findWindow( ev.key.windowID ) )
            {
                //pWindow->on_event( ev );
            }
            break;  //< Keyboard event data 
                
        case SDL_TEXTEDITING:            //< Keyboard text editing (composition) 
            {

            }
            break;  
        case SDL_TEXTINPUT:            //< Keyboard text input 
            {

            }
            break;  
                
        // Mouse events 
        case SDL_MOUSEMOTION: // Mouse moved 
            //if( Window* pWindow = findWindow( ev.motion.windowID ) )
            //{
            //    pWindow->on_event( ev );
            //}
            break; 
        case SDL_MOUSEBUTTONDOWN:        //< Mouse button pressed 
        case SDL_MOUSEBUTTONUP:          //< Mouse button released 
            //if( Window* pWindow = findWindow( ev.button.windowID ) )
            //{
            //    pWindow->on_event( ev );
            //}
            break;  
        case SDL_MOUSEWHEEL:            //< Mouse wheel motion
            //if( Window* pWindow = findWindow( ev.wheel.windowID ) )
            //{
            //    pWindow->on_event( ev );
            //} 
            break;  
                
        // Joystick events 
        case SDL_JOYAXISMOTION: //< Joystick axis motion 
            //m_applicationEvents.push_back( ev );
            break;  
        case SDL_JOYBALLMOTION:          //< Joystick trackball motion 
            //m_applicationEvents.push_back( ev );
            break;  
        case SDL_JOYHATMOTION:           //< Joystick hat position change 
            //m_applicationEvents.push_back( ev );
            break;  
        case SDL_JOYBUTTONDOWN:          //< Joystick button pressed 
        case SDL_JOYBUTTONUP:            //< Joystick button released 
            //m_applicationEvents.push_back( ev );
            break; 
        case SDL_JOYDEVICEADDED:        //< A new joystick has been inserted into the system 
        case SDL_JOYDEVICEREMOVED:       //< An opened joystick has been removed 
            //m_applicationEvents.push_back( ev );
            break;  
                
        // Game controller events 
        case SDL_CONTROLLERAXISMOTION: //< Game controller axis motion 
            //m_applicationEvents.push_back( ev );
            break;  
        case SDL_CONTROLLERBUTTONDOWN:          //< Game controller button pressed 
        case SDL_CONTROLLERBUTTONUP:            //< Game controller button released 
            //m_applicationEvents.push_back( ev );
            break; 
        case SDL_CONTROLLERDEVICEADDED:         //< A new Game controller has been inserted into the system 
        case SDL_CONTROLLERDEVICEREMOVED:      //< An opened Game controller has been removed 
        case SDL_CONTROLLERDEVICEREMAPPED:      //< The controller mapping was updated 
            //m_applicationEvents.push_back( ev );
            break;  
                
        // Touch events 
        case SDL_FINGERDOWN:
        case SDL_FINGERUP:
        case SDL_FINGERMOTION:
            //m_applicationEvents.push_back( ev );
            break;  
                
        // Gesture events 
        case SDL_DOLLARGESTURE:
            //m_applicationEvents.push_back( ev );
            break; 
        case SDL_DOLLARRECORD:
            //m_applicationEvents.push_back( ev );
            break; 
        case SDL_MULTIGESTURE:
            //m_applicationEvents.push_back( ev );
            break;  
                
        // Clipboard events 
        case SDL_CLIPBOARDUPDATE: //< The clipboard changed 
            //m_applicationEvents.push_back( ev );
            break;  
                
        case SDL_DROPFILE: //< The system requests a file open 
            //m_applicationEvents.push_back( ev );
            break;  
                
        // Events ::SDL_USEREVENT through ::SDL_LASTEVENT are for your use,
            // and should be allocated with SDL_RegisterEvents()
        case SDL_USEREVENT:
            {

            }
            break;
    }
}
}
