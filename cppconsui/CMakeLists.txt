# @TODO make sure ConsuiLog.h and ConsuiCurses.h (*Internal.h) are not distributed

# When you add files here, also add them in po/POTFILES.in
set( cppconsui_SOURCES
	AbstractLine.cpp
	AbstractListBox.cpp
	Button.cpp
	ConsuiCurses.cpp
	Container.cpp
	ColorScheme.cpp
	ComboBox.cpp
	CoreManager.cpp
	CppConsUI.cpp
	Dialog.cpp
	FreeWindow.cpp
	HorizontalLine.cpp
	HorizontalListBox.cpp
	InputDialog.cpp
	InputProcessor.cpp
	Label.cpp
	LineStyle.cpp
	ListBox.cpp
	KeyConfig.cpp
	Keys.cpp
	MenuWindow.cpp
	MessageDialog.cpp
	Panel.cpp
	TextEdit.cpp
	TextEntry.cpp
	TextView.cpp
	TreeView.cpp
	ScrollPane.cpp
	Spacer.cpp
	SplitDialog.cpp
	VerticalLine.cpp
	Widget.cpp
	Window.cpp
)

set( cppconsui_HEADERS
	AbstractLine.h
	AbstractListBox.h
	Button.h
	ConsuiCurses.h
	Container.h
	ColorScheme.h
	ComboBox.h
	CoreManager.h
	CppConsUI.h
	Dialog.h
	FreeWindow.h
	HorizontalLine.h
	HorizontalListBox.h
	InputDialog.h
	InputProcessor.h
	Label.h
	LineStyle.h
	ListBox.h
	KeyConfig.h
	Keys.h
	MenuWindow.h
	MessageDialog.h
	Panel.h
	Spacer.h
	SplitDialog.h
	TextEdit.h
	TextEntry.h
	TextView.h
	TreeView.h
	ScrollPane.h
	VerticalLine.h
	Widget.h
	Window.h
	tree.hh
)

add_library(	cppconsui STATIC
		${cppconsui_SOURCES}
		${cppconsui_HEADERS}
	)

target_link_libraries( cppconsui
		termkey
		${CURSES_LIBRARIES}
		${GLIB2_LIBRARIES}
		${SIGC_LIBRARIES}
	)
