

#get boost
include( ${WORKSPACE_ROOT_PATH}/thirdparty/boost/boost_include.cmake )

#get spdlog
include( ${WORKSPACE_ROOT_PATH}/thirdparty/spdlog/spdlog_include.cmake )

#get json
include( ${WORKSPACE_ROOT_PATH}/thirdparty/nlohmann/json_include.cmake )

#get vulkan
include( ${WORKSPACE_ROOT_PATH}/thirdparty/khronosgroup/vulkan_include.cmake )

#get sdl
include( ${WORKSPACE_ROOT_PATH}/thirdparty/sdl/sdl_include.cmake )

#get common
include( ${WORKSPACE_ROOT_PATH}/src/common/common_include.cmake )

#get database
include( ${WORKSPACE_ROOT_PATH}/src/mega/mega_include.cmake )

set( RETAIL_SOURCE 
        demo.hpp
        demo.cpp
        application.hpp
        application.cpp
        window.hpp
        window.cpp
        main.cpp )

add_executable( retail_test ${RETAIL_SOURCE} )

target_compile_definitions( retail_test PUBLIC VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1)

link_spdlog( retail_test )
link_boost( retail_test program_options )
link_boost( retail_test filesystem )
link_boost( retail_test atomic )
link_sdl( retail_test )
link_json( retail_test )
link_common( retail_test )
link_database( retail_test )
link_vulkan( retail_test )

install( TARGETS retail_test DESTINATION bin)

MegaRetailProgram( retail_test )
