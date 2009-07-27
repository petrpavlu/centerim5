bin_PROGRAMS = textline_test textbuffer_test

textline_test_SOURCES = \
	textline_test.cpp

textline_test_CPPFLAGS =\
	$(DEBUG_CFLAGS) \
	$(GLIBMM_CFLAGS) \
	$(CURSES_INCLUDEDIR) \
	-I$(top_srcdir)

textline_test_LDADD =\
	$(GLIBMM_LIBS) \
	$(CURSES_LIBS) \
	$(top_builddir)/cppconsui/libcppconsui.la

textbuffer_test_SOURCES = \
	textbuffer_test.cpp

textbuffer_test_CPPFLAGS =\
	$(DEBUG_CFLAGS) \
	$(GLIBMM_CFLAGS) \
	$(CURSES_INCLUDEDIR) \
	-I$(top_srcdir)

textbuffer_test_LDADD =\
	$(GLIBMM_LIBS) \
	$(CURSES_LIBS) \
	$(top_builddir)/cppconsui/libcppconsui.la
