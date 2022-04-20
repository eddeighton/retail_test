

#get boost
include( ${WORKSPACE_ROOT_PATH}/thirdparty/boost/boost_include.cmake )

#get spdlog
include( ${WORKSPACE_ROOT_PATH}/thirdparty/spdlog/spdlog_include.cmake )

#get sdl
include( ${WORKSPACE_ROOT_PATH}/thirdparty/sdl/sdl_include.cmake )

#get common
include( ${WORKSPACE_ROOT_PATH}/src/common/common_include.cmake )

add_executable( retail main.cpp )

link_spdlog( retail )
link_boost( retail atomic )
link_boost( retail program_options )
link_boost( retail timer )
link_boost( retail filesystem )
link_boost( retail iostreams )
link_boost( retail serialization )
link_sdl( retail )
link_common( retail )

install( TARGETS retail DESTINATION bin)
