ACLOCAL_AMFLAGS = -I m4

SUBDIRS = contrib cppconsui doc misc plugins po src tests

EXTRA_DIST = \
	HACKING \
	bootstrap \
	gettext.h

dist_man_MANS = centerim5.1

dist-hook:
	$(AM_V_at)echo $(VERSION) > $(distdir)/version

.PHONY: doc
doc:
	$(AM_V_at)cd doc; $(MAKE) doc

# Package in all cmake files.
EXTRA_DIST += \
	CMakeLists.txt \
	cmake/FindIconv.cmake \
	config.h.cmake \
	cppconsui/CMakeLists.txt \
	doc/CMakeLists.txt \
	plugins/CMakeLists.txt \
	po/CMakeLists.txt \
	src/CMakeLists.txt \
	tests/CMakeLists.txt
