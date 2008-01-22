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
	git-version.cpp \
	git-version.h.in \
	gettext.h

centerim_CPPFLAGS =\
	$(PURPLE_CFLAGS) \
	$(GLIBMM_CFLAGS) \
	$(NCURSESW_CFLAGS) \
	-I$(top_srcdir)

centerim_LDADD =\
	$(PURPLE_LIBS) \
	$(GLIBMM_LIBS) \
	$(LIBINTL) \
	$(top_builddir)/cppconsui/libcppconsui.la

# regenerate git-version.h if its content differs from the output of
# git-version-gen
g_get_cur = sed -ne "s/^\#[ \x09]*define[ \x09]\+GIT_VERSION[ \x09]\+\"\([0-9]\+.*\)\"/\1/p" \
             2>/dev/null < git-version.h | tr -d '\012'
g_subst_f  = sed -e "s/@GIT_VERSION@/$$VER_GIT/" < git-version.h.in > git-version.h

$(centerim_OBJECTS) git-version.h : | git_version
.PHONY: git_version

git_version: git-version.h.in
	VER_FILE=`$(g_get_cur)` || true; \
	VER_GIT=`cd $(top_srcdir); ./misc/git-version-gen .version`; \
	if [ -z $$VER_FILE ] || [ $$VER_FILE != $$VER_GIT ]; then \
		$(g_subst_f); \
	fi
