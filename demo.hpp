
#ifndef DEMO_25_APRIL_2022
#define DEMO_25_APRIL_2022

#include "application.hpp"
#include "debug.hpp"

#include <vulkan/vulkan.hpp>

#include <optional>

namespace retail
{

class Demo : public Application
{
public:
    Demo();
    ~Demo();

    virtual void frame();

private:
    vk::DynamicLoader            m_dynamic_loader;
    vk::UniqueInstance           m_instance;
    vk::SurfaceKHR               m_surface;
    vk::PhysicalDevice           m_physical_device;
    vk::Device                   m_logical_device;
    vk::Queue                    m_queue;
    vk::SwapchainKHR             m_swapchain;
    std::vector< vk::Image >     m_swapChainImages;
    std::vector< vk::ImageView > m_swapChainImageViews;

    std::optional< uint32_t >        m_graphics_queue_index;
    std::unique_ptr< DebugCallback > m_pDebugCallback;
    std::set< std::string >          m_required_instance_extensions;
    std::set< std::string >          m_supportedValidationLayers;
};

} // namespace retail

#endif // DEMO_25_APRIL_2022
