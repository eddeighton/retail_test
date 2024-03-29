#ifndef WINDOW_20_APRIL_2022
#define WINDOW_20_APRIL_2022

#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_handles.hpp>

#include <SDL2/SDL.h>

#include <memory>
#include <string>
#include <functional>
#include <set>

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

        std::set< std::string > getRequiredSDLVulkanExtensions() const;

        vk::SurfaceKHR createVulkanSurface( VkInstance instance ) const;

        vk::Extent2D getDrawableSize() const;

        void onResize();

    private:
        std::unique_ptr< SDL_Window, decltype( std::bind( &SDL_DestroyWindow, std::placeholders::_1 ) ) > m_pWnd;
    };
} // namespace retail

#endif // WINDOW_20_APRIL_2022
