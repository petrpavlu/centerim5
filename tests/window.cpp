#include <cppconsui/CoreManager.h>
#include <cppconsui/KeyConfig.h>
#include <cppconsui/Label.h>
#include <cppconsui/Window.h>

#include <stdio.h>

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
  addWidget(*label, 2, 1);

  if (number == 1) {
    label = new CppConsUI::Label("Press F10 to quit.");
    addWidget(*label, 2, 2);

    label = new CppConsUI::Label("Press ESC to close a focused window.");
    addWidget(*label, 2, 3);
  }
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
  for (int i = 1; i <= 4; i++) {
    TestWindow *win = new TestWindow(i, (i - 1) % 2 * 40, (i - 1) / 2 * 10,
        40, 10);
    win->show();
  }

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
