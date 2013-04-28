#include <cppconsui/Button.h>
#include <cppconsui/CoreManager.h>
#include <cppconsui/KeyConfig.h>
#include <cppconsui/Label.h>
#include <cppconsui/Window.h>

// ButtonWindow class
class ButtonWindow
: public CppConsUI::Window
{
public:
  /* This is a main window, make sure it can not be closed with ESC key by
   * overriding Close() method. */
  static ButtonWindow *instance();
  virtual void close() {}

protected:

private:
  CppConsUI::Label *label;

  ButtonWindow();
  virtual ~ButtonWindow() {}
  ButtonWindow(const ButtonWindow&);
  ButtonWindow& operator=(const ButtonWindow&);

  void onButtonActivate(CppConsUI::Button& activator);
};

ButtonWindow *ButtonWindow::instance()
{
  static ButtonWindow instance;
  return &instance;
}

ButtonWindow::ButtonWindow()
: CppConsUI::Window(0, 0, AUTOSIZE, AUTOSIZE)
{
  addWidget(*(new CppConsUI::Label("Press F10 to quit.")), 1, 1);
  label = new CppConsUI::Label;
  addWidget(*label, 1, 2);

  CppConsUI::Button *button;

  button = new CppConsUI::Button(20, 1, "Normal button");
  button->signal_activate.connect(sigc::mem_fun(this,
        &ButtonWindow::onButtonActivate));
  addWidget(*button, 1, 8);

  button = new CppConsUI::Button("Simple autosize");
  button->signal_activate.connect(sigc::mem_fun(this,
        &ButtonWindow::onButtonActivate));
  addWidget(*button, 1, 10);

  button = new CppConsUI::Button(CppConsUI::Button::FLAG_VALUE,
      "Text+value button", "value");
  button->signal_activate.connect(sigc::mem_fun(this,
        &ButtonWindow::onButtonActivate));
  addWidget(*button, 1, 12);

  button = new CppConsUI::Button(CppConsUI::Button::FLAG_VALUE
      | CppConsUI::Button::FLAG_UNIT, "Text+value+unit button", "value",
      "unit");
  button->signal_activate.connect(sigc::mem_fun(this,
        &ButtonWindow::onButtonActivate));
  addWidget(*button, 1, 14);

  button = new CppConsUI::Button(CppConsUI::Button::FLAG_VALUE
      | CppConsUI::Button::FLAG_UNIT, "Text+value+unit\n2-line button",
      "value", "unit");
  button->signal_activate.connect(sigc::mem_fun(this,
        &ButtonWindow::onButtonActivate));
  addWidget(*button, 1, 16);

  button = new CppConsUI::Button(CppConsUI::Button::FLAG_VALUE
      | CppConsUI::Button::FLAG_UNIT, "Text+value+unit\n3-line\nbutton",
      "value", "unit");
  button->signal_activate.connect(sigc::mem_fun(this,
        &ButtonWindow::onButtonActivate));
  addWidget(*button, 1, 19);

  button = new CppConsUI::Button(CppConsUI::Button::FLAG_VALUE
      | CppConsUI::Button::FLAG_UNIT,
      "Text+value+unit\n4-line\n\nbutton", "value", "unit");
  button->signal_activate.connect(sigc::mem_fun(this,
        &ButtonWindow::onButtonActivate));
  addWidget(*button, 1, 23);

  button = new CppConsUI::Button(30, 1, CppConsUI::Button::FLAG_RIGHT,
      "Text+right button", NULL, NULL, "right");
  button->signal_activate.connect(sigc::mem_fun(this,
        &ButtonWindow::onButtonActivate));
  addWidget(*button, 1, 28);
}

void ButtonWindow::onButtonActivate(CppConsUI::Button& activator)
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
  mngr->addWindow(*ButtonWindow::instance());
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
