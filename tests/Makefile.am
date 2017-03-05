check_PROGRAMS = \
	button \
	colorpicker \
	label \
	submenu \
	textentry \
	textview \
	treeview \
	window

AM_CPPFLAGS = \
	$(SIGC_CFLAGS) \
	-I$(top_srcdir)

LDADD = \
	$(SIGC_LIBS) \
	$(top_builddir)/cppconsui/libcppconsui.la

button_SOURCES = button.cpp main.cpp
colorpicker_SOURCES = colorpicker.cpp main.cpp
label_SOURCES = label.cpp main.cpp
submenu_SOURCES = submenu.cpp main.cpp
textentry_SOURCES = textentry.cpp main.cpp
textview_SOURCES = textview.cpp main.cpp
treeview_SOURCES = treeview.cpp main.cpp
window_SOURCES = window.cpp main.cpp

# List of test playbooks.
TERMEX_TESTS = \
	treeview.test

EXTRA_DIST = \
	termex.py \
	termex.src \
	$(TERMEX_TESTS)

if TERMEX_TESTS
terminfo: termex.src
	$(AM_V_GEN)$(TIC) -o terminfo "$(srcdir)/termex.src"
	$(AM_V_at)touch terminfo

TEST_LOG_COMPILER = $(PYTHON) "$(srcdir)/termex.py"
AM_TEST_LOG_FLAGS = -t "$(abs_builddir)/terminfo" test
TESTS = $(TERMEX_TESTS)

# Use check_DATA to compile terminfo. This target is guaranteed to be built
# before check-TESTS, unlike check-local.
check_DATA = terminfo

else
fail-test:
	$(error tests cannot be run because required dependencies are missing, \
		reconfigure the build and install the tools that are reported \
		as needed for the testing)

TESTS = fail-test
endif

mostlyclean-local:
	-$(AM_V_at)rm -r terminfo
