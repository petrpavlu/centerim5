#include <cppconsui/Label.h>
#include <cppconsui/Window.h>
#include <sstream>

// TestWindow class
class TestWindow : public CppConsUI::Window {
public:
  TestWindow(int number, int x, int y, int w, int h);
  virtual ~TestWindow() override {}

private:
  CONSUI_DISABLE_COPY(TestWindow);
};

TestWindow::TestWindow(int number, int x, int y, int w, int h)
  : CppConsUI::Window(x, y, w, h)
{
  CppConsUI::Label *label;

  std::string text = std::string("Win ") +
    dynamic_cast<std::ostringstream *>(&(std::ostringstream() << number))
      ->str();
  label = new CppConsUI::Label(w - 4, 1, text.c_str());
  addWidget(*label, 2, 1);

  if (number == 1) {
    label = new CppConsUI::Label("Press F10 to quit.");
    addWidget(*label, 2, 2);

    label = new CppConsUI::Label("Press ESC to close a focused window.");
    addWidget(*label, 2, 3);
  }
}

void setupTest()
{
  // Create test windows.
  for (int i = 1; i <= 4; ++i) {
    auto win = new TestWindow(i, (i - 1) % 2 * 40, (i - 1) / 2 * 10, 40, 10);
    win->show();
  }
}

// vim: set tabstop=2 shiftwidth=2 textwidth=80 expandtab
