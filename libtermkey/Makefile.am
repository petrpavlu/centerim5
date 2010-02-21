lib_LTLIBRARIES = libtermkey.la

libtermkey_la_SOURCES =\
	driver-csi.c \
	driver-ti.c \
	termkey-internal.h \
	termkey.c \
	termkey.h

# TODO remove -std=c99 from here
libtermkey_la_CPPFLAGS = -std=c99 $(CFLAGS) $(CURSES_INCLUDEDIR) $(DEBUG_CFLAGS)
libtermkey_la_LIBADD = $(CURSES_LIBS)
