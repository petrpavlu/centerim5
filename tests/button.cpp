#include <cppconsui/Button.h>
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
  TestWindow();
  virtual ~TestWindow() {}

protected:

private:
  CppConsUI::Label *label;

  TestWindow(const TestWindow&);
  TestWindow& operator=(const TestWindow&);

  void onButtonActivate(CppConsUI::Button& activator);
};

TestWindow::TestWindow()
: CppConsUI::Window(0, 0, AUTOSIZE, AUTOSIZE)
{
  setClosable(false);

  addWidget(*(new CppConsUI::Label("Press F10 to quit.")), 1, 1);
  label = new CppConsUI::Label;
  addWidget(*label, 1, 2);

  CppConsUI::Button *button;

  button = new CppConsUI::Button(20, 1, "Normal button");
  button->signal_activate.connect(sigc::mem_fun(this,
        &TestWindow::onButtonActivate));
  addWidget(*button, 1, 8);

  button = new CppConsUI::Button("Simple autosize");
  button->signal_activate.connect(sigc::mem_fun(this,
        &TestWindow::onButtonActivate));
  addWidget(*button, 1, 10);

  button = new CppConsUI::Button(CppConsUI::Button::FLAG_VALUE,
      "Text+value button", "value");
  button->signal_activate.connect(sigc::mem_fun(this,
        &TestWindow::onButtonActivate));
  addWidget(*button, 1, 12);

  button = new CppConsUI::Button(CppConsUI::Button::FLAG_VALUE
      | CppConsUI::Button::FLAG_UNIT, "Text+value+unit button", "value",
      "unit");
  button->signal_activate.connect(sigc::mem_fun(this,
        &TestWindow::onButtonActivate));
  addWidget(*button, 1, 14);

  button = new CppConsUI::Button(CppConsUI::Button::FLAG_VALUE
      | CppConsUI::Button::FLAG_UNIT, "Text+value+unit\n2-line button",
      "value", "unit");
  button->signal_activate.connect(sigc::mem_fun(this,
        &TestWindow::onButtonActivate));
  addWidget(*button, 1, 16);

  button = new CppConsUI::Button(CppConsUI::Button::FLAG_VALUE
      | CppConsUI::Button::FLAG_UNIT, "Text+value+unit\n3-line\nbutton",
      "value", "unit");
  button->signal_activate.connect(sigc::mem_fun(this,
        &TestWindow::onButtonActivate));
  addWidget(*button, 1, 19);

  button = new CppConsUI::Button(CppConsUI::Button::FLAG_VALUE
      | CppConsUI::Button::FLAG_UNIT,
      "Text+value+unit\n4-line\n\nbutton", "value", "unit");
  button->signal_activate.connect(sigc::mem_fun(this,
        &TestWindow::onButtonActivate));
  addWidget(*button, 1, 23);

  button = new CppConsUI::Button(30, 1, CppConsUI::Button::FLAG_RIGHT,
      "Text+right button", NULL, NULL, "right");
  button->signal_activate.connect(sigc::mem_fun(this,
        &TestWindow::onButtonActivate));
  addWidget(*button, 1, 28);
}

void TestWindow::onButtonActivate(CppConsUI::Button& activator)
{
  char *text = g_strdup_printf("%s activated.", activator.getText());;
  label->setText(text);
  g_free(text);
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
