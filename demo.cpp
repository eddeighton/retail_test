
#include "demo.hpp"

#include "common/assert_verify.hpp"

#include "spdlog/spdlog.h"

#include <vulkan/vulkan_handles.hpp>
#include <vulkan/vulkan_structs.hpp>

#include <vector>
#include <algorithm>
#include <string>

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
//#include "database/common/environments.hpp"
//#include "database/model/FinalStage.hxx"
//#include "nlohmann/json.hpp"
/*
try
{
    mega::io::RetailEnvironment environment( boost::filesystem::current_path() / "archive.zip" );

    {
        using namespace FinalStage;

        Database database( environment, environment.project_manifest() );
        database.load();

        {
            boost::filesystem::path outputFilePath = boost::filesystem::current_path() / "database.json";
            nlohmann::json data;
            database.to_json( data );
            std::ofstream os( outputFilePath.native(), std::ios_base::trunc | std::ios_base::out );
            os << data;
        }

        std::cout << "Loaded database" << std::endl;

    }
}
catch( std::exception& ex )
{
    std::cout << "Exception: " << ex.what() << std::endl;
    return 1;
}*/

// see /workspace/root/thirdparty/khronosgroup/Vulkan-Samples/framework/common/strings.h
// << vkb::to_string( err )
#define VK_CHECK( x )                                                   \
    do                                                                  \
    {                                                                   \
        vk::Result err = x;                                             \
        if ( err != vk::Result::eSuccess )                              \
        {                                                               \
            SPDLOG_ERROR( "Error: {} : {}", #x, vk::to_string( err ) ); \
            abort();                                                    \
        }                                                               \
    } while ( 0 )

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
        PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr
            = m_dynamic_loader.getProcAddress< PFN_vkGetInstanceProcAddr >( "vkGetInstanceProcAddr" );
        VULKAN_HPP_DEFAULT_DISPATCHER.init( vkGetInstanceProcAddr );

        CStringVector available_instance_extensions;
        {
            uint32_t instance_extension_count = 0U;

            VK_CHECK( vk::enumerateInstanceExtensionProperties( nullptr, &instance_extension_count, nullptr ) );
            std::vector< vk::ExtensionProperties > availableVkExtensions( instance_extension_count );
            VK_CHECK( vk::enumerateInstanceExtensionProperties( nullptr, &instance_extension_count, availableVkExtensions.data() ) );
            for ( const vk::ExtensionProperties& extensionProperty : availableVkExtensions )
            {
                SPDLOG_INFO( "Extension: {} Version: {}", extensionProperty.extensionName, extensionProperty.specVersion );
                available_instance_extensions.push_back( extensionProperty.extensionName );
            }
        }

        CStringVector required_instance_extensions = m_mainWindow.getRequiredSDLVulkanExtensions();

        insert_once( required_instance_extensions, VK_KHR_SURFACE_EXTENSION_NAME );
        insert_once( required_instance_extensions, VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME );

        VERIFY_RTE_MSG( contains( available_instance_extensions, required_instance_extensions ), "Required extensions not available" );

        // std::vector<vk::LayerProperties> supported_validation_layers = vk::enumerateInstanceLayerProperties();

        vk::ApplicationInfo app( "Vulkan Demo", {}, "Eds VUlkan Prototype", VK_MAKE_VERSION( 1, 0, 0 ) );

        vk::InstanceCreateInfo instance_info( {}, &app, {}, required_instance_extensions );

        m_instance = vk::createInstance( instance_info );

        VULKAN_HPP_DEFAULT_DISPATCHER.init( m_instance );

        m_surface = m_mainWindow.createVulkanSurface( m_instance );
        VERIFY_RTE_MSG( m_surface, "Failed to initialise surface" );

        {
            std::vector< vk::PhysicalDevice > gpus = m_instance.enumeratePhysicalDevices();

            for ( const vk::PhysicalDevice& gpu : gpus )
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

                    vk::Bool32 supports_present = gpu.getSurfaceSupportKHR( j, m_surface );

                    // Find a queue family which supports graphics and presentation.
                    if ( ( prop.queueFlags & vk::QueueFlagBits::eGraphics ) && supports_present )
                    {
                        m_physical_device      = gpu;
                        m_graphics_queue_index = j;
                        SPDLOG_INFO( "Found GPU supporting graphics and presentation" );
                        break;
                    }
                }
                if ( m_graphics_queue_index != -1 )
                    break;
            }
        }

        VERIFY_RTE_MSG( m_graphics_queue_index != -1, "Failed to initialise physical device" );

        uint32_t windowWidth = 0U, windowHeight = 0U;
        {
            const vk::SurfaceCapabilitiesKHR surfaceCapabilities = m_physical_device.getSurfaceCapabilitiesKHR( m_surface );
            m_mainWindow.getDrawableSize( windowWidth, windowHeight );
            windowWidth
                = std::min( std::max( windowWidth, surfaceCapabilities.minImageExtent.width ), surfaceCapabilities.maxImageExtent.width );
            windowHeight = std::min(
                std::max( windowHeight, surfaceCapabilities.minImageExtent.height ), surfaceCapabilities.maxImageExtent.height );
        }

        SPDLOG_INFO( "Created Vulkan Window with width: {} and height: {}", windowWidth, windowHeight );

        CStringVector available_device_extensions;
        {
            for ( const vk::ExtensionProperties& extensionProperty : m_physical_device.enumerateDeviceExtensionProperties() )
            {
                SPDLOG_INFO( "Device Extension: {} Version: {}", extensionProperty.extensionName, extensionProperty.specVersion );
                available_device_extensions.push_back( extensionProperty.extensionName );
            }
        }

        CStringVector required_device_extensions;
        insert_once( required_device_extensions, VK_KHR_SWAPCHAIN_EXTENSION_NAME );

        VERIFY_RTE_MSG( contains( available_device_extensions, required_device_extensions ), "Required device extensions not available" );

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

        
    }

    Demo::~Demo()
    {
        if ( m_logical_device )
        {
            m_logical_device.destroy();
        }
        if ( m_surface )
        {
            m_instance.destroySurfaceKHR( m_surface );
        }
        m_instance.destroy();
    }

    void Demo::frame() {}

} // namespace retail