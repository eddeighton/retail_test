
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
using CString       = const char*;
using CStringVector = std::vector< const char* >;

struct CStringEquality
{
    CString str;
    CStringEquality( CString str )
        : str( str )
    {
    }
    inline bool operator()( CString cmp ) const { return 0 == strcmp( cmp, str ); }
};

CStringVector::const_iterator find_str( const CStringVector& list, CString str )
{
    return std::find_if( list.begin(), list.end(), CStringEquality{ str } );
}

CStringVector::iterator find_str( CStringVector& list, CString str )
{
    return std::find_if( list.begin(), list.end(), CStringEquality{ str } );
}

bool contains( const CStringVector& list, CString str ) { return !( find_str( list, str ) == list.end() ); }

void insert_once( CStringVector& list, CString str )
{
    if ( !contains( list, str ) )
    {
        list.push_back( str );
    }
}

bool contains( const CStringVector& outer, const CStringVector& inner )
{
    for ( CString str : inner )
    {
        if ( !contains( outer, str ) )
            return false;
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

    CStringVector available_instance_extensions;
    {
        uint32_t instance_extension_count = 0U;

        VK_CHECK( vk::enumerateInstanceExtensionProperties( nullptr, &instance_extension_count, nullptr ) );
        std::vector< vk::ExtensionProperties > availableVkExtensions( instance_extension_count );
        VK_CHECK( vk::enumerateInstanceExtensionProperties(
            nullptr, &instance_extension_count, availableVkExtensions.data() ) );
        for ( const vk::ExtensionProperties& extensionProperty : availableVkExtensions )
        {
            SPDLOG_TRACE( "Extension: {} Version: {}", extensionProperty.extensionName, extensionProperty.specVersion );
            available_instance_extensions.push_back( extensionProperty.extensionName );
        }
    }

    CStringVector required_instance_extensions = m_mainWindow.getRequiredSDLVulkanExtensions();
    {
        insert_once( required_instance_extensions, VK_KHR_SURFACE_EXTENSION_NAME );
        insert_once( required_instance_extensions, VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME );
        // if debug...
        insert_once( required_instance_extensions, VK_EXT_DEBUG_UTILS_EXTENSION_NAME );

        VERIFY_RTE_MSG( contains( available_instance_extensions, required_instance_extensions ),
                        "Required extensions not available" );
    }

    // if debug...
    CStringVector supportedValidationLayers;
    {
        std::uint32_t ui32_layerCount = 0U;

        // for now get all...
        static const std::array< const char*, 1U > requiredInstanceLayerProperties = { "VK_LAYER_KHRONOS_validation" };
        // const auto requiredInstanceLayerProperties = vk::enumerateInstanceLayerProperties();

        const auto availableValidationLayers = vk::enumerateInstanceLayerProperties();
        SPDLOG_TRACE( "Got {} instance layer properties", availableValidationLayers.size() );

        for ( const auto& requiredInstanceLayerProperties : requiredInstanceLayerProperties )
        {
            auto iFind = std::find_if(
                availableValidationLayers.cbegin(), availableValidationLayers.cend(),
                [ &requiredInstanceLayerProperties ]( const auto& instanceLayerProperties ) -> bool
                { return CStringEquality{ instanceLayerProperties.layerName }( requiredInstanceLayerProperties ); } );

            if ( iFind != availableValidationLayers.end() )
            {
                SPDLOG_INFO( "Found validation layer {} in available layers", requiredInstanceLayerProperties );
                supportedValidationLayers.push_back( requiredInstanceLayerProperties );
            }
            else
            {
                SPDLOG_INFO(
                    "Failed to find validation layer {} in available layers", requiredInstanceLayerProperties );
            }
        }
    }

    // initialise the instance
    {
        vk::ApplicationInfo app( "Vulkan Demo", {}, "Eds Vulkan Prototype", VK_MAKE_VERSION( 1, 0, 0 ) );

        vk::InstanceCreateInfo instance_info( {}, &app, supportedValidationLayers, required_instance_extensions );

        m_instance = vk::createInstanceUnique( instance_info );

        // initialise the dispatcher to get function pointers for instance
        VULKAN_HPP_DEFAULT_DISPATCHER.init( m_instance.get() );
    }

    // the surface
    m_surface = m_mainWindow.createVulkanSurface( m_instance.get() );
    VERIFY_RTE_MSG( m_surface, "Failed to initialise surface" );

    // select m_graphics_queue_index
    {
        std::vector< vk::PhysicalDevice > gpus = m_instance->enumeratePhysicalDevices();

        for ( const vk::PhysicalDevice& gpu : gpus )
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

                    for ( uint32_t j = 0; j < to_u32( queue_family_properties.size() ); j++ )
                    {
                        const vk::QueueFamilyProperties& prop = queue_family_properties[ j ];

                        if ( const vk::Bool32 supports_present = gpu.getSurfaceSupportKHR( j, m_surface ) )
                        {
                            // Find a queue family which supports graphics and presentation.
                            if ( prop.queueFlags & vk::QueueFlagBits::eGraphics )
                            {
                                m_physical_device      = gpu;
                                m_graphics_queue_index = j;
                                SPDLOG_TRACE( "Found GPU supporting graphics and presentation" );
                                break;
                            }
                        }
                    }
                    if ( m_graphics_queue_index != -1 )
                        break;
                }
            }
        }
    }

    VERIFY_RTE_MSG( m_graphics_queue_index != -1, "Failed to initialise physical device" );

    uint32_t windowWidth = 0U, windowHeight = 0U;

    vk::Extent2D swapchainExtent;
    {
        const vk::SurfaceCapabilitiesKHR surfaceCapabilities = m_physical_device.getSurfaceCapabilitiesKHR( m_surface );
        const vk::Extent2D               windowExtent        = m_mainWindow.getDrawableSize();
        swapchainExtent
            = vk::Extent2D{ std::min( std::max( windowExtent.width, surfaceCapabilities.minImageExtent.width ),
                                      surfaceCapabilities.maxImageExtent.width ),
                            std::min( std::max( windowExtent.height, surfaceCapabilities.minImageExtent.height ),
                                      surfaceCapabilities.maxImageExtent.height ) };
    }

    SPDLOG_INFO( "Created Vulkan Window with width: {} and height: {}", windowWidth, windowHeight );

    CStringVector available_device_extensions;
    {
        for ( const vk::ExtensionProperties& extensionProperty :
              m_physical_device.enumerateDeviceExtensionProperties() )
        {
            SPDLOG_TRACE(
                "Device Extension: {} Version: {}", extensionProperty.extensionName, extensionProperty.specVersion );
            available_device_extensions.push_back( extensionProperty.extensionName );
        }
    }

    CStringVector required_device_extensions;
    {
        insert_once( required_device_extensions, VK_KHR_SWAPCHAIN_EXTENSION_NAME );
    }

    VERIFY_RTE_MSG( contains( available_device_extensions, required_device_extensions ),
                    "Required device extensions not available" );

    float queue_priority = 1.0f;

    // Create one queue
    vk::DeviceQueueCreateInfo queue_info( {}, m_graphics_queue_index, 1, &queue_priority );

    vk::DeviceCreateInfo device_info( {}, queue_info, {}, required_device_extensions );

    m_logical_device = m_physical_device.createDevice( device_info );

    // initialize function pointers for device
    VULKAN_HPP_DEFAULT_DISPATCHER.init( m_logical_device );

    m_queue = m_logical_device.getQueue( m_graphics_queue_index, 0 );

    // initialise the swap chain - refactor from here for resizing window
    vk::SurfaceCapabilitiesKHR surface_properties = m_physical_device.getSurfaceCapabilitiesKHR( m_surface );

    std::vector< vk::SurfaceFormatKHR > formats = m_physical_device.getSurfaceFormatsKHR( m_surface );

    m_pDebugCallback = std::move( std::make_unique< DebugCallback >( m_instance.get() ) );

    vk::SurfaceCapabilitiesKHR          surfaceCapabilities = m_physical_device.getSurfaceCapabilitiesKHR( m_surface );
    std::vector< vk::SurfaceFormatKHR > surfaceFormats      = m_physical_device.getSurfaceFormatsKHR( m_surface );
    std::vector< vk::PresentModeKHR >   presentModes        = m_physical_device.getSurfacePresentModesKHR( m_surface );

    VERIFY_RTE( !surfaceFormats.empty() );
    VERIFY_RTE( !presentModes.empty() );

    std::optional< vk::SurfaceFormatKHR > idealFormatOpt;

    {
        for ( const auto& format : surfaceFormats )
        {
            if ( format.format == vk::Format::eB8G8R8Srgb && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear )
            {
                idealFormatOpt = format;
            }
        }
    }

    //swapchainExtent

}

Demo::~Demo()
{
    m_pDebugCallback.reset();

    if ( m_logical_device )
    {
        m_logical_device.destroy();
    }
    if ( m_surface )
    {
        m_instance->destroySurfaceKHR( m_surface );
    }
    // m_instance.destroy();
}

void Demo::frame() {}

} // namespace retail
