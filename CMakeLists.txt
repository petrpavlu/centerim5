project( centerim)

cmake_minimum_required( VERSION 2.8)

set( CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")

include( cmake/CMakeConfs.cmake)

add_subdirectory( po/)
add_subdirectory( cppconsui/)
add_subdirectory( doc/)
add_subdirectory( src/)
add_subdirectory( tests/)
