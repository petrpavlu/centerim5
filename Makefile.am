ACLOCAL_AMFLAGS = -I m4

SUBDIRS = libtermkey cppconsui doc misc po src tests

EXTRA_DIST =\
	bootstrap \
	HACKING \
	gettext.h

BUILT_SOURCES = $(top_srcdir)/.version
# we want to execute this target everytime make is run
.PHONY: $(top_srcdir)/.version
$(top_srcdir)/.version:
	@cd $(top_srcdir); version=`misc/git-version-gen .tarball-version`; \
	if test $$version != "`cat .version 2> /dev/null`"; then printf '%s' $$version > .version; fi

dist-hook:
	@cd $(top_srcdir); printf '%s' `misc/git-version-gen .tarball-version` > $(distdir)/.tarball-version

.PHONY: doc
doc:
	cd doc; $(MAKE) doc
