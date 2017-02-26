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

button_SOURCES = \
	button.cpp \
	main.cpp

colorpicker_SOURCES = \
	colorpicker.cpp \
	main.cpp

label_SOURCES = \
	label.cpp \
	main.cpp

submenu_SOURCES = \
	submenu.cpp \
	main.cpp

textentry_SOURCES = \
	textentry.cpp \
	main.cpp

textview_SOURCES = \
	textview.cpp \
	main.cpp

treeview_SOURCES = \
	treeview.cpp \
	main.cpp

window_SOURCES = \
	window.cpp \
	main.cpp

if TERMEX_TESTS
terminfo: $(srcdir)/termex.src
	$(AM_V_GEN)$(TIC) -o terminfo "$(srcdir)/termex.src"
	$(AM_V_at)touch terminfo

check-local: terminfo

else
check-local:
	$(error tests cannot be run because required dependencies are missing, \
		reconfigure the build and install the tools that are reported \
		as needed for the testing)

endif
