pkglib_LTLIBRARIES = extaction.la

# When you add files here, also add them in po/POTFILES.in.
extaction_la_SOURCES = \
	extaction.c

extaction_la_CPPFLAGS = \
	$(PURPLE_CFLAGS) \
	$(GLIB_CFLAGS) \
	-I$(top_srcdir)

extaction_la_LDFLAGS = \
	-avoid-version -module

extaction_la_LIBADD = \
	$(LTLIBINTL) \
	$(PURPLE_LIBS) \
	$(GLIB_LIBS)
