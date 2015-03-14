# when you add files here, also add them in po/POTFILES.in
set(cppconsui_SOURCES
  AbstractDialog.cpp
  AbstractLine.cpp
  AbstractListBox.cpp
  Button.cpp
  CheckBox.cpp
  ColorPicker.cpp
  ColorPickerPalette.cpp
  ColorPickerComboBox.cpp
  ColorPickerDialog.cpp
  ColorScheme.cpp
  ConsUICurses.cpp
  Container.cpp
  ComboBox.cpp
  CoreManager.cpp
  Dialog.cpp
  FreeWindow.cpp
  HorizontalLine.cpp
  HorizontalListBox.cpp
  InputDialog.cpp
  InputProcessor.cpp
  Label.cpp
  ListBox.cpp
  KeyConfig.cpp
  Keys.cpp
  MenuWindow.cpp
  MessageDialog.cpp
  Panel.cpp
  ScrollPane.cpp
  Spacer.cpp
  SplitDialog.cpp
  TextEdit.cpp
  TextEntry.cpp
  TextView.cpp
  TreeView.cpp
  VerticalLine.cpp
  Widget.cpp
  Window.cpp
	libtermkey/driver-csi.c
	libtermkey/driver-ti.c
	libtermkey/termkey.c)

set(cppconsui_HEADERS
  AbstractDialog.h
  AbstractLine.h
  AbstractListBox.h
  Button.h
  CheckBox.h
  ColorPicker.h
  ColorPickerPalette.h
  ColorPickerDialog.h
  ColorScheme.h
  ConsUICurses.h
  Container.h
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
  ListBox.h
  KeyConfig.h
  Keys.h
  MenuWindow.h
  MessageDialog.h
  Panel.h
  ScrollPane.h
  Spacer.h
  SplitDialog.h
  TextEdit.h
  TextEntry.h
  TextView.h
  TreeView.h
  VerticalLine.h
  Widget.h
  Window.h
  tree.hh
	libtermkey/termkey-internal.h
	libtermkey/termkey.h)

add_definitions(-DG_LOG_DOMAIN="cppconsui")

add_library(cppconsui SHARED
  ${cppconsui_SOURCES}
  ${cppconsui_HEADERS})

set_target_properties(cppconsui
  PROPERTIES VERSION 0.0.0)

target_link_libraries(cppconsui
  ${CURSES_LIBRARIES}
  ${GLIB2_LIBRARIES}
  ${SIGC_LIBRARIES})

install(TARGETS cppconsui DESTINATION lib)
