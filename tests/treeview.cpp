#include <MainLoop.h>

#include <cppconsui/Button.h>
#include <cppconsui/CoreManager.h>
#include <cppconsui/Label.h>
#include <cppconsui/KeyConfig.h>
#include <cppconsui/TreeView.h>
#include <cppconsui/Window.h>

#include <iostream>

// TestWindow class
class TestWindow : public CppConsUI::Window {
public:
  TestWindow();
  virtual ~TestWindow() {}

private:
  CONSUI_DISABLE_COPY(TestWindow);
};

TestWindow::TestWindow() : Window(0, 0, AUTOSIZE, AUTOSIZE)
{
  setClosable(false);

  CppConsUI::TreeView *tree;
  CppConsUI::TreeView::NodeReference node;
  CppConsUI::TreeView::NodeReference node2;

  addWidget(*(new CppConsUI::Label(20, 1, "Press F10 to quit.")), 1, 1);

  tree = new CppConsUI::TreeView(30, 12);
  addWidget(*tree, 1, 3);
  setInputChild(*tree);

  node = tree->appendNode(
    tree->getRootNode(), *(new CppConsUI::Button("Button node A")));
  node2 = tree->appendNode(node, *(new CppConsUI::Button("Button node A-1")));
  tree->appendNode(node2, *(new CppConsUI::Button("Button node A-1-a")));
  tree->appendNode(node2, *(new CppConsUI::Button("Button node A-1-b")));
  tree->appendNode(node2, *(new CppConsUI::Button("Button node A-1-c")));
  tree->appendNode(node, *(new CppConsUI::Button("Button node A-2")));
  tree->appendNode(node, *(new CppConsUI::Button("Button node A-3")));

  node = tree->appendNode(
    tree->getRootNode(), *(new CppConsUI::Label("Label node B")));
  tree->appendNode(node, *(new CppConsUI::Label("Label node B-1")));
  tree->appendNode(node, *(new CppConsUI::Label("Label node B-2")));
  tree->appendNode(node, *(new CppConsUI::Label("Label node B-3")));

  node = tree->appendNode(
    tree->getRootNode(), *(new CppConsUI::Button("Button node C")));
  tree->appendNode(node, *(new CppConsUI::Button("Button node C-1")));
  tree->appendNode(node, *(new CppConsUI::Button("Button node C-2")));
  tree->appendNode(node, *(new CppConsUI::Button("Button node C-3")));
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
