check_PROGRAMS = \
	button_test \
	label_test \
	linestyle_test \
	textentry_test \
	textview_test \
	treeview_test \
	scrollpane_test \
	window_test

AM_CPPFLAGS = \
	$(GLIB_CFLAGS) \
	$(SIGC_CFLAGS) \
	-I$(top_srcdir)

LDADD = \
	$(GLIB_LIBS) \
	$(SIGC_LIBS) \
	$(top_builddir)/cppconsui/libcppconsui.la

button_test_SOURCES = \
	button_test.cpp

label_test_SOURCES = \
	label_test.cpp

linestyle_test_SOURCES = \
	linestyle_test.cpp

textentry_test_SOURCES = \
	textentry_test.cpp

textview_test_SOURCES = \
	textview_test.cpp

treeview_test_SOURCES = \
	treeview_test.cpp

scrollpane_test_SOURCES = \
	scrollpane_test.cpp

window_test_SOURCES = \
	window_test.cpp
