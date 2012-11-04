# extaction plugin
if (GLIB232_FOUND)
  # when you add files here, also add them in po/POTFILES.in
  set(extaction_SOURCES
    extaction.c)

  add_library(extaction SHARED
    ${extaction_SOURCES})

  target_link_libraries(extaction
    ${PURPLE_LIBRARIES}
    ${GLIB232_LIBRARIES})

  install(TARGETS extaction DESTINATION lib/centerim5)
endif (GLIB232_FOUND)
