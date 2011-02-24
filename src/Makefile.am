bin_PROGRAMS = centerim
DEFS = @DEFS@ -DLOCALEDIR=\"$(localedir)\"

# When you add files here, also add them in po/POTFILES.in
centerim_SOURCES = \
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

centerim_CPPFLAGS =\
	$(PURPLE_CFLAGS) \
	$(GLIB_CFLAGS) \
	$(SIGC_CFLAGS) \
	-I$(top_srcdir)

centerim_LDADD =\
	$(PURPLE_LIBS) \
	$(GLIB_LIBS) \
	$(SIGC_LIBS) \
	$(top_builddir)/cppconsui/libcppconsui.la

# regenerate git-version.h if its content differs from the output of
# git-version-gen

git-version: .force_git_version
	@$(top_srcdir)/misc/git-version-gen $(top_srcdir)/.version git-version || echo "GIT version updated successfully"

git-version.cpp: git-version.h

git-version.h : git-version git-version.h.in
	@VER_GIT=`cat git-version 2>/dev/null` || true; \
	sed -e "s/@GIT_VERSION@/$$VER_GIT/" < $(srcdir)/git-version.h.in > git-version.h

.PHONY: .force_git_version
