#include <cppconsui/Label.h>
#include <cppconsui/Panel.h>
#include <cppconsui/TextEntry.h>
#include <cppconsui/Window.h>

// TestWindow class
class TestWindow : public CppConsUI::Window {
public:
  TestWindow();
  virtual ~TestWindow() override {}

private:
  CONSUI_DISABLE_COPY(TestWindow);
};

TestWindow::TestWindow() : CppConsUI::Window(0, 0, AUTOSIZE, AUTOSIZE)
{
  setClosable(false);

  addWidget(*(new CppConsUI::Label("Press F10 to quit.")), 1, 1);
  addWidget(*(new CppConsUI::Label("Press TAB to move focus.")), 1, 2);
  addWidget(*(new CppConsUI::Label(
              "All TextEntry widgets are surrouned by "
              "Panel widget in this test (except the autosize example).")),
    1, 3);

  addWidget(*(new CppConsUI::Panel(22, 3)), 1, 5);
  addWidget(*(new CppConsUI::TextEntry(20, 1, "Edit me.")), 2, 6);

  addWidget(*(new CppConsUI::Panel(22, 3)), 1, 9);
  addWidget(*(new CppConsUI::TextEntry(
              20, 1, "Too wide string, too wide string, too wide string")),
    2, 10);

  addWidget(*(new CppConsUI::Panel(22, 5)), 1, 13);
  addWidget(*(new CppConsUI::TextEntry(
              20, 3, "Multiline textentry, multiline textentry")),
    2, 14);

  // Unicode test.
  addWidget(*(new CppConsUI::Panel(32, 5)), 1, 19);
  addWidget(*(new CppConsUI::TextEntry(30, 3,
              "\x56\xc5\x99\x65\xc5\xa1\x74\xc3\xad\x63\xc3\xad\x20\x70\xc5\x99"
              "\xc3\xad\xc5\xa1\x65\x72\x79\x20\x73\x65\x20\x64\x6f\xc5\xbe\x61"
              "\x64\x6f\x76\x61\x6c\x79\x20\xc3\xba\x70\x6c\x6e\xc4\x9b\x20\xc4"
              "\x8d\x65\x72\x73\x74\x76\xc3\xbd\x63\x68\x20\xc5\x99\xc3\xad\x7a"
              "\x65\xc4\x8d\x6b\xc5\xaf\x2e\x0a")),
    2, 20);

  addWidget(*(new CppConsUI::TextEntry("Autosize")), 2, 25);
}

void setupTest()
{
  // Create the main window.
  auto win = new TestWindow;
  win->show();
}

// vim: set tabstop=2 shiftwidth=2 textwidth=80 expandtab
