#include <cppconsui/Button.h>
#include <cppconsui/CoreManager.h>
#include <cppconsui/Label.h>
#include <cppconsui/KeyConfig.h>
#include <cppconsui/TreeView.h>
#include <cppconsui/Window.h>

#include <stdio.h>

// TestWindow class
class TestWindow
: public CppConsUI::Window
{
public:
  TestWindow();
  virtual ~TestWindow() {}

protected:

private:
  TestWindow(const TestWindow&);
  TestWindow& operator=(const TestWindow&);
};

TestWindow::TestWindow()
: Window(0, 0, AUTOSIZE, AUTOSIZE)
{
  setClosable(false);

  CppConsUI::TreeView *tree;
  CppConsUI::TreeView::NodeReference node;
  CppConsUI::TreeView::NodeReference node2;

  addWidget(*(new CppConsUI::Label(20, 1, "Press F10 to quit.")), 1, 1);

  tree = new CppConsUI::TreeView(30, 12);
  addWidget(*tree, 1, 3);
  setInputChild(*tree);

  node = tree->appendNode(tree->getRootNode(),
      *(new CppConsUI::Button("Button node A")));
  node2 = tree->appendNode(node, *(new CppConsUI::Button("Button node A-1")));
  tree->appendNode(node2, *(new CppConsUI::Button("Button node A-1-a")));
  tree->appendNode(node2, *(new CppConsUI::Button("Button node A-1-b")));
  tree->appendNode(node2, *(new CppConsUI::Button("Button node A-1-c")));
  tree->appendNode(node, *(new CppConsUI::Button("Button node A-2")));
  tree->appendNode(node, *(new CppConsUI::Button("Button node A-3")));

  node = tree->appendNode(tree->getRootNode(),
      *(new CppConsUI::Label("Label node B")));
  tree->appendNode(node, *(new CppConsUI::Label("Label node B-1")));
  tree->appendNode(node, *(new CppConsUI::Label("Label node B-2")));
  tree->appendNode(node, *(new CppConsUI::Label("Label node B-3")));

  node = tree->appendNode(tree->getRootNode(),
      *(new CppConsUI::Button("Button node C")));
  tree->appendNode(node, *(new CppConsUI::Button("Button node C-1")));
  tree->appendNode(node, *(new CppConsUI::Button("Button node C-2")));
  tree->appendNode(node, *(new CppConsUI::Button("Button node C-3")));
}

// TestApp class
class TestApp
: public CppConsUI::InputProcessor
{
public:
  TestApp();
  virtual ~TestApp() {}

  void run();

  // ignore every message
  static void g_log_func_(const gchar * /*log_domain*/,
      GLogLevelFlags /*log_level*/, const gchar * /*message*/,
      gpointer /*user_data*/)
    {}

protected:

private:
  TestApp(const TestApp&);
  TestApp& operator=(const TestApp&);
};

TestApp::TestApp()
{
  KEYCONFIG->loadDefaultKeyConfig();
  KEYCONFIG->bindKey("testapp", "quit", "F10");

  g_log_set_default_handler(g_log_func_, this);

  declareBindable("testapp", "quit", sigc::mem_fun(COREMANAGER,
        &CppConsUI::CoreManager::quitMainLoop),
      InputProcessor::BINDABLE_OVERRIDE);
}

void TestApp::run()
{
  TestWindow *win = new TestWindow;
  win->show();

  COREMANAGER->setTopInputProcessor(*this);
  COREMANAGER->enableResizing();
  COREMANAGER->startMainLoop();
}

// main function
int main()
{
  setlocale(LC_ALL, "");

  // initialize CppConsUI
  int consui_res = CppConsUI::initializeConsUI();
  if (consui_res) {
    fprintf(stderr, "CppConsUI initialization failed.\n");
    return consui_res;
  }

  TestApp *app = new TestApp;
  app->run();
  delete app;

  // finalize CppConsUI
  consui_res = CppConsUI::finalizeConsUI();
  if (consui_res) {
    fprintf(stderr, "CppConsUI deinitialization failed.\n");
    return consui_res;
  }

  return 0;
}

/* vim: set tabstop=2 shiftwidth=2 textwidth=78 expandtab : */
