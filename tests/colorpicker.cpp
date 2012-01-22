#include <cppconsui/Button.h>
#include <cppconsui/CoreManager.h>
#include <cppconsui/KeyConfig.h>
#include <cppconsui/ColorPickerDialog.h>
#include <cppconsui/Window.h>

// TestWindow class
class TestWindow
: public CppConsUI::Window
{
  public:
    /* This is a main window, make sure it can not be closed with ESC key by
     * overriding Hide() method. */
    static TestWindow *Instance();
    virtual void Hide() {}

  protected:

  private:
    CppConsUI::Label *label;

    TestWindow();
    virtual ~TestWindow() {}
    TestWindow(const TestWindow&);
    TestWindow& operator=(const TestWindow&);

    void OnButtonActivate(CppConsUI::Button& activator);
};

TestWindow *TestWindow::Instance()
{
  static TestWindow instance;
  return &instance;
}

TestWindow::TestWindow()
: CppConsUI::Window(0, 0, AUTOSIZE, AUTOSIZE)
{
  AddWidget(*(new CppConsUI::Label("Press F10 to quit.")), 1, 1);

  CppConsUI::Button *button = new CppConsUI::Button ("Open Colorpicker...");

  AddWidget(*button, 0, 2);

  button->signal_activate.connect(
      sigc::mem_fun(this, &TestWindow::OnButtonActivate));
}

void TestWindow::OnButtonActivate(CppConsUI::Button& activator)
{
  CppConsUI::ColorPickerDialog *dlg =
      new CppConsUI::ColorPickerDialog("Test Colorpicker", 0);

  dlg->Show();
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
  mngr->AddWindow(*TestWindow::Instance());
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
