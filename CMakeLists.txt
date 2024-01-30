cmake_minimum_required(VERSION 3.22)

project(nets)

if(NOT DEFINED CMAKE_INSTALL_PREFIX)
    set(CMAKE_INSTALL_PREFIX "${PROJECT_BINARY_DIR}/install")
elseif(CMAKE_INSTALL_PREFIX STREQUAL "/usr/local")
    set(CMAKE_INSTALL_PREFIX "${PROJECT_BINARY_DIR}/install")
endif()

if(CMAKE_INCLUDE_PATH)
    include_directories("${CMAKE_INCLUDE_PATH};$ENV{INCLUDES}")
endif()
if(CMAKE_LIBRARY_PATH)
    link_directories("${CMAKE_LIBRARY_PATH};$ENV{LIBRARYS}")
endif()

#message(STATUS "PROJECT_ROOT_DIR:${PROJECT_SOURCE_DIR}/include")

add_subdirectory(sockets)
add_subdirectory(externals)
add_subdirectory(protocols)

