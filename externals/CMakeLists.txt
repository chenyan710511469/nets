cmake_minimum_required(VERSION 3.22)


#add_subdirectory(jsoncpp-src-0.5.0)
add_subdirectory(tinyxml2)

install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/tinyxml2/libtinyxml2.so.7.0.1 DESTINATION ${PROJECT_SOURCE_DIR}/libs)


