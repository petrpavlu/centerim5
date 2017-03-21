include_directories(${Intl_INCLUDE_DIRS})
add_compile_options(${SIGC_CFLAGS})
link_libraries(${Intl_LIBRARIES} ${SIGC_LDFLAGS} cppconsui)

add_executable(button button.cpp main.cpp)
add_executable(colorpicker colorpicker.cpp main.cpp)
add_executable(label label.cpp main.cpp)
add_executable(submenu submenu.cpp main.cpp)
add_executable(textentry textentry.cpp main.cpp)
add_executable(textview textview.cpp main.cpp)
add_executable(treeview treeview.cpp main.cpp)
add_executable(window window.cpp main.cpp)

if(TERMEX_TESTS)
  add_custom_command(OUTPUT terminfo VERBATIM
    COMMAND "${TIC_EXECUTABLE}" -o terminfo
      "${CMAKE_CURRENT_SOURCE_DIR}/termex.src"
    COMMAND "${CMAKE_COMMAND}" -E touch terminfo
    DEPENDS termex.src)

  add_custom_target(check VERBATIM COMMAND "${CMAKE_CTEST_COMMAND}"
    DEPENDS terminfo)

else()
  add_custom_target(check)

  add_custom_command(TARGET check POST_BUILD VERBATIM
    COMMAND "${CMAKE_COMMAND}" -E echo "Tests cannot be run because required \
dependencies are missing, reconfigure the build and install the tools that are \
reported as needed for the testing"
    COMMAND false)
endif()

add_dependencies(check
  button
  colorpicker
  label
  submenu
  textentry
  textview
  treeview
  window)

# List of test playbooks.
set(TESTS
  treeview)

foreach(testname ${TESTS})
  add_test(NAME "${testname}"
    COMMAND "${PYTHON_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/termex.py"
      -t "${CMAKE_CURRENT_BINARY_DIR}/terminfo" test
      "${CMAKE_CURRENT_SOURCE_DIR}/${testname}.test")
endforeach()
