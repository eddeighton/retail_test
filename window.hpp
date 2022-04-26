#ifndef WINDOW_20_APRIL_2022
#define WINDOW_20_APRIL_2022

#include <vulkan/vulkan.hpp>
#include <SDL2/SDL.h>

#include <memory>
#include <string>
#include <functional>

namespace retail
{
    class Window
    {
    public:
        struct Config
        {
            std::string  strName = "retail test";
            int          left = SDL_WINDOWPOS_CENTERED, top = SDL_WINDOWPOS_CENTERED, width = 512, height = 512;
            unsigned int flags = SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_VULKAN;
        };

        Window( const Config& config );

        std::vector< const char* > getRequiredSDLVulkanExtensions() const;

        VkSurfaceKHR createVulkanSurface( VkInstance instance ) const;

        void getDrawableSize( uint32_t& width, uint32_t& height ) const;

        void onResize();

    private:
        std::unique_ptr< SDL_Window, decltype( std::bind( &SDL_DestroyWindow, std::placeholders::_1 ) ) > m_pWnd;
    };
} // namespace retail

#endif // WINDOW_20_APRIL_2022
