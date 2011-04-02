ACLOCAL_AMFLAGS = -I m4

SUBDIRS = libtermkey cppconsui doc misc po src tests

EXTRA_DIST =\
	bootstrap \
	HACKING \
	gettext.h

BUILT_SOURCES = $(top_srcdir)/.version
$(top_srcdir)/.version:
	echo $(VERSION) > $@-t && mv $@-t $@
dist-hook:
	echo $(VERSION) > $(distdir)/.tarball-version

.PHONY: doc
doc:
	cd doc; $(MAKE) doc
