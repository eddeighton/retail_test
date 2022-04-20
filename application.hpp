#ifndef APPLICATION_20_APRIL_2022
#define APPLICATION_20_APRIL_2022

#include "SDL2/SDL_events.h"

namespace retail
{
    class Application
    {
    public:
        Application();

        void run();

    private:
        void onSDLEvent( const SDL_Event& ev );

        bool m_bContinue;
    };

}

#endif //APPLICATION_20_APRIL_2022
