
#include "window.hpp"

#include "SDL_vulkan.h"

#include "common/assert_verify.hpp"

namespace retail
{

Window::Window( const Window::Config& config )
    : m_pWnd(
        SDL_CreateWindow( config.strName.c_str(), config.left, config.top, config.width, config.height, config.flags ),
        std::bind( &SDL_DestroyWindow, std::placeholders::_1 ) )
{
    VERIFY_RTE( m_pWnd );
}

std::set< std::string > Window::getRequiredSDLVulkanExtensions() const
{
    unsigned int count = 0U;
    if ( !SDL_Vulkan_GetInstanceExtensions( m_pWnd.get(), &count, nullptr ) )
    {
        throw std::runtime_error( "Failed to get sdl vulkan extension requirements count" );
    }

    std::vector< const char* > requiredExtensions( count );
    if ( !SDL_Vulkan_GetInstanceExtensions( m_pWnd.get(), &count, requiredExtensions.data() ) )
    {
        throw std::runtime_error( "Failed to get sdl vulkan extension requirements" );
    }
    return std::set< std::string >{ requiredExtensions.begin(), requiredExtensions.end() };
}

vk::SurfaceKHR Window::createVulkanSurface( VkInstance instance ) const
{
    VkSurfaceKHR surface = nullptr;

    if ( !SDL_Vulkan_CreateSurface( m_pWnd.get(), instance, &surface ) )
    {
        throw std::runtime_error( "Failed to get sdl vulkan surface" );
    }

    return vk::SurfaceKHR( surface );
}

vk::Extent2D Window::getDrawableSize() const
{
    int iWidth = 0, iHeight = 0;
    SDL_Vulkan_GetDrawableSize( m_pWnd.get(), &iWidth, &iHeight );
    VERIFY_RTE( iWidth >= 0 && iHeight >= 0 );
    return vk::Extent2D{ static_cast< uint32_t >( iWidth ), static_cast< uint32_t >( iHeight ) };
}

void Window::onResize() {}

} // namespace retail
