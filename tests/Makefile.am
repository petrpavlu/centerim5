check_PROGRAMS = \
	button \
	colorpicker \
	keybinding_config \
	label \
	textentry \
	textview \
	treeview \
	scrollpane \
	submenu \
	window

AM_CPPFLAGS = \
	$(GLIB_CFLAGS) \
	$(SIGC_CFLAGS) \
	-I$(top_srcdir)

LDADD = \
	$(GLIB_LIBS) \
	$(SIGC_LIBS) \
	$(top_builddir)/cppconsui/libcppconsui.la

button_SOURCES = \
	button.cpp

colorpicker_SOURCES = \
	colorpicker.cpp

keybinding_config_SOURCES = \
	keybinding_config.cpp

label_SOURCES = \
	label.cpp

textentry_SOURCES = \
	textentry.cpp

textview_SOURCES = \
	textview.cpp

treeview_SOURCES = \
	treeview.cpp

scrollpane_SOURCES = \
	scrollpane.cpp

submenu_SOURCES = \
	submenu.cpp

window_SOURCES = \
	window.cpp
