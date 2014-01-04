#include <MainLoop.h>

#include <cppconsui/CoreManager.h>
#include <cppconsui/KeyConfig.h>
#include <cppconsui/Label.h>
#include <cppconsui/Window.h>

#include <iostream>
#include <sstream>

// TestWindow class
class TestWindow
: public CppConsUI::Window
{
public:
  TestWindow(int number, int x, int y, int w, int h);
  virtual ~TestWindow() {}

protected:

private:
  TestWindow(const TestWindow&);
  TestWindow& operator=(const TestWindow&);
};

TestWindow::TestWindow(int number, int x, int y, int w, int h)
: CppConsUI::Window(x, y, w, h)
{
  CppConsUI::Label *label;

  std::string text = std::string("Win ")
    + dynamic_cast<std::ostringstream*>(
        &(std::ostringstream() << number))->str();
  label = new CppConsUI::Label(w - 4, 1, text.c_str());
  addWidget(*label, 2, 1);

  if (number == 1) {
    label = new CppConsUI::Label("Press F10 to quit.");
    addWidget(*label, 2, 2);

    label = new CppConsUI::Label("Press ESC to close a focused window.");
    addWidget(*label, 2, 3);
  }
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

  // create test windows
  for (int i = 1; i <= 4; i++) {
    TestWindow *win = new TestWindow(i, (i - 1) % 2 * 40, (i - 1) / 2 * 10,
        40, 10);
    win->show();
  }

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
