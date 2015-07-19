# extaction plugin
if (GLIB232_FOUND)
  # When you add files here, also add them in po/POTFILES.in.
  set(extaction_SOURCES
    extaction.c)

  add_library(extaction SHARED
    ${extaction_SOURCES})

  set_property(TARGET extaction PROPERTY
    PREFIX "")
  target_compile_options(extaction PRIVATE
    ${PURPLE_CFLAGS} ${GLIB232_CFLAGS})
  target_link_libraries(extaction PRIVATE
    ${PURPLE_LDFLAGS} ${GLIB232_LDFLAGS})

  install(TARGETS extaction DESTINATION lib/centerim5)
endif (GLIB232_FOUND)
