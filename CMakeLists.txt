project( centerim)

cmake_minimum_required( VERSION 2.8)

include( cmake/CMakeConfs.cmake)

add_subdirectory( po/)
add_subdirectory( libtermkey/)
add_subdirectory( cppconsui/)
add_subdirectory( doc/)
add_subdirectory( src/)
add_subdirectory( tests/)
