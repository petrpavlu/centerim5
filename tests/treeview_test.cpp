#include <cppconsui/Button.h>
#include <cppconsui/CoreManager.h>
#include <cppconsui/Label.h>
#include <cppconsui/KeyConfig.h>
#include <cppconsui/Keys.h>
#include <cppconsui/TreeView.h>
#include <cppconsui/Window.h>

// TreeViewWindow class
class TreeViewWindow
: public CppConsUI::Window
{
  public:
    /* This is a main window, make sure it can not be closed with ESC key by
     * overriding Close() method. */
    static TreeViewWindow *Instance();
    virtual void Close() {}

    virtual void ScreenResized();

  protected:

  private:
    TreeViewWindow();
    virtual ~TreeViewWindow() {}
    TreeViewWindow(const TreeViewWindow&);
    TreeViewWindow& operator=(const TreeViewWindow&);
};

TreeViewWindow *TreeViewWindow::Instance()
{
  static TreeViewWindow instance;
  return &instance;
}

TreeViewWindow::TreeViewWindow()
: Window(0, 0, 0, 0)
{
  CppConsUI::TreeView *tree;
  CppConsUI::TreeView::NodeReference node;
  CppConsUI::TreeView::NodeReference node2;

  AddWidget(*(new CppConsUI::Label(20, 1, "Press F10 to quit.")), 1, 1);

  tree = new CppConsUI::TreeView(30, 12);
  AddWidget(*tree, 1, 3);
  SetInputChild(*tree);

  node = tree->AppendNode(tree->GetRootNode(),
      *(new CppConsUI::Button("Button node A")));
  node2 = tree->AppendNode(node, *(new CppConsUI::Button("Button node A-1")));
  tree->AppendNode(node2, *(new CppConsUI::Button("Button node A-1-a")));
  tree->AppendNode(node2, *(new CppConsUI::Button("Button node A-1-b")));
  tree->AppendNode(node2, *(new CppConsUI::Button("Button node A-1-c")));
  tree->AppendNode(node, *(new CppConsUI::Button("Button node A-2")));
  tree->AppendNode(node, *(new CppConsUI::Button("Button node A-3")));

  node = tree->AppendNode(tree->GetRootNode(),
      *(new CppConsUI::Label("Label node B")));
  tree->AppendNode(node, *(new CppConsUI::Label("Label node B-1")));
  tree->AppendNode(node, *(new CppConsUI::Label("Label node B-2")));
  tree->AppendNode(node, *(new CppConsUI::Label("Label node B-3")));

  node = tree->AppendNode(tree->GetRootNode(),
      *(new CppConsUI::Button("Button node C")));
  tree->AppendNode(node, *(new CppConsUI::Button("Button node C-1")));
  tree->AppendNode(node, *(new CppConsUI::Button("Button node C-2")));
  tree->AppendNode(node, *(new CppConsUI::Button("Button node C-3")));
}

void TreeViewWindow::ScreenResized()
{
  MoveResize(0, 0, CppConsUI::Curses::getmaxx(),
      CppConsUI::Curses::getmaxy());
}

// TestApp class
class TestApp
: public CppConsUI::InputProcessor
{
  public:
    static TestApp *Instance();

    void Run();

    // ignore every message
    static void g_log_func_(const gchar *log_domain, GLogLevelFlags log_level,
        const gchar *message, gpointer user_data)
      {}

  protected:

  private:
    CppConsUI::CoreManager *mngr;

    TestApp();
    TestApp(const TestApp&);
    TestApp& operator=(const TestApp&);
    virtual ~TestApp() {}
};

TestApp *TestApp::Instance()
{
  static TestApp instance;
  return &instance;
}

TestApp::TestApp()
{
  mngr = CppConsUI::CoreManager::Instance();
  KEYCONFIG->AddDefaultKeyBind("testapp", "quit", "F10");
  KEYCONFIG->RegisterDefaultKeyBinds();

  g_log_set_default_handler(g_log_func_, this);

  DeclareBindable("testapp", "quit", sigc::mem_fun(mngr,
        &CppConsUI::CoreManager::QuitMainLoop),
      InputProcessor::BINDABLE_OVERRIDE);
}

void TestApp::Run()
{
  mngr->AddWindow(*TreeViewWindow::Instance());
  mngr->SetTopInputProcessor(*this);
  mngr->EnableResizing();
  mngr->StartMainLoop();
}

// main function
int main()
{
  setlocale(LC_ALL, "");

  TestApp *app = TestApp::Instance();
  app->Run();

  return 0;
}
