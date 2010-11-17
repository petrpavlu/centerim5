if (DOXYGEN_FOUND AND DOXYGEN_DOT_FOUND)

	# we only make us this work, if doxygen is really found
	set( top_srcdir ${centerim_SOURCE_DIR})

	configure_file( doxygen-cppconsui.conf.in	doxygen-cppconsui.conf	@ONLY)

	unset( top_srcdir)

	add_custom_target( cppconsui_documentation
				COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/doxygen-cppconsui.conf
			)

endif (DOXYGEN_FOUND AND DOXYGEN_DOT_FOUND)
