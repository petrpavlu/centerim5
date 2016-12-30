project(centerim5)

cmake_minimum_required(VERSION 2.8)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(cmake/CMakeConfs.cmake)

# libtermkey requires c99
set(CMAKE_C_FLAGS "-std=c99")

# Newer versions of sigc++ require this (this mirrors what's done in configure.ac)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11")

install(FILES centerim5.1 DESTINATION share/man/man1)

add_subdirectory(cppconsui)
add_subdirectory(src)
add_subdirectory(tests)
add_subdirectory(plugins)
add_subdirectory(po)
add_subdirectory(doc)
