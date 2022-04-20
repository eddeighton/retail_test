

#get boost
include( ${WORKSPACE_ROOT_PATH}/thirdparty/boost/boost_include.cmake )

#get spdlog
include( ${WORKSPACE_ROOT_PATH}/thirdparty/spdlog/spdlog_include.cmake )

#get sdl
include( ${WORKSPACE_ROOT_PATH}/thirdparty/sdl/sdl_include.cmake )

#get common
include( ${WORKSPACE_ROOT_PATH}/src/common/common_include.cmake )

set( RETAIL_SOURCE 
        application.hpp
        application.cpp
        window.hpp
        window.cpp
        main.cpp )

add_executable( retail_test ${RETAIL_SOURCE} )

link_spdlog( retail_test )
link_boost( retail_test program_options )
link_boost( retail_test filesystem )
link_boost( retail_test atomic )
link_sdl( retail_test )
link_common( retail_test )

install( TARGETS retail_test DESTINATION bin)
