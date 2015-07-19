# Find ncursesw using ncursesw5-config. Set <CMAKE_FIND_PACKAGE_NAME>_FOUND,
# <CMAKE_FIND_PACKAGE_NAME>_CFLAGS and <CMAKE_FIND_PACKAGE_NAME>_LDFLAGS in
# the same way pkg_check_modules() does.

if(${CMAKE_FIND_PACKAGE_NAME}_CFLAGS OR ${CMAKE_FIND_PACKAGE_NAME}_LDFLAGS)
  # Use externally set variables.
  set(${CMAKE_FIND_PACKAGE_NAME}_LIBRARY true)
else()
  # Try to use ncursesw5-config to obtain correct flags.
  find_program(NCURSESW5_CONFIG_EXECUTABLE ncursesw5-config)
  if(NCURSESW5_CONFIG_EXECUTABLE)
    execute_process(COMMAND ${NCURSESW5_CONFIG_EXECUTABLE} --cflags
      OUTPUT_VARIABLE ${CMAKE_FIND_PACKAGE_NAME}_CFLAGS
      OUTPUT_STRIP_TRAILING_WHITESPACE)
    execute_process(COMMAND ${NCURSESW5_CONFIG_EXECUTABLE} --libs
      OUTPUT_VARIABLE ${CMAKE_FIND_PACKAGE_NAME}_LDFLAGS
      OUTPUT_STRIP_TRAILING_WHITESPACE)
    set(${CMAKE_FIND_PACKAGE_NAME}_LIBRARY true)
  endif()
endif()

# Handle correctly standard package arguments.
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(${CMAKE_FIND_PACKAGE_NAME}
  REQUIRED_VARS ${CMAKE_FIND_PACKAGE_NAME}_LIBRARY)

mark_as_advanced(NCURSESW5_CONFIG_EXECUTABLE)
