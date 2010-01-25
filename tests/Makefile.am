check_PROGRAMS = \
	textline_test \
	textbuffer_test \
	label_test

CPPFLAGS = \
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
textline_test_CPPFLAGS = $(CPPFLAGS)
textline_test_LDADD = $(LDADD)

textbuffer_test_SOURCES = \
	textbuffer_test.cpp
textbuffer_test_CPPFLAGS = $(CPPFLAGS)
textbuffer_test_LDADD = $(LDADD)
