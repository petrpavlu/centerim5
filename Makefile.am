SUBDIRS = po cppconsui tests src

EXTRA_DIST = HACKING

ACLOCAL_AMFLAGS = -I m4

.PHONY: doc
doc:
	doxygen doc/doxygen-cppconsui.conf > /dev/null
	doxygen doc/doxygen-cim5.conf > /dev/null
	@echo "open doc/html/index.html"
