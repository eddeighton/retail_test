#ifndef DEBUG_CALLBACK_7_JUNE_2022
#define DEBUG_CALLBACK_7_JUNE_2022

#include <vulkan/vulkan.hpp>

namespace retail
{

// see /workspace/root/thirdparty/khronosgroup/Vulkan-Samples/framework/common/strings.h
// << vkb::to_string( err )
#define VK_CHECK( x )                                                   \
    do                                                                  \
    {                                                                   \
        const vk::Result err{ x };                                      \
        if ( err != vk::Result::eSuccess )                              \
        {                                                               \
            SPDLOG_ERROR( "Error: {} : {}", #x, vk::to_string( err ) ); \
            abort();                                                    \
        }                                                               \
    } while ( 0 )

class DebugCallback
{
public:
    DebugCallback( vk::Instance& instance );
    ~DebugCallback();

private:
    vk::Instance&              m_instance;
    vk::DebugUtilsMessengerEXT m_debugMessenger;
};

} // namespace retail

#endif // DEBUG_CALLBACK_7_JUNE_2022
