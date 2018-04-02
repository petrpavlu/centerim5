# When you add files here, also add them in po/POTFILES.in.
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
  CppConsUI.cpp
  Dialog.cpp
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
  Spacer.cpp
  SplitDialog.cpp
  TextEdit.cpp
  TextEntry.cpp
  TextView.cpp
  TreeView.cpp
  VerticalLine.cpp
  Widget.cpp
  Window.cpp)

add_library(cppconsui SHARED ${cppconsui_SOURCES})

set_property(TARGET cppconsui
  PROPERTY VERSION 0.0.0)
target_include_directories(cppconsui PRIVATE
  ${Iconv_INCLUDE_DIRS} ${Intl_INCLUDE_DIRS})
target_compile_options(cppconsui PRIVATE
  ${SIGC_CFLAGS} ${NCURSESW_CFLAGS} ${TERMKEY_CFLAGS}
  "-DG_LOG_DOMAIN=\"cppconsui\"")
target_link_libraries(cppconsui PRIVATE
  ${Iconv_LIBRARIES} ${Intl_LIBRARIES} ${SIGC_LDFLAGS} ${NCURSESW_LDFLAGS}
  ${TERMKEY_LDFLAGS})

install(TARGETS cppconsui DESTINATION lib)
