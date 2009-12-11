bin_PROGRAMS = centerim
DEFS = @DEFS@ -DLOCALEDIR=\"$(localedir)\"

# When you add files here, also add them in po/POTFILES.in
centerim_SOURCES = \
	Accounts.cpp \
	Accounts.h \
	GeneralMenu.cpp \
	GeneralMenu.h \
	AccountWindow.cpp \
	AccountWindow.h \
	AccountStatusMenu.cpp \
	AccountStatusMenu.h \
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
	git-version.cpp \
	git-version.h.in

centerim_CPPFLAGS =\
	$(CFLAGS) \
	$(PURPLE_CFLAGS) \
	$(GLIBMM_CFLAGS) \
	$(CURSES_INCLUDEDIR) \
	$(DEBUG_CFLAGS) \
	-I$(top_srcdir)

centerim_LDADD =\
	$(PURPLE_LIBS) \
	$(GLIBMM_LIBS) \
	$(LIBINTL) \
	$(CURSES_LIBS) \
	$(top_builddir)/cppconsui/libcppconsui.la

# regenerate git-version.h if its content differs from the output of
# git-version-gen

git-version: .force_git_version
	@$(top_srcdir)/misc/git-version-gen $(top_srcdir)/.version git-version || echo "GIT version updated successfuly"

git-version.cpp: git-version.h

git-version.h : git-version git-version.h.in
	@VER_GIT=`cat git-version 2>/dev/null` || true; \
	sed -e "s/@GIT_VERSION@/$$VER_GIT/" < $(srcdir)/git-version.h.in > git-version.h

.PHONY: .force_git_version
