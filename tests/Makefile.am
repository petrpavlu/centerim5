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
	button.cpp \
	MainLoop.cpp

colorpicker_SOURCES = \
	colorpicker.cpp \
	MainLoop.cpp

label_SOURCES = \
	label.cpp \
	MainLoop.cpp

scrollpane_SOURCES = \
	scrollpane.cpp \
	MainLoop.cpp

submenu_SOURCES = \
	submenu.cpp \
	MainLoop.cpp

textentry_SOURCES = \
	textentry.cpp \
	MainLoop.cpp

textview_SOURCES = \
	textview.cpp \
	MainLoop.cpp

treeview_SOURCES = \
	treeview.cpp \
	MainLoop.cpp

window_SOURCES = \
	window.cpp \
	MainLoop.cpp
