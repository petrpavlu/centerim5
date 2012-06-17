#include <cppconsui/CoreManager.h>
#include <cppconsui/KeyConfig.h>
#include <cppconsui/Label.h>
#include <cppconsui/Window.h>

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

  gchar *t = g_strdup_printf("Win %d", number);
  label = new CppConsUI::Label(w - 4, 1, t);
  g_free(t);
  AddWidget(*label, 2, 1);

  if (number == 1) {
    label = new CppConsUI::Label("Press F10 to quit.");
    AddWidget(*label, 2, 2);

    label = new CppConsUI::Label("Press ESC to close a focused window.");
    AddWidget(*label, 2, 3);
  }
}

// TestApp class
class TestApp
: public CppConsUI::InputProcessor
{
public:
  static TestApp *Instance();

  void Run();

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
  for (int i = 1; i <= 4; i++) {
    TestWindow *w = new TestWindow(i, (i - 1) % 2 * 40, (i - 1) / 2 * 10, 40,
        10);
    mngr->AddWindow(*w);
  }

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
