check_PROGRAMS = \
	textline_test \
	textbuffer_test \
	label_test \
	window_test \
	linestyle_test

AM_CPPFLAGS = \
	$(DEBUG_CFLAGS) \
	$(GLIBMM_CFLAGS) \
	$(CURSES_INCLUDEDIR) \
	-I$(top_srcdir)

LDADD = \
	$(GLIBMM_LIBS) \
	$(CURSES_LIBS) \
	$(top_builddir)/cppconsui/libcppconsui.la

textline_test_SOURCES = \
	textline_test.cpp

textbuffer_test_SOURCES = \
	textbuffer_test.cpp

label_test_SOURCES = \
	label_test.cpp

window_test_SOURCES = \
	window_test.cpp

linestyle_test_SOURCES = \
	linestyle_test.cpp
