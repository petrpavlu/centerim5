#include <cppconsui/Button.h>
#include <cppconsui/CoreManager.h>
#include <cppconsui/KeyConfig.h>
#include <cppconsui/Label.h>
#include <cppconsui/MenuWindow.h>
#include <cppconsui/Window.h>

// SubMenuWindow class
class SubMenuWindow
: public CppConsUI::Window
{
public:
  /* This is a main window, make sure it can not be closed with ESC key by
   * overriding Close() method. */
  static SubMenuWindow *instance();
  virtual void close() {}

protected:

private:
  CppConsUI::Label *label;
  CppConsUI::MenuWindow *menu;

  SubMenuWindow();
  virtual ~SubMenuWindow();
  SubMenuWindow(const SubMenuWindow&);
  SubMenuWindow& operator=(const SubMenuWindow&);

  void onButtonActivate(CppConsUI::Button& activator);
};

SubMenuWindow *SubMenuWindow::instance()
{
  static SubMenuWindow instance;
  return &instance;
}

SubMenuWindow::SubMenuWindow()
: CppConsUI::Window(0, 0, AUTOSIZE, AUTOSIZE)
{
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

SubMenuWindow::~SubMenuWindow()
{
  delete menu;
}

void SubMenuWindow::onButtonActivate(CppConsUI::Button& activator)
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
  mngr->addWindow(*SubMenuWindow::instance());
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

/* vim: set tabstop=2 shiftwidth=2 textwidth=78 expandtab : */
