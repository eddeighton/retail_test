

#include "application.hpp"

#include "database/common/environments.hpp"
#include "database/model/FinalStage.hxx"

#include "common/assert_verify.hpp"

#include "nlohmann/json.hpp"

#include <SDL2/SDL_config.h>
#include <SDL2/SDL.h>

#include "spdlog/spdlog.h"

#include <boost/filesystem/path.hpp>

#include <vector>

#include "vulkan/vulkan.h"

// see /workspace/root/thirdparty/khronosgroup/Vulkan-Samples/framework/common/strings.h
// << vkb::to_string( err )
#define VK_CHECK( x )                                  \
    do                                                 \
    {                                                  \
        VkResult err = x;                              \
        if ( err )                                     \
        {                                              \
            SPDLOG_ERROR( "Detected Vulkan error: " ); \
            abort();                                   \
        }                                              \
    } while ( 0 )

int main( int argc, const char* argv[] )
{
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

    uint32_t instance_extension_count = 0U;
    VK_CHECK( vkEnumerateInstanceExtensionProperties( nullptr, &instance_extension_count, nullptr ) );

    std::vector< VkExtensionProperties > available_instance_extensions( instance_extension_count );
    VK_CHECK( vkEnumerateInstanceExtensionProperties( nullptr, &instance_extension_count, available_instance_extensions.data() ) );

    for ( const VkExtensionProperties& extensionProperty : available_instance_extensions )
    {
        SPDLOG_INFO( "Extension: {} Version: {}", extensionProperty.extensionName, extensionProperty.specVersion );
    }

    // vk::Instance* pInstance = nullptr;
    //  vk::InstanceCreateInfo createInfo;
    //  vk::AllocationCallbacks allocatorCallbacks;
    //  vk::Result r = vk::createInstance( &createInfo, &allocatorCallbacks, pInstance );

    try
    {
        const int iInitResult = SDL_Init( SDL_INIT_VIDEO ); // Initialize SDL2
        if ( iInitResult )
        {
            // uh oh!!
            THROW_RTE( "Failed to initialise SDL. Error: " << SDL_GetError() );
        }
        else
        {
            SPDLOG_INFO( "SDL Initialisation successful" );

            SDL_SetHint( SDL_HINT_RENDER_OPENGL_SHADERS, "0" );
            SDL_SetHint( SDL_HINT_FRAMEBUFFER_ACCELERATION, "vulkan" );
            SDL_SetHint( SDL_HINT_RENDER_SCALE_QUALITY, "1" );

            // int major, minor;
            // VERIFY_RTE( SDL_GL_GetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, &major ) == 0 );
            // VERIFY_RTE( SDL_GL_GetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, &minor ) == 0 );

            // SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
            // SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
            // SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);

            retail::Application application;

            application.run();

            atexit( SDL_Quit );
        }
    }
    catch ( std::exception& ex )
    {
        SPDLOG_ERROR( "Exception: {}", ex.what() );
        return 1;
    }
    catch ( ... )
    {
        SPDLOG_ERROR( "Unknown exception" );
        return 1;
    }
    return 0;
}
