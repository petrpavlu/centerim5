#include <MainLoop.h>

#include <cppconsui/Button.h>
#include <cppconsui/CoreManager.h>
#include <cppconsui/KeyConfig.h>
#include <cppconsui/Label.h>
#include <cppconsui/Window.h>

#include <cassert>
#include <iostream>
#include <string>

// TestWindow class
class TestWindow
: public CppConsUI::Window
{
public:
  TestWindow();
  virtual ~TestWindow() {}

protected:

private:
  CppConsUI::Label *label;

  CONSUI_DISABLE_COPY(TestWindow);

  void onButtonActivate(CppConsUI::Button& activator);
};

TestWindow::TestWindow()
: CppConsUI::Window(0, 0, AUTOSIZE, AUTOSIZE)
{
  setClosable(false);

  addWidget(*(new CppConsUI::Label("Press F10 to quit.")), 1, 1);
  label = new CppConsUI::Label;
  addWidget(*label, 1, 2);

  CppConsUI::Button *button;

  button = new CppConsUI::Button(20, 1, "Normal button");
  button->signal_activate.connect(sigc::mem_fun(this,
        &TestWindow::onButtonActivate));
  addWidget(*button, 1, 8);

  button = new CppConsUI::Button("Simple autosize");
  button->signal_activate.connect(sigc::mem_fun(this,
        &TestWindow::onButtonActivate));
  addWidget(*button, 1, 10);

  button = new CppConsUI::Button(CppConsUI::Button::FLAG_VALUE,
      "Text+value button", "value");
  button->signal_activate.connect(sigc::mem_fun(this,
        &TestWindow::onButtonActivate));
  addWidget(*button, 1, 12);

  button = new CppConsUI::Button(CppConsUI::Button::FLAG_VALUE
      | CppConsUI::Button::FLAG_UNIT, "Text+value+unit button", "value",
      "unit");
  button->signal_activate.connect(sigc::mem_fun(this,
        &TestWindow::onButtonActivate));
  addWidget(*button, 1, 14);

  button = new CppConsUI::Button(CppConsUI::Button::FLAG_VALUE
      | CppConsUI::Button::FLAG_UNIT, "Text+value+unit\n2-line button",
      "value", "unit");
  button->signal_activate.connect(sigc::mem_fun(this,
        &TestWindow::onButtonActivate));
  addWidget(*button, 1, 16);

  button = new CppConsUI::Button(CppConsUI::Button::FLAG_VALUE
      | CppConsUI::Button::FLAG_UNIT, "Text+value+unit\n3-line\nbutton",
      "value", "unit");
  button->signal_activate.connect(sigc::mem_fun(this,
        &TestWindow::onButtonActivate));
  addWidget(*button, 1, 19);

  button = new CppConsUI::Button(CppConsUI::Button::FLAG_VALUE
      | CppConsUI::Button::FLAG_UNIT,
      "Text+value+unit\n4-line\n\nbutton", "value", "unit");
  button->signal_activate.connect(sigc::mem_fun(this,
        &TestWindow::onButtonActivate));
  addWidget(*button, 1, 23);

  button = new CppConsUI::Button(30, 1, CppConsUI::Button::FLAG_RIGHT,
      "Text+right button", NULL, NULL, "right");
  button->signal_activate.connect(sigc::mem_fun(this,
        &TestWindow::onButtonActivate));
  addWidget(*button, 1, 28);
}

void TestWindow::onButtonActivate(CppConsUI::Button& activator)
{
  std::string text = std::string(activator.getText()) + " activated.";
  label->setText(text.c_str());
}

// TestApp class
class TestApp
: public CppConsUI::InputProcessor
{
public:
  static int run();

protected:

private:
  static TestApp *my_instance;

  static void log_error_cppconsui(const char *message);

  TestApp() {}
  virtual ~TestApp() {}
  int runAll();

  CONSUI_DISABLE_COPY(TestApp);
};

TestApp *TestApp::my_instance = NULL;

int TestApp::run()
{
  // init my instance
  assert(!my_instance);
  my_instance = new TestApp;

  // run the program
  int res = my_instance->runAll();

  // finalize my instance
  assert(my_instance);

  delete my_instance;
  my_instance = NULL;

  return res;
}

void TestApp::log_error_cppconsui(const char * /*message*/)
{
  // ignore all messages
}

int TestApp::runAll()
{
  int res = 1;
  bool mainloop_initialized = false;
  bool cppconsui_initialized = false;
  TestWindow *win;

  // init locale support
  setlocale(LC_ALL, "");

  // init mainloop
  MainLoop::init();
  mainloop_initialized = true;

  // initialize CppConsUI
  CppConsUI::AppInterface interface = {
    MainLoop::timeout_add_cppconsui,
    MainLoop::timeout_remove_cppconsui,
    MainLoop::input_add_cppconsui,
    MainLoop::input_remove_cppconsui,
    log_error_cppconsui
  };
  int consui_res = CppConsUI::initializeConsUI(interface);
  if (consui_res) {
    std::cerr << "CppConsUI initialization failed." << std::endl;
    goto out;
  }
  cppconsui_initialized = true;

  // declare local bindables
  declareBindable("testapp", "quit", sigc::ptr_fun(MainLoop::quit),
      InputProcessor::BINDABLE_OVERRIDE);

  // create the main window
  win = new TestWindow;
  win->show();

  // setup key binds
  KEYCONFIG->loadDefaultKeyConfig();
  KEYCONFIG->bindKey("testapp", "quit", "F10");

  // run the main loop
  COREMANAGER->setTopInputProcessor(*this);
  COREMANAGER->enableResizing();
  MainLoop::run();

  // everything went ok
  res = 0;

out:
  // finalize CppConsUI
  if (cppconsui_initialized) {
    if (CppConsUI::finalizeConsUI())
      std::cerr << "CppConsUI finalization failed." << std::endl;
  }

  // finalize mainloop
  if (mainloop_initialized)
    MainLoop::finalize();

  return res;
}

// main function
int main()
{
  return TestApp::run();
}

/* vim: set tabstop=2 shiftwidth=2 textwidth=78 expandtab : */
