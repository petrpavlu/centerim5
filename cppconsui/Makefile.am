lib_LTLIBRARIES = libcppconsui.la

# @TODO make sure ConsuiLog.h and ConsuiCurses.h (*Internal.h) are not distributed

# When you add files here, also add them in po/POTFILES.in
libcppconsui_la_SOURCES =\
	AbstractListBox.cpp \
	AbstractListBox.h \
	Application.cpp \
	Application.h \
	Button.cpp \
	Button.h \
	ConsuiCurses.cpp \
	ConsuiCurses.h \
	Container.cpp \
	Container.h \
	ColorScheme.h \
	ColorScheme.cpp \
	ComboBox.cpp \
	ComboBox.h \
	CppConsUI.cpp \
	CppConsUI.h \
	CppConsUIInternal.h \
	Dialog.cpp \
	Dialog.h \
	HorizontalLine.cpp \
	HorizontalLine.h \
	HorizontalListBox.cpp \
	HorizontalListBox.h \
	InputDialog.cpp \
	InputDialog.h \
	InputProcessor.cpp \
	InputProcessor.h \
	Label.cpp \
	Label.h \
	LineStyle.cpp \
	LineStyle.h \
	ListBox.cpp \
	ListBox.h \
	KeyConfig.cpp \
	KeyConfig.h \
	Keys.cpp \
	Keys.h \
	MenuWindow.cpp \
	MenuWindow.h \
	MessageDialog.cpp \
	MessageDialog.h \
	Panel.cpp \
	Panel.h \
	TextEdit.cpp \
	TextEdit.h \
	TextEntry.cpp \
	TextEntry.h \
	TextView.cpp \
	TextView.h \
	TreeView.cpp \
	TreeView.h \
	ScrollPane.cpp \
	ScrollPane.h \
	VerticalLine.cpp \
	VerticalLine.h \
	Widget.cpp \
	Widget.h \
	Window.cpp \
	Window.h \
	WindowManager.cpp \
	WindowManager.h \
	tree.hh \
	\
	TextBrowser.cpp \
	TextBrowser.h \
	TextBuffer.cpp \
	TextBuffer.h \
	TextInput.cpp \
	TextInput.h \
	TextLine.cpp \
	TextLine.h \
	TextLineRBTree.cpp \
	TextLineRBTree.h \
	TextRBTree.cpp \
	TextRBTree.h \
	TextRBView.cpp \
	TextRBView.h

libcppconsui_la_CPPFLAGS = $(CFLAGS) $(GLIBMM_CFLAGS) $(CURSES_INCLUDEDIR) $(DEBUG_CFLAGS) -DG_LOG_DOMAIN=\"cppconsui\"
libcppconsui_la_LIBADD = $(GLIBMM_LIBS) $(CURSES_LIBS)
