
#include "window.hpp"

#include "common/assert_verify.hpp"

namespace retail
{

    Window::Window( const Window::Config& config )
        : m_pWnd( SDL_CreateWindow( config.strName.c_str(), config.left, config.top, config.width, config.height, config.flags ),
                  std::bind( &SDL_DestroyWindow, std::placeholders::_1 ) )
    {
        VERIFY_RTE( m_pWnd );

    }

    void Window::onResize()
    {

    }

} // namespace retail
