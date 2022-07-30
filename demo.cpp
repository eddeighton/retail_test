
#include "demo.hpp"

#include "common/assert_verify.hpp"

#include "debug.hpp"
#include "spdlog/spdlog.h"

#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_handles.hpp>
#include <vulkan/vulkan_structs.hpp>

#include <vector>
#include <algorithm>
#include <string>
#include <memory>
#include <optional>

// https://github.com/KhronosGroup/Vulkan-Hpp
VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

namespace
{
template < class T >
uint32_t to_u32( T value )
{
    static_assert( std::is_arithmetic< T >::value, "T must be numeric" );

    if ( static_cast< uintmax_t >( value ) > static_cast< uintmax_t >( std::numeric_limits< uint32_t >::max() ) )
    {
        throw std::runtime_error( "to_u32() failed, value is too big to be converted to uint32_t" );
    }

    return static_cast< uint32_t >( value );
}
} // namespace

namespace retail
{

bool contains( std::vector< vk::ExtensionProperties > const& extensionProperties,
               const std::set< std::string >&                required )
{
    for ( const std::string& extensionName : required )
    {
        auto iFind = std::find_if( extensionProperties.cbegin(),
                                   extensionProperties.cend(),
                                   [ &extensionName ]( vk::ExtensionProperties const& ep )
                                   { return extensionName == ep.extensionName; } );
        if ( iFind == extensionProperties.end() )
        {
            return false;
        }
    }
    return true;
}

Demo::Demo()
{
    // initialise the vulkan-hpp DispatchLoaderDynamic
    {
        PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr
            = m_dynamic_loader.getProcAddress< PFN_vkGetInstanceProcAddr >( "vkGetInstanceProcAddr" );
        VULKAN_HPP_DEFAULT_DISPATCHER.init( vkGetInstanceProcAddr );
    }

    const auto available_instance_extensions = vk::enumerateInstanceExtensionProperties();

    std::vector< const char* > required_instance_extensions;
    {
        m_required_instance_extensions = m_mainWindow.getRequiredSDLVulkanExtensions();
        m_required_instance_extensions.insert( VK_KHR_SURFACE_EXTENSION_NAME );
        m_required_instance_extensions.insert( VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME );
        m_required_instance_extensions.insert( VK_EXT_DEBUG_UTILS_EXTENSION_NAME );

        VERIFY_RTE_MSG( contains( available_instance_extensions, m_required_instance_extensions ),
                        "Required extensions not available" );
        for ( const std::string& str : m_required_instance_extensions )
            required_instance_extensions.push_back( str.c_str() );
    }

    std::vector< const char* > supportedValidationLayers;
    {
        const std::vector< vk::LayerProperties > availableValidationLayers = vk::enumerateInstanceLayerProperties();
        SPDLOG_TRACE( "Got {} instance layer properties", availableValidationLayers.size() );

        const std::vector< std::string > requiredInstanceLayerProperties = { "VK_LAYER_KHRONOS_validation" };

        for ( const std::string& requiredInstanceLayerProperties : requiredInstanceLayerProperties )
        {
            auto iFind = std::find_if(
                availableValidationLayers.cbegin(), availableValidationLayers.cend(),
                [ &requiredInstanceLayerProperties ]( const vk::LayerProperties& instanceLayerProperties ) -> bool
                { return requiredInstanceLayerProperties == instanceLayerProperties.layerName; } );

            if ( iFind != availableValidationLayers.end() )
            {
                SPDLOG_INFO( "Found validation layer {} in available layers", requiredInstanceLayerProperties );
                m_supportedValidationLayers.insert( requiredInstanceLayerProperties );
            }
            else
            {
                SPDLOG_INFO(
                    "Failed to find validation layer {} in available layers", requiredInstanceLayerProperties );
            }
        }
        for ( const std::string& str : m_supportedValidationLayers )
            supportedValidationLayers.push_back( str.c_str() );
    }

    // initialise the instance
    {
        vk::ApplicationInfo    app( "Vulkan Demo", {}, "Eds Vulkan Prototype", VK_MAKE_VERSION( 1, 0, 0 ) );
        vk::InstanceCreateInfo instance_info( {}, &app, supportedValidationLayers, required_instance_extensions );
        m_instance = vk::createInstanceUnique( instance_info );
        // initialise the dispatcher to get function pointers for instance
        VULKAN_HPP_DEFAULT_DISPATCHER.init( m_instance.get() );
        m_pDebugCallback = std::move( std::make_unique< DebugCallback >( m_instance.get() ) );
    }

    // the surface
    m_surface = m_mainWindow.createVulkanSurface( m_instance.get() );
    VERIFY_RTE_MSG( m_surface, "Failed to initialise surface" );

    // select m_graphics_queue_index
    {
        for ( const vk::PhysicalDevice& gpu : m_instance->enumeratePhysicalDevices() )
        {
            if ( gpu.getProperties().deviceType == vk::PhysicalDeviceType::eDiscreteGpu )
            {
                if ( gpu.getFeatures().geometryShader )
                {
                    std::vector< vk::QueueFamilyProperties > queue_family_properties = gpu.getQueueFamilyProperties();
                    if ( queue_family_properties.empty() )
                    {
                        // throw std::runtime_error("No queue family found.");
                        continue;
                    }

                    const std::uint32_t uiTotalQueueFamilies = to_u32( queue_family_properties.size() );
                    for ( uint32_t uiQueueIndex = 0; uiQueueIndex < uiTotalQueueFamilies; uiQueueIndex++ )
                    {
                        if ( const vk::Bool32 supports_present = gpu.getSurfaceSupportKHR( uiQueueIndex, m_surface ) )
                        {
                            // Find a queue family which supports graphics and presentation.
                            const vk::QueueFamilyProperties& prop = queue_family_properties[ uiQueueIndex ];
                            if ( prop.queueFlags & vk::QueueFlagBits::eGraphics )
                            {
                                m_physical_device      = gpu;
                                m_graphics_queue_index = uiQueueIndex;
                                SPDLOG_TRACE( "Found GPU supporting graphics and presentation" );
                                break;
                            }
                        }
                    }
                    if ( m_graphics_queue_index.has_value() )
                        break;
                }
            }
        }
        VERIFY_RTE_MSG( m_graphics_queue_index.has_value(), "Failed to find graphics device with required queue" );
    }

    std::vector< const char* > available_device_extensions;
    {
        for ( const vk::ExtensionProperties& extensionProperty :
              m_physical_device.enumerateDeviceExtensionProperties() )
        {
            SPDLOG_TRACE(
                "Device Extension: {} Version: {}", extensionProperty.extensionName, extensionProperty.specVersion );
            available_device_extensions.push_back( extensionProperty.extensionName );
        }
    }

    std::vector< const char* > required_device_extensions;
    {
        required_device_extensions.push_back( VK_KHR_SWAPCHAIN_EXTENSION_NAME );
    }

    for ( const char* pRequired : required_device_extensions )
    {
        auto iFind
            = std::find_if( available_device_extensions.cbegin(),
                            available_device_extensions.cend(),
                            [ pRequired ]( const auto& available ) { return strcmp( pRequired, available ) == 0; } );
        if ( iFind == available_device_extensions.end() )
        {
            THROW_RTE( "Failed to locate required device extension: " << pRequired );
        }
    }

    float queue_priority = 1.0f;

    // Create one queue
    vk::DeviceQueueCreateInfo queue_info( {}, m_graphics_queue_index.value(), 1, &queue_priority );

    vk::DeviceCreateInfo device_info( {}, queue_info, {}, required_device_extensions );

    m_logical_device = m_physical_device.createDevice( device_info );

    // initialize function pointers for device
    VULKAN_HPP_DEFAULT_DISPATCHER.init( m_logical_device );

    m_queue = m_logical_device.getQueue( m_graphics_queue_index.value(), 0 );

    // initialise the swap chain - refactor from here for resizing window
    vk::SurfaceCapabilitiesKHR surface_properties = m_physical_device.getSurfaceCapabilitiesKHR( m_surface );

    std::vector< vk::SurfaceFormatKHR > formats = m_physical_device.getSurfaceFormatsKHR( m_surface );

    const vk::SurfaceCapabilitiesKHR surfaceCapabilities     = m_physical_device.getSurfaceCapabilitiesKHR( m_surface );
    const std::vector< vk::SurfaceFormatKHR > surfaceFormats = m_physical_device.getSurfaceFormatsKHR( m_surface );
    const std::vector< vk::PresentModeKHR >   presentModes   = m_physical_device.getSurfacePresentModesKHR( m_surface );

    VERIFY_RTE( !surfaceFormats.empty() );
    VERIFY_RTE( !presentModes.empty() );

    std::optional< vk::SurfaceFormatKHR > idealFormatOpt;
    {
        for ( const auto& format : surfaceFormats )
        {
            if ( !idealFormatOpt.has_value() )
                idealFormatOpt = format;
            if ( format.format == vk::Format::eB8G8R8Srgb && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear )
            {
                idealFormatOpt = format;
                break;
            }
        }
        VERIFY_RTE_MSG( idealFormatOpt.has_value(), "Failed to find ideal format" );
    }

    vk::Extent2D swapchainExtent;
    {
        uint32_t           windowWidth = 0U, windowHeight = 0U;
        const vk::Extent2D windowExtent = m_mainWindow.getDrawableSize();
        swapchainExtent
            = vk::Extent2D{ std::min( std::max( windowExtent.width, surfaceCapabilities.minImageExtent.width ),
                                      surfaceCapabilities.maxImageExtent.width ),
                            std::min( std::max( windowExtent.height, surfaceCapabilities.minImageExtent.height ),
                                      surfaceCapabilities.maxImageExtent.height ) };
        SPDLOG_INFO( "Created Vulkan Window with width: {} and height: {}", windowWidth, windowHeight );
    }

    std::optional< vk::PresentModeKHR > bestPresentationMode;
    {
        for ( const vk::PresentModeKHR& presentationMode : presentModes )
        {
            switch ( presentationMode )
            {
                case vk::PresentModeKHR::eImmediate: //= VK_PRESENT_MODE_IMMEDIATE_KHR,
                    break;
                case vk::PresentModeKHR::eMailbox: //= VK_PRESENT_MODE_MAILBOX_KHR,
                    break;
                case vk::PresentModeKHR::eFifo:                       //= VK_PRESENT_MODE_FIFO_KHR,
                    bestPresentationMode = vk::PresentModeKHR::eFifo; // guarenteed to be available
                    break;
                case vk::PresentModeKHR::eFifoRelaxed: //= VK_PRESENT_MODE_FIFO_RELAXED_KHR,
                    break;
                case vk::PresentModeKHR::eSharedDemandRefresh: //= VK_PRESENT_MODE_SHARED_DEMAND_REFRESH_KHR,
                    break;
                case vk::PresentModeKHR::eSharedContinuousRefresh: //= VK_PRESENT_MODE_SHARED_CONTINUOUS_REFRESH_KHR
                    break;
                default:
                    break;
            }
        }
        VERIFY_RTE_MSG( bestPresentationMode.has_value(), "Failed to find presentation mode" );
    }

    {
        std::array< std::uint32_t, 1 > queues{ m_graphics_queue_index.value() };
        vk::SwapchainCreateInfoKHR     swapchainCreateInfo{
            vk::SwapchainCreateFlagsKHR{},
            m_surface,
            std::min( surfaceCapabilities.minImageCount + 1, surfaceCapabilities.maxImageCount ),
            idealFormatOpt.value().format,
            idealFormatOpt.value().colorSpace,
            swapchainExtent,
            1, // imageArrayLayers_
            vk::ImageUsageFlagBits::eColorAttachment,
            VULKAN_HPP_NAMESPACE::SharingMode::eExclusive,
            queues,
            vk::SurfaceTransformFlagBitsKHR::eIdentity,
            vk::CompositeAlphaFlagBitsKHR::eOpaque,
            bestPresentationMode.value(),
            true, // clipped_
            {}    // oldSwapchain_
        };

        m_swapchain       = m_logical_device.createSwapchainKHR( swapchainCreateInfo );
        m_swapChainImages = m_logical_device.getSwapchainImagesKHR( m_swapchain );
    }

    for ( const vk::Image& image : m_swapChainImages )
    {
        // clang-format off
        vk::ImageViewCreateInfo imageViewCreateInfo = 
        {
            vk::ImageViewCreateFlags{},
            image,
            vk::ImageViewType::e2D,
            idealFormatOpt.value().format,
            vk::ComponentMapping{},
            vk::ImageSubresourceRange
            {
                vk::ImageAspectFlagBits::eColor,
                0, //baseMipLevel
                1, //levelCount
                0, //baseArrayLayer_
                1  //layerCount_
            }
        };
        // clang-format on
        vk::ImageView imageView = m_logical_device.createImageView( imageViewCreateInfo );
        m_swapChainImageViews.push_back( imageView );
    }
}

Demo::~Demo()
{
    for ( vk::ImageView& imageView : m_swapChainImageViews )
    {
        m_logical_device.destroyImageView( imageView );
    }

    if ( m_logical_device )
    {
        m_logical_device.destroy();
    }
    if ( m_surface )
    {
        m_instance->destroySurfaceKHR( m_surface );
    }

    m_pDebugCallback.reset();
}

void Demo::frame() {}

} // namespace retail
