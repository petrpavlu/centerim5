if(DOXYGEN_FOUND AND DOXYGEN_DOT_FOUND)
  set(top_srcdir "${centerim5_SOURCE_DIR}")
  configure_file(doxygen-cppconsui.conf.in doxygen-cppconsui.conf @ONLY)
  unset(top_srcdir)

  add_custom_target(doc VERBATIM
    COMMAND "${DOXYGEN_EXECUTABLE}" doxygen-cppconsui.conf)

else()
  add_custom_target(doc)

  add_custom_command(TARGET doc POST_BUILD VERBATIM
    COMMAND "${CMAKE_COMMAND}" -E echo "Documentation cannot be generated \
because required dependencies are missing, reconfigure the build and install \
the tools that are reported as needed for generating the documentation"
    COMMAND false)
endif()
