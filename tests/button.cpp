#include <cppconsui/Button.h>
#include <cppconsui/CoreManager.h>
#include <cppconsui/KeyConfig.h>
#include <cppconsui/Window.h>

// ButtonWindow class
class ButtonWindow
: public CppConsUI::Window
{
public:
  /* This is a main window, make sure it can not be closed with ESC key by
   * overriding Close() method. */
  static ButtonWindow *Instance();
  virtual void Close() {}

protected:

private:
  CppConsUI::Label *label;

  ButtonWindow();
  virtual ~ButtonWindow() {}
  ButtonWindow(const ButtonWindow&);
  ButtonWindow& operator=(const ButtonWindow&);

  void OnButtonActivate(CppConsUI::Button& activator);
};

ButtonWindow *ButtonWindow::Instance()
{
  static ButtonWindow instance;
  return &instance;
}

ButtonWindow::ButtonWindow()
: CppConsUI::Window(0, 0, AUTOSIZE, AUTOSIZE)
{
  AddWidget(*(new CppConsUI::Label("Press F10 to quit.")), 1, 1);
  label = new CppConsUI::Label;
  AddWidget(*label, 1, 2);

  CppConsUI::Button *button;

  button = new CppConsUI::Button(20, 1, "Normal button");
  button->signal_activate.connect(sigc::mem_fun(this,
        &ButtonWindow::OnButtonActivate));
  AddWidget(*button, 1, 8);

  button = new CppConsUI::Button("Simple autosize");
  button->signal_activate.connect(sigc::mem_fun(this,
        &ButtonWindow::OnButtonActivate));
  AddWidget(*button, 1, 10);

  button = new CppConsUI::Button(CppConsUI::Button::FLAG_VALUE,
      "Text+value button", "value");
  button->signal_activate.connect(sigc::mem_fun(this,
        &ButtonWindow::OnButtonActivate));
  AddWidget(*button, 1, 12);

  button = new CppConsUI::Button(CppConsUI::Button::FLAG_VALUE
      | CppConsUI::Button::FLAG_UNIT, "Text+value+unit button", "value",
      "unit");
  button->signal_activate.connect(sigc::mem_fun(this,
        &ButtonWindow::OnButtonActivate));
  AddWidget(*button, 1, 14);

  button = new CppConsUI::Button(CppConsUI::Button::FLAG_VALUE
      | CppConsUI::Button::FLAG_UNIT, "Text+value+unit\n2-line button",
      "value", "unit");
  button->signal_activate.connect(sigc::mem_fun(this,
        &ButtonWindow::OnButtonActivate));
  AddWidget(*button, 1, 16);

  button = new CppConsUI::Button(CppConsUI::Button::FLAG_VALUE
      | CppConsUI::Button::FLAG_UNIT, "Text+value+unit\n3-line\nbutton",
      "value", "unit");
  button->signal_activate.connect(sigc::mem_fun(this,
        &ButtonWindow::OnButtonActivate));
  AddWidget(*button, 1, 19);

  button = new CppConsUI::Button(CppConsUI::Button::FLAG_VALUE
      | CppConsUI::Button::FLAG_UNIT,
      "Text+value+unit\n4-line\n\nbutton", "value", "unit");
  button->signal_activate.connect(sigc::mem_fun(this,
        &ButtonWindow::OnButtonActivate));
  AddWidget(*button, 1, 23);

  button = new CppConsUI::Button(30, 1, CppConsUI::Button::FLAG_RIGHT,
      "Text+right button", NULL, NULL, "right");
  button->signal_activate.connect(sigc::mem_fun(this,
        &ButtonWindow::OnButtonActivate));
  AddWidget(*button, 1, 28);
}

void ButtonWindow::OnButtonActivate(CppConsUI::Button& activator)
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
  mngr->AddWindow(*ButtonWindow::Instance());
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
