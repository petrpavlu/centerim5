#include <MainLoop.h>

#include <cppconsui/Button.h>
#include <cppconsui/CoreManager.h>
#include <cppconsui/KeyConfig.h>
#include <cppconsui/Label.h>
#include <cppconsui/MenuWindow.h>
#include <cppconsui/Window.h>

#include <iostream>
#include <string>

// TestWindow class
class TestWindow : public CppConsUI::Window {
public:
  TestWindow();
  virtual ~TestWindow();

private:
  CppConsUI::MenuWindow *menu;

  CONSUI_DISABLE_COPY(TestWindow);
};

TestWindow::TestWindow() : CppConsUI::Window(0, 0, AUTOSIZE, AUTOSIZE)
{
  setClosable(false);

  addWidget(*(new CppConsUI::Label("Press F10 to quit.")), 1, 1);

  CppConsUI::Button *button = new CppConsUI::Button("Open Menu...");
  addWidget(*button, 1, 3);

  menu = new CppConsUI::MenuWindow(*button, AUTOSIZE, AUTOSIZE);
  menu->setHideOnClose(true);

  button->signal_activate.connect(
    sigc::hide(sigc::mem_fun(menu, &CppConsUI::MenuWindow::show)));

  menu->appendItem(
    "Item 1", sigc::hide(sigc::mem_fun(menu, &CppConsUI::MenuWindow::hide)));
  menu->appendItem(
    "Item 2", sigc::hide(sigc::mem_fun(menu, &CppConsUI::MenuWindow::hide)));
  menu->appendItem(
    "Item 3", sigc::hide(sigc::mem_fun(menu, &CppConsUI::MenuWindow::hide)));
  menu->appendItem(
    "Item 4", sigc::hide(sigc::mem_fun(menu, &CppConsUI::MenuWindow::hide)));
  menu->appendSeparator();

  CppConsUI::MenuWindow *submenu =
    new CppConsUI::MenuWindow(*button, AUTOSIZE, AUTOSIZE);
  submenu->appendItem(
    "Item 1", sigc::hide(sigc::mem_fun(submenu, &CppConsUI::MenuWindow::hide)));
  submenu->appendItem(
    "Item 2", sigc::hide(sigc::mem_fun(submenu, &CppConsUI::MenuWindow::hide)));
  submenu->appendItem(
    "Item 3", sigc::hide(sigc::mem_fun(submenu, &CppConsUI::MenuWindow::hide)));
  submenu->appendItem(
    "Item 3", sigc::hide(sigc::mem_fun(submenu, &CppConsUI::MenuWindow::hide)));
  menu->appendSubMenu("First submenu", *submenu);

  submenu = new CppConsUI::MenuWindow(*button, AUTOSIZE, AUTOSIZE);
  submenu->appendItem(
    "Item 1", sigc::hide(sigc::mem_fun(menu, &CppConsUI::MenuWindow::hide)));
  submenu->appendItem(
    "Item 2", sigc::hide(sigc::mem_fun(menu, &CppConsUI::MenuWindow::hide)));
  submenu->appendItem(
    "Item 3", sigc::hide(sigc::mem_fun(menu, &CppConsUI::MenuWindow::hide)));
  submenu->appendItem(
    "Item 4", sigc::hide(sigc::mem_fun(menu, &CppConsUI::MenuWindow::hide)));
  menu->appendSubMenu("Second submenu", *submenu);
}

TestWindow::~TestWindow()
{
  delete menu;
}

// TestApp class
class TestApp : public CppConsUI::InputProcessor {
public:
  static int run();

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
