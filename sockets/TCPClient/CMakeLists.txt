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

add_library(TCPClient_shared SHARED ${TARGET_SOURCE})
set_target_properties(TCPClient_shared PROPERTIES OUTPUT_NAME "TcpClient")
set_target_properties(TCPClient_shared PROPERTIES CLEAN_DIRECT_OUTPUT 1)

add_library(TCPClient_static STATIC ${TARGET_SOURCE})
set_target_properties(TCPClient_static PROPERTIES OUTPUT_NAME "TcpClient")
set_target_properties(TCPClient_static PROPERTIES CLEAN_DIRECT_OUTPUT 1)

target_link_libraries(TCPClient_shared ${CMAKE_THREAD_LIBS_INIT})
install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/libTcpClient.so DESTINATION ${PROJECT_SOURCE_DIR}/libs)

target_link_libraries(TCPClient_static ${CMAKE_THREAD_LIBS_INIT})
install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/libTcpClient.a DESTINATION ${PROJECT_SOURCE_DIR}/libs)

