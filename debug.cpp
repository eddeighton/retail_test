
#include "debug.hpp"

#include "common/assert_verify.hpp"

#include "spdlog/spdlog.h"

namespace
{

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback( VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
                                                     VkDebugUtilsMessageTypeFlagsEXT             messageType,
                                                     const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                     void*                                       pUserData )
{
    // VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT = 0x00000001,
    // VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT = 0x00000002,
    // VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT = 0x00000004,

    if ( messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT )
    {
        SPDLOG_ERROR( "debugCallback: {}", pCallbackData->pMessage );
    }
    else if ( messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT )
    {
        SPDLOG_WARN( "debugCallback: {}", pCallbackData->pMessage );
    }
    else if ( messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT )
    {
        SPDLOG_INFO( "debugCallback: {}", pCallbackData->pMessage );
    }
    else if ( messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT )
    {
        SPDLOG_DEBUG( "debugCallback: {}", pCallbackData->pMessage );
    }
    else
    {
        SPDLOG_TRACE( "debugCallback: {}", pCallbackData->pMessage );
    }

    return VK_FALSE;
}

VkResult CreateDebugUtilsMessengerEXT( vk::Instance&                             instance,
                                       const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                       const VkAllocationCallbacks*              pAllocator,
                                       vk::DebugUtilsMessengerEXT&               debugMessenger )
{
    auto func
        = ( PFN_vkCreateDebugUtilsMessengerEXT )vkGetInstanceProcAddr( instance, "vkCreateDebugUtilsMessengerEXT" );
    if ( func != nullptr )
    {
        VkDebugUtilsMessengerEXT_T* pResult = nullptr;
        const auto                  result  = func( instance, pCreateInfo, pAllocator, &pResult );
        VK_CHECK( result );
        debugMessenger = pResult;
        return result;
    }
    else
    {
        THROW_RTE( "Failed to acquire PFN_vkCreateDebugUtilsMessengerEXT" );
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void DestroyDebugUtilsMessengerEXT( vk::Instance&                instance,
                                    vk::DebugUtilsMessengerEXT   debugMessenger,
                                    const VkAllocationCallbacks* pAllocator )
{
    auto func
        = ( PFN_vkDestroyDebugUtilsMessengerEXT )vkGetInstanceProcAddr( instance, "vkDestroyDebugUtilsMessengerEXT" );
    if ( func != nullptr )
    {
        func( instance, debugMessenger, pAllocator );
    }
}
} // namespace

namespace retail
{

DebugCallback::DebugCallback( vk::Instance& instance )
    : m_instance( instance )
{
    VkDebugUtilsMessengerCreateInfoEXT createInfo{};
    {
        createInfo.sType           = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
                                     | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
                                     | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
                                 | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
                                 | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback = debugCallback;
        createInfo.pUserData       = ( void* )this; // Optional
    }

    VK_CHECK( CreateDebugUtilsMessengerEXT( m_instance, &createInfo, nullptr, m_debugMessenger ) );
}

DebugCallback::~DebugCallback()
{
    //
    DestroyDebugUtilsMessengerEXT( m_instance, m_debugMessenger, nullptr );
}

} // namespace retail