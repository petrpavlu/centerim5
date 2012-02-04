#include <cppconsui/Button.h>
#include <cppconsui/CoreManager.h>
#include <cppconsui/KeyConfig.h>
#include <cppconsui/MenuWindow.h>
#include <cppconsui/Window.h>

// SubMenuWindow class
class SubMenuWindow
: public CppConsUI::Window
{
  public:
    /* This is a main window, make sure it can not be closed with ESC key by
     * overriding Close() method. */
    static SubMenuWindow *Instance();
    virtual void Close() {}

  protected:

  private:
    CppConsUI::Label *label;
    CppConsUI::MenuWindow *menu;

    SubMenuWindow();
    virtual ~SubMenuWindow();
    SubMenuWindow(const SubMenuWindow&);
    SubMenuWindow& operator=(const SubMenuWindow&);

    void OnButtonActivate(CppConsUI::Button& activator);
};

SubMenuWindow *SubMenuWindow::Instance()
{
  static SubMenuWindow instance;
  return &instance;
}

SubMenuWindow::SubMenuWindow()
: CppConsUI::Window(0, 0, AUTOSIZE, AUTOSIZE)
{
  AddWidget(*(new CppConsUI::Label("Press F10 to quit.")), 1, 1);

  CppConsUI::Button *button = new CppConsUI::Button("Open Menu...");
  AddWidget(*button, 1, 3);

  menu = new CppConsUI::MenuWindow(*button, AUTOSIZE, AUTOSIZE);
  menu->SetHideOnClose(true);

  button->signal_activate.connect(sigc::hide(sigc::mem_fun(menu,
          &CppConsUI::MenuWindow::Show)));

  menu->AppendItem("Item 1", sigc::hide(sigc::mem_fun(menu,
          &CppConsUI::MenuWindow::Hide)));
  menu->AppendItem("Item 2", sigc::hide(sigc::mem_fun(menu,
          &CppConsUI::MenuWindow::Hide)));
  menu->AppendItem("Item 3", sigc::hide(sigc::mem_fun(menu,
          &CppConsUI::MenuWindow::Hide)));
  menu->AppendItem("Item 4", sigc::hide(sigc::mem_fun(menu,
          &CppConsUI::MenuWindow::Hide)));
  menu->AppendSeparator();

  CppConsUI::MenuWindow *submenu = new CppConsUI::MenuWindow(*button,
      AUTOSIZE, AUTOSIZE);
  submenu->AppendItem("Item 1", sigc::hide(sigc::mem_fun(submenu,
          &CppConsUI::MenuWindow::Hide)));
  submenu->AppendItem("Item 2", sigc::hide(sigc::mem_fun(submenu,
          &CppConsUI::MenuWindow::Hide)));
  submenu->AppendItem("Item 3", sigc::hide(sigc::mem_fun(submenu,
          &CppConsUI::MenuWindow::Hide)));
  submenu->AppendItem("Item 3", sigc::hide(sigc::mem_fun(submenu,
          &CppConsUI::MenuWindow::Hide)));
  menu->AppendSubMenu("First submenu", *submenu);

  submenu = new CppConsUI::MenuWindow(*button, AUTOSIZE, AUTOSIZE);
  submenu->AppendItem("Item 1", sigc::hide(sigc::mem_fun(menu,
          &CppConsUI::MenuWindow::Hide)));
  submenu->AppendItem("Item 2", sigc::hide(sigc::mem_fun(menu,
          &CppConsUI::MenuWindow::Hide)));
  submenu->AppendItem("Item 3", sigc::hide(sigc::mem_fun(menu,
          &CppConsUI::MenuWindow::Hide)));
  submenu->AppendItem("Item 4", sigc::hide(sigc::mem_fun(menu,
          &CppConsUI::MenuWindow::Hide)));
  menu->AppendSubMenu("Second submenu", *submenu);
}

SubMenuWindow::~SubMenuWindow()
{
  delete menu;
}

void SubMenuWindow::OnButtonActivate(CppConsUI::Button& activator)
{
  char *text = g_strdup_printf("%s activated.", activator.GetText());;
  label->SetText(text);
  g_free(text);
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
  KEYCONFIG->BindKey("testapp", "quit", "F10");
  KEYCONFIG->LoadDefaultKeyConfig();

  g_log_set_default_handler(g_log_func_, this);

  DeclareBindable("testapp", "quit", sigc::mem_fun(mngr,
        &CppConsUI::CoreManager::QuitMainLoop),
      InputProcessor::BINDABLE_OVERRIDE);
}

void TestApp::Run()
{
  mngr->AddWindow(*SubMenuWindow::Instance());
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
