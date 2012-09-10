if (DOXYGEN_FOUND AND DOXYGEN_DOT_FOUND)
  # we only make this work if doxygen is really found
  set(top_srcdir ${centerim5_SOURCE_DIR})
  configure_file(doxygen-cppconsui.conf.in doxygen-cppconsui.conf @ONLY)
  unset(top_srcdir)

  add_custom_target(doc
    COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/doxygen-cppconsui.conf)
endif (DOXYGEN_FOUND AND DOXYGEN_DOT_FOUND)
