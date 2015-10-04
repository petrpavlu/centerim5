#include <cppconsui/Label.h>
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

  CppConsUI::Label *label;

  label = new CppConsUI::Label(20, // width
    1,                             // height
    "Press F10 to quit.");         // text
  // Add the label to the container. It will take ownership of the widget and
  // delete it when no longer needed.
  addWidget(*label, 1, 1);

  label = new CppConsUI::Label(
    20, 1, "Too wide string, too wide string, too wide string");
  addWidget(*label, 1, 3);

  label = new CppConsUI::Label(
    20, 3, "Multiline label, multiline label, multiline label");
  addWidget(*label, 1, 5);

  label = new CppConsUI::Label(
    "Auto multiline label,\nauto multiline label,\nauto multiline label");
  addWidget(*label, 1, 9);

  // Unicode test.
  label = new CppConsUI::Label(30, 3,
    "\x56\xc5\x99\x65\xc5\xa1\x74\xc3\xad\x63\xc3\xad\x20\x70\xc5\x99"
    "\xc3\xad\xc5\xa1\x65\x72\x79\x20\x73\x65\x20\x64\x6f\xc5\xbe\x61"
    "\x64\x6f\x76\x61\x6c\x79\x20\xc3\xba\x70\x6c\x6e\xc4\x9b\x20\xc4"
    "\x8d\x65\x72\x73\x74\x76\xc3\xbd\x63\x68\x20\xc5\x99\xc3\xad\x7a"
    "\x65\xc4\x8d\x6b\xc5\xaf\x2e\x0a");
  addWidget(*label, 1, 13);

  label = new CppConsUI::Label("Autosize");
  addWidget(*label, 1, 17);

  const char *long_text =
    "Lorem ipsum dolor sit amet, consectetur"
    "adipiscing elit. Duis dui dui, interdum eget tempor auctor, viverra"
    "suscipit velit. Phasellus vel magna odio. Duis rutrum tortor at nisi"
    "auctor tincidunt. Mauris libero neque, faucibus sit amet semper in, "
    "dictum ut tortor. Duis lacinia justo non lorem blandit ultrices."
    "Nullam vel purus erat, eget aliquam massa. Aenean eget mi a nunc"
    "lacinia consectetur sed a neque. Cras varius, dolor nec rhoncus"
    "ultricies, leo ipsum adipiscing mi, vel feugiat ipsum urna id "
    "metus. Cras non pulvinar nisi. Vivamus nisi lorem, tempor tristique"
    "cursus sit amet, ultricies interdum metus. Nullam tortor tortor, "
    "iaculis sed tempor non, tincidunt ac mi. Quisque id diam vitae diam"
    "dictum facilisis eget ac lacus. Vivamus at gravida felis. Curabitur"
    "fermentum mattis eros, ut auctor urna tincidunt vitae. Praesent"
    "tincidunt laoreet lobortis.";

  label = new CppConsUI::Label(AUTOSIZE, 10, long_text);
  addWidget(*label, 42, 17);

  label = new CppConsUI::Label(40, AUTOSIZE, long_text);
  addWidget(*label, 1, 28);

  label = new CppConsUI::Label(AUTOSIZE, AUTOSIZE, long_text);
  addWidget(*label, 42, 28);
}

void setupTest()
{
  // Create the main window.
  auto win = new TestWindow;
  win->show();
}

// vim: set tabstop=2 shiftwidth=2 textwidth=80 expandtab
