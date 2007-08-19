bin_PROGRAMS = centerim
DEFS = @DEFS@ -DLOCALEDIR=\"$(localedir)\"

# When you add files here, also add them in po/POTFILES.in
centerim_SOURCES = \
	Accounts.cpp \
	Accounts.h \
	BuddyList.cpp \
	BuddyList.h \
	BuddyListNode.cpp \
	BuddyListNode.h \
	CenterIM.cpp \
	CenterIM.h \
	CenterMain.cpp \
	CenterMain.h \
	Conf.cpp \
	Conf.h \
	Connections.cpp \
	Connections.h \
	Conversation.cpp \
	Conversation.h \
	Conversations.cpp \
	Conversations.h \
	Defines.h \
	Transfers.cpp \
	Transfers.h \
	Log.cpp \
	Log.h \
	gettext.h

centerim_CPPFLAGS =\
	$(PURPLE_CFLAGS) \
	$(GLIBMM_CFLAGS) \
	$(GLIB_CFLAGS) \
	$(CURSES_INCLUDEDIR)

centerim_LDADD =\
	$(PURPLE_LIBS) \
	$(GLIBMM_LIBS) \
	$(GLIB_LIBS) \
	$(CURSES_LIBS) \
	$(LIBINTL) \
	$(top_builddir)/cppconsui/libcppconsui.la
