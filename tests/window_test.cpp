#include <cppconsui/CoreManager.h>
#include <cppconsui/KeyConfig.h>
#include <cppconsui/Keys.h>
#include <cppconsui/Label.h>
#include <cppconsui/Window.h>

// TestWindow class
class TestWindow
: public Window
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
: Window(x, y, w, h)
{
  Label *label;

  gchar *t = g_strdup_printf("Win %d", number);
  label = new Label(w - 4, 1, t);
  g_free(t);
  AddWidget(*label, 2, 1);

  if (number == 1) {
    label = new Label("Press F10 to quit.");
    AddWidget(*label, 2, 2);

    label = new Label("Press ESC to close a focused window.");
    AddWidget(*label, 2, 3);
  }
}

// TestApp class

#define CONTEXT_TESTAPP "testapp"

class TestApp
: public InputProcessor
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
    CoreManager *mngr;

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
: InputProcessor()
{
  mngr = CoreManager::Instance();

  g_log_set_default_handler(g_log_func_, this);

  DeclareBindable(CONTEXT_TESTAPP, "quit", sigc::mem_fun(mngr,
        &CoreManager::QuitMainLoop), InputProcessor::BINDABLE_OVERRIDE);
  KEYCONFIG->RegisterKeyDef(CONTEXT_TESTAPP, "quit",
      Keys::FunctionTermKey(10));
}

void TestApp::Run()
{
  for (int i = 1; i <= 4; i++) {
    Window *w = new TestWindow(i, (i - 1) % 2 * 40, (i - 1) / 2 * 10, 40, 10);
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
