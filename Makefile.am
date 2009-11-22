SUBDIRS = po cppconsui tests src

EXTRA_DIST = HACKING

ACLOCAL_AMFLAGS = -I m4

.PHONY: doc
doc:
	[ -d $(builddir)/doc ] || mkdir $(builddir)/doc
	doxygen $(srcdir)/doc/doxygen-cppconsui.conf > /dev/null
	doxygen $(srcdir)/doc/doxygen-cim5.conf > /dev/null
	@echo "open doc/html/index.html"
