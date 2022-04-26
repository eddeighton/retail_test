#ifndef APPLICATION_20_APRIL_2022
#define APPLICATION_20_APRIL_2022

#include "SDL2/SDL_events.h"

#include "window.hpp"

namespace retail
{
    class Application
    {
    public:
        Application();
        ~Application();

        virtual void frame() = 0;

        void run();

    private:
        void onSDLEvent( const SDL_Event& ev );

        bool m_bContinue;
    protected:
        Window m_mainWindow;
    };

}

#endif //APPLICATION_20_APRIL_2022
