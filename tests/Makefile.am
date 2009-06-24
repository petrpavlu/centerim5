bin_PROGRAMS = textline_test

textline_test_SOURCES = \
	textline_test.cpp

textline_test_CPPFLAGS =\
	$(GLIBMM_CFLAGS) \
	$(CURSES_INCLUDEDIR) \
	-I$(top_srcdir)

textline_test_LDADD =\
	$(GLIBMM_LIBS) \
	$(CURSES_LIBS) \
	$(top_builddir)/cppconsui/libcppconsui.la
