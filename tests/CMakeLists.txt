add_executable( button_test EXCLUDE_FROM_ALL button_test.cpp )

target_link_libraries( button_test
				${GLIB2_LIBRARIES}
				${SIGC_LIBRARIES}
				cppconsui
		)

######################################################################
add_executable( label_test EXCLUDE_FROM_ALL label_test.cpp )

target_link_libraries( label_test
				${GLIB2_LIBRARIES}
				${SIGC_LIBRARIES}
				cppconsui
		)

######################################################################
add_executable( linestyle_test EXCLUDE_FROM_ALL linestyle_test.cpp)

target_link_libraries( linestyle_test
				${GLIB2_LIBRARIES}
				${SIGC_LIBRARIES}
				cppconsui
		)

######################################################################
add_executable ( textentry_test EXCLUDE_FROM_ALL textentry_test.cpp)

target_link_libraries( textentry_test
				${GLIB2_LIBRARIES}
				${SIGC_LIBRARIES}
				cppconsui
		)

######################################################################
add_executable ( textview_test EXCLUDE_FROM_ALL textview_test.cpp)

target_link_libraries( textview_test
				${GLIB2_LIBRARIES}
				${SIGC_LIBRARIES}
				cppconsui
		)

######################################################################
add_executable ( treeview_test EXCLUDE_FROM_ALL treeview_test.cpp)

target_link_libraries( treeview_test
				${GLIB2_LIBRARIES}
				${SIGC_LIBRARIES}
				cppconsui
		)

######################################################################
add_executable ( scrollpane_test EXCLUDE_FROM_ALL scrollpane_test.cpp)

target_link_libraries( scrollpane_test
				${GLIB2_LIBRARIES}
				${SIGC_LIBRARIES}
				cppconsui
		)

######################################################################
add_executable( submenu_test EXCLUDE_FROM_ALL submenu_test.cpp )

target_link_libraries( submenu_test
				${GLIB2_LIBRARIES}
				${SIGC_LIBRARIES}
				cppconsui
		)

######################################################################
add_executable ( window_test EXCLUDE_FROM_ALL window_test.cpp)

target_link_libraries( window_test
				${GLIB2_LIBRARIES}
				${SIGC_LIBRARIES}
				cppconsui
		)
