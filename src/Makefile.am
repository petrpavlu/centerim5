bin_PROGRAMS = centerim5

# when you add files here, also add them in po/POTFILES.in
centerim5_SOURCES = \
	AccountStatusMenu.cpp \
	AccountStatusMenu.h \
	AccountWindow.cpp \
	AccountWindow.h \
	Accounts.cpp \
	Accounts.h \
	BuddyList.cpp \
	BuddyList.h \
	BuddyListNode.cpp \
	BuddyListNode.h \
	CenterIM.cpp \
	CenterIM.h \
	CenterMain.cpp \
	Conversation.cpp \
	Conversation.h \
	Conversations.cpp \
	Conversations.h \
	Connections.cpp \
	Connections.h \
	Footer.cpp \
	Footer.h \
	GeneralMenu.cpp \
	GeneralMenu.h \
	Header.cpp \
	Header.h \
	Log.cpp \
	Log.h \
	Notify.cpp \
	Notify.h \
	OptionWindow.cpp \
	OptionWindow.h \
	Request.cpp \
	Request.h \
	Transfers.cpp \
	Transfers.h \
	Utils.cpp \
	Utils.h \
	git-version.cpp \
	git-version.h.in

centerim5_CPPFLAGS =\
	$(PURPLE_CFLAGS) \
	$(GLIB_CFLAGS) \
	$(SIGC_CFLAGS) \
	-I$(top_srcdir)

centerim5_LDADD =\
	$(PURPLE_LIBS) \
	$(GLIB_LIBS) \
	$(SIGC_LIBS) \
	$(top_builddir)/cppconsui/libcppconsui.la

git-version.cpp: git-version.h

git-version.h : $(top_srcdir)/.version git-version.h.in
	@sed -e "s/@GIT_VERSION@/`cat $(top_srcdir)/.version`/" < $(srcdir)/git-version.h.in > git-version.h
