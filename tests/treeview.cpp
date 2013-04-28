#include <cppconsui/Button.h>
#include <cppconsui/CoreManager.h>
#include <cppconsui/Label.h>
#include <cppconsui/KeyConfig.h>
#include <cppconsui/TreeView.h>
#include <cppconsui/Window.h>

// TreeViewWindow class
class TreeViewWindow
: public CppConsUI::Window
{
public:
  /* This is a main window, make sure it can not be closed with ESC key by
   * overriding Close() method. */
  static TreeViewWindow *instance();
  virtual void close() {}

protected:

private:
  TreeViewWindow();
  virtual ~TreeViewWindow() {}
  TreeViewWindow(const TreeViewWindow&);
  TreeViewWindow& operator=(const TreeViewWindow&);
};

TreeViewWindow *TreeViewWindow::instance()
{
  static TreeViewWindow instance;
  return &instance;
}

TreeViewWindow::TreeViewWindow()
: Window(0, 0, AUTOSIZE, AUTOSIZE)
{
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
  static TestApp *instance();

  void run();

  // ignore every message
  static void g_log_func_(const gchar * /*log_domain*/,
      GLogLevelFlags /*log_level*/, const gchar * /*message*/,
      gpointer /*user_data*/)
    {}

protected:

private:
  CppConsUI::CoreManager *mngr;

  TestApp();
  TestApp(const TestApp&);
  TestApp& operator=(const TestApp&);
  virtual ~TestApp() {}
};

TestApp *TestApp::instance()
{
  static TestApp instance;
  return &instance;
}

TestApp::TestApp()
{
  mngr = CppConsUI::CoreManager::instance();
  KEYCONFIG->loadDefaultKeyConfig();
  KEYCONFIG->bindKey("testapp", "quit", "F10");

  g_log_set_default_handler(g_log_func_, this);

  declareBindable("testapp", "quit", sigc::mem_fun(mngr,
        &CppConsUI::CoreManager::quitMainLoop),
      InputProcessor::BINDABLE_OVERRIDE);
}

void TestApp::run()
{
  mngr->addWindow(*TreeViewWindow::instance());
  mngr->setTopInputProcessor(*this);
  mngr->enableResizing();
  mngr->startMainLoop();
}

// main function
int main()
{
  setlocale(LC_ALL, "");

  TestApp *app = TestApp::instance();
  app->run();

  return 0;
}
