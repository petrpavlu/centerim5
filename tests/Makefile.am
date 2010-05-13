check_PROGRAMS = \
	label_test \
	linestyle_test \
	textentry_test \
	textview_test \
	treeview_test \
	window_test
#textline_test
#textbuffer_test

AM_CPPFLAGS = \
	$(GLIB_CFLAGS) \
	$(SIGC_CFLAGS) \
	-I$(top_srcdir)

LDADD = \
	$(GLIB_LIBS) \
	$(SIGC_LIBS) \
	$(top_builddir)/cppconsui/libcppconsui.la

label_test_SOURCES = \
	label_test.cpp

linestyle_test_SOURCES = \
	linestyle_test.cpp

textbuffer_test_SOURCES = \
	textbuffer_test.cpp

textentry_test_SOURCES = \
	textentry_test.cpp

textline_test_SOURCES = \
	textline_test.cpp

textview_test_SOURCES = \
	textview_test.cpp

treeview_test_SOURCES = \
	treeview_test.cpp

window_test_SOURCES = \
	window_test.cpp
