#include <cppconsui/Button.h>
#include <cppconsui/CoreManager.h>
#include <cppconsui/KeyConfig.h>
#include <cppconsui/Label.h>
#include <cppconsui/MenuWindow.h>
#include <cppconsui/Window.h>

#include <stdio.h>

// TestWindow class
class TestWindow
: public CppConsUI::Window
{
public:
  TestWindow();
  virtual ~TestWindow();

protected:

private:
  CppConsUI::Label *label;
  CppConsUI::MenuWindow *menu;

  TestWindow(const TestWindow&);
  TestWindow& operator=(const TestWindow&);

  void onButtonActivate(CppConsUI::Button& activator);
};

TestWindow::TestWindow()
: CppConsUI::Window(0, 0, AUTOSIZE, AUTOSIZE)
{
  setClosable(false);

  addWidget(*(new CppConsUI::Label("Press F10 to quit.")), 1, 1);

  CppConsUI::Button *button = new CppConsUI::Button("Open Menu...");
  addWidget(*button, 1, 3);

  menu = new CppConsUI::MenuWindow(*button, AUTOSIZE, AUTOSIZE);
  menu->setHideOnClose(true);

  button->signal_activate.connect(sigc::hide(sigc::mem_fun(menu,
          &CppConsUI::MenuWindow::show)));

  menu->appendItem("Item 1", sigc::hide(sigc::mem_fun(menu,
          &CppConsUI::MenuWindow::hide)));
  menu->appendItem("Item 2", sigc::hide(sigc::mem_fun(menu,
          &CppConsUI::MenuWindow::hide)));
  menu->appendItem("Item 3", sigc::hide(sigc::mem_fun(menu,
          &CppConsUI::MenuWindow::hide)));
  menu->appendItem("Item 4", sigc::hide(sigc::mem_fun(menu,
          &CppConsUI::MenuWindow::hide)));
  menu->appendSeparator();

  CppConsUI::MenuWindow *submenu = new CppConsUI::MenuWindow(*button,
      AUTOSIZE, AUTOSIZE);
  submenu->appendItem("Item 1", sigc::hide(sigc::mem_fun(submenu,
          &CppConsUI::MenuWindow::hide)));
  submenu->appendItem("Item 2", sigc::hide(sigc::mem_fun(submenu,
          &CppConsUI::MenuWindow::hide)));
  submenu->appendItem("Item 3", sigc::hide(sigc::mem_fun(submenu,
          &CppConsUI::MenuWindow::hide)));
  submenu->appendItem("Item 3", sigc::hide(sigc::mem_fun(submenu,
          &CppConsUI::MenuWindow::hide)));
  menu->appendSubMenu("First submenu", *submenu);

  submenu = new CppConsUI::MenuWindow(*button, AUTOSIZE, AUTOSIZE);
  submenu->appendItem("Item 1", sigc::hide(sigc::mem_fun(menu,
          &CppConsUI::MenuWindow::hide)));
  submenu->appendItem("Item 2", sigc::hide(sigc::mem_fun(menu,
          &CppConsUI::MenuWindow::hide)));
  submenu->appendItem("Item 3", sigc::hide(sigc::mem_fun(menu,
          &CppConsUI::MenuWindow::hide)));
  submenu->appendItem("Item 4", sigc::hide(sigc::mem_fun(menu,
          &CppConsUI::MenuWindow::hide)));
  menu->appendSubMenu("Second submenu", *submenu);
}

TestWindow::~TestWindow()
{
  delete menu;
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
