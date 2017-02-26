set(check_DEPENDS "")
if(TERMEX_TESTS)
  add_custom_command(OUTPUT terminfo VERBATIM
    COMMAND "${TIC_EXECUTABLE}" -o terminfo
      "${CMAKE_CURRENT_SOURCE_DIR}/termex.src"
    COMMAND "${CMAKE_COMMAND}" -E touch terminfo)
  list(APPEND check_DEPENDS terminfo)
endif()

add_custom_target(check DEPENDS ${check_DEPENDS})

if(NOT TERMEX_TESTS)
  add_custom_command(TARGET check POST_BUILD VERBATIM
    COMMAND "${CMAKE_COMMAND}" -E echo "Tests cannot be run because required \
dependencies are missing, reconfigure the build and install the tools that are \
reported as needed for the testing"
    COMMAND false)
endif()

add_compile_options(${SIGC_CFLAGS})
link_libraries(${SIGC_LDFLAGS} cppconsui)

add_executable(button button.cpp main.cpp)
add_dependencies(check button)
add_executable(colorpicker colorpicker.cpp main.cpp)
add_dependencies(check colorpicker)
add_executable(label label.cpp main.cpp)
add_dependencies(check label)
add_executable(submenu submenu.cpp main.cpp)
add_dependencies(check submenu)
add_executable(textentry textentry.cpp main.cpp)
add_dependencies(check textentry)
add_executable(textview textview.cpp main.cpp)
add_dependencies(check textview)
add_executable(treeview treeview.cpp main.cpp)
add_dependencies(check treeview)
add_executable(window window.cpp main.cpp)
add_dependencies(check window)
