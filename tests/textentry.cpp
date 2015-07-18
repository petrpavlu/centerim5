#include <MainLoop.h>

#include <cppconsui/CoreManager.h>
#include <cppconsui/KeyConfig.h>
#include <cppconsui/Label.h>
#include <cppconsui/Panel.h>
#include <cppconsui/TextEntry.h>
#include <cppconsui/Window.h>

#include <iostream>

// TestWindow class
class TestWindow : public CppConsUI::Window {
public:
  TestWindow();
  virtual ~TestWindow() {}

protected:
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

  // unicode test
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

// TestApp class
class TestApp : public CppConsUI::InputProcessor {
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
  CppConsUI::AppInterface interface = {MainLoop::timeout_add_cppconsui,
    MainLoop::timeout_remove_cppconsui, MainLoop::input_add_cppconsui,
    MainLoop::input_remove_cppconsui, log_error_cppconsui};
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

/* vim: set tabstop=2 shiftwidth=2 textwidth=80 expandtab : */
