add_executable(button EXCLUDE_FROM_ALL button.cpp)

target_link_libraries(button
  cppconsui
  ${GLIB2_LIBRARIES}
  ${SIGC_LIBRARIES})

##############################################################################
add_executable(colorpicker EXCLUDE_FROM_ALL colorpicker.cpp) 

target_link_libraries(colorpicker
  cppconsui
  ${GLIB2_LIBRARIES}
  ${SIGC_LIBRARIES})

##############################################################################
add_executable(label EXCLUDE_FROM_ALL label.cpp)

target_link_libraries(label
  cppconsui
  ${GLIB2_LIBRARIES}
  ${SIGC_LIBRARIES})

##############################################################################
add_executable(scrollpane EXCLUDE_FROM_ALL scrollpane.cpp)

target_link_libraries(scrollpane
  cppconsui
  ${GLIB2_LIBRARIES}
  ${SIGC_LIBRARIES})

##############################################################################
add_executable(submenu EXCLUDE_FROM_ALL submenu.cpp)

target_link_libraries(submenu
  cppconsui
  ${GLIB2_LIBRARIES}
  ${SIGC_LIBRARIES})

##############################################################################
add_executable(textentry EXCLUDE_FROM_ALL textentry.cpp)

target_link_libraries(textentry
  cppconsui
  ${GLIB2_LIBRARIES}
  ${SIGC_LIBRARIES})

##############################################################################
add_executable(textview EXCLUDE_FROM_ALL textview.cpp)

target_link_libraries(textview
  cppconsui
  ${GLIB2_LIBRARIES}
  ${SIGC_LIBRARIES})

##############################################################################
add_executable(treeview EXCLUDE_FROM_ALL treeview.cpp)

target_link_libraries(treeview
  cppconsui
  ${GLIB2_LIBRARIES}
  ${SIGC_LIBRARIES})

##############################################################################
add_executable(window EXCLUDE_FROM_ALL window.cpp)

target_link_libraries(window
  cppconsui
  ${GLIB2_LIBRARIES}
  ${SIGC_LIBRARIES})
