add_executable( button EXCLUDE_FROM_ALL button.cpp )

target_link_libraries( button
				${GLIB2_LIBRARIES}
				${SIGC_LIBRARIES}
				cppconsui
		)

######################################################################
add_executable( colorpicker EXCLUDE_FROM_ALL colorpicker.cpp )

target_link_libraries( colorpicker
				${GLIB2_LIBRARIES}
				${SIGC_LIBRARIES}
				cppconsui
		)

######################################################################
add_executable( label EXCLUDE_FROM_ALL label.cpp )

target_link_libraries( label
				${GLIB2_LIBRARIES}
				${SIGC_LIBRARIES}
				cppconsui
		)

######################################################################
add_executable ( textentry EXCLUDE_FROM_ALL textentry.cpp)

target_link_libraries( textentry
				${GLIB2_LIBRARIES}
				${SIGC_LIBRARIES}
				cppconsui
		)

######################################################################
add_executable ( textview EXCLUDE_FROM_ALL textview.cpp)

target_link_libraries( textview
				${GLIB2_LIBRARIES}
				${SIGC_LIBRARIES}
				cppconsui
		)

######################################################################
add_executable ( treeview EXCLUDE_FROM_ALL treeview.cpp)

target_link_libraries( treeview
				${GLIB2_LIBRARIES}
				${SIGC_LIBRARIES}
				cppconsui
		)

######################################################################
add_executable ( scrollpane EXCLUDE_FROM_ALL scrollpane.cpp)

target_link_libraries( scrollpane
				${GLIB2_LIBRARIES}
				${SIGC_LIBRARIES}
				cppconsui
		)

######################################################################
add_executable( submenu EXCLUDE_FROM_ALL submenu.cpp )

target_link_libraries( submenu
				${GLIB2_LIBRARIES}
				${SIGC_LIBRARIES}
				cppconsui
		)

######################################################################
add_executable ( window EXCLUDE_FROM_ALL window.cpp)

target_link_libraries( window
				${GLIB2_LIBRARIES}
				${SIGC_LIBRARIES}
				cppconsui
		)
