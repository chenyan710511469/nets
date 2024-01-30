cmake_minimum_required(VERSION 3.22)
set(CMAKE_VERBOSE_MAKEFILE on)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(NETS_TCP_SERVER ${CMAKE_CURRENT_SOURCE_DIR})
file(GLOB_RECURSE NETS_TCP_SERVER_SRC ${NETS_TCP_SERVER}/*.cpp)

include_directories(${PROJECT_SOURCE_DIR}/include)
#include_directories(/home/chenyan/projects/nets/include)

set(TARGET_SOURCE
    ${NETS_TCP_SERVER_SRC}
    ${TARGET_SOURCE}
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -Wall -D_GLIBCXX_USE_NANOSLEEP -DLINUX ")

link_directories(${PROJECT_SOURCE_DIR}/lib64)

add_library(ProtocolService_shared SHARED ${TARGET_SOURCE})
set_target_properties(ProtocolService_shared PROPERTIES OUTPUT_NAME "ProtocolService")
set_target_properties(ProtocolService_shared PROPERTIES CLEAN_DIRECT_OUTPUT 1)

add_library(ProtocolService_static STATIC ${TARGET_SOURCE})
set_target_properties(ProtocolService_static PROPERTIES OUTPUT_NAME "ProtocolService")
set_target_properties(ProtocolService_static PROPERTIES CLEAN_DIRECT_OUTPUT 1)

add_dependencies(ProtocolService_shared TCPServer_shared TCPClient_shared UDPServer_shared UDPClient_shared)
target_link_libraries(ProtocolService_shared PUBLIC ${CMAKE_THREAD_LIBS_INIT} TCPServer_shared TCPClient_shared UDPServer_shared UDPClient_shared tinyxml2  json_linux-gcc-7_libmt)

install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/libProtocolService.so DESTINATION ${PROJECT_SOURCE_DIR}/libs)

add_dependencies(ProtocolService_static TCPServer_static TCPClient_static UDPServer_static UDPClient_static)
target_link_libraries(ProtocolService_static PUBLIC ${CMAKE_THREAD_LIBS_INIT} TCPServer_static TCPClient_static UDPServer_static UDPClient_static tinyxml2  json_linux-gcc-7_libmt)
install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/libProtocolService.a DESTINATION ${PROJECT_SOURCE_DIR}/libs)

