#include <cppconsui/Button.h>
#include <cppconsui/Label.h>
#include <cppconsui/Window.h>

// TestWindow class
class TestWindow : public CppConsUI::Window {
public:
  TestWindow();
  virtual ~TestWindow() override {}

private:
  CppConsUI::Label *label;

  CONSUI_DISABLE_COPY(TestWindow);

  void onButtonActivate(CppConsUI::Button &activator);
};

TestWindow::TestWindow() : CppConsUI::Window(0, 0, AUTOSIZE, AUTOSIZE)
{
  setClosable(false);

  addWidget(*(new CppConsUI::Label("Press F10 to quit.")), 1, 1);
  label = new CppConsUI::Label;
  addWidget(*label, 1, 2);

  CppConsUI::Button *button;

  button = new CppConsUI::Button(20, 1, "Normal button");
  button->signal_activate.connect(
    sigc::mem_fun(this, &TestWindow::onButtonActivate));
  addWidget(*button, 1, 8);

  button = new CppConsUI::Button("Simple autosize");
  button->signal_activate.connect(
    sigc::mem_fun(this, &TestWindow::onButtonActivate));
  addWidget(*button, 1, 10);

  button = new CppConsUI::Button(
    CppConsUI::Button::FLAG_VALUE, "Text+value button", "value");
  button->signal_activate.connect(
    sigc::mem_fun(this, &TestWindow::onButtonActivate));
  addWidget(*button, 1, 12);

  button = new CppConsUI::Button(
    CppConsUI::Button::FLAG_VALUE | CppConsUI::Button::FLAG_UNIT,
    "Text+value+unit button", "value", "unit");
  button->signal_activate.connect(
    sigc::mem_fun(this, &TestWindow::onButtonActivate));
  addWidget(*button, 1, 14);

  button = new CppConsUI::Button(
    CppConsUI::Button::FLAG_VALUE | CppConsUI::Button::FLAG_UNIT,
    "Text+value+unit\n2-line button", "value", "unit");
  button->signal_activate.connect(
    sigc::mem_fun(this, &TestWindow::onButtonActivate));
  addWidget(*button, 1, 16);

  button = new CppConsUI::Button(
    CppConsUI::Button::FLAG_VALUE | CppConsUI::Button::FLAG_UNIT,
    "Text+value+unit\n3-line\nbutton", "value", "unit");
  button->signal_activate.connect(
    sigc::mem_fun(this, &TestWindow::onButtonActivate));
  addWidget(*button, 1, 19);

  button = new CppConsUI::Button(
    CppConsUI::Button::FLAG_VALUE | CppConsUI::Button::FLAG_UNIT,
    "Text+value+unit\n4-line\n\nbutton", "value", "unit");
  button->signal_activate.connect(
    sigc::mem_fun(this, &TestWindow::onButtonActivate));
  addWidget(*button, 1, 23);

  button = new CppConsUI::Button(30, 1, CppConsUI::Button::FLAG_RIGHT,
    "Text+right button", nullptr, nullptr, "right");
  button->signal_activate.connect(
    sigc::mem_fun(this, &TestWindow::onButtonActivate));
  addWidget(*button, 1, 28);
}

void TestWindow::onButtonActivate(CppConsUI::Button &activator)
{
  std::string text = std::string(activator.getText()) + " activated.";
  label->setText(text.c_str());
}

void setupTest()
{
  // Create the main window.
  auto win = new TestWindow;
  win->show();
}

// vim: set tabstop=2 shiftwidth=2 textwidth=80 expandtab
