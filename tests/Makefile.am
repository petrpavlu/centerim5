check_PROGRAMS = \
	button \
	colorpicker \
	label \
	scrollpane \
	submenu \
	textentry \
	textview \
	treeview \
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

label_SOURCES = \
	label.cpp

scrollpane_SOURCES = \
	scrollpane.cpp

submenu_SOURCES = \
	submenu.cpp

textentry_SOURCES = \
	textentry.cpp

textview_SOURCES = \
	textview.cpp

treeview_SOURCES = \
	treeview.cpp

window_SOURCES = \
	window.cpp
