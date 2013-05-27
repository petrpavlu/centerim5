#include <cppconsui/ColorScheme.h>
#include <cppconsui/CoreManager.h>
#include <cppconsui/KeyConfig.h>
#include <cppconsui/TextView.h>
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
  CppConsUI::TextView *textview;

private:
  TestWindow(const TestWindow&);
  TestWindow& operator=(const TestWindow&);

  void actionToggleScrollbar();
};

TestWindow::TestWindow()
: CppConsUI::Window(0, 0, AUTOSIZE, AUTOSIZE)
{
  setClosable(false);
  setColorScheme("textviewwindow");

  textview = new CppConsUI::TextView(AUTOSIZE, AUTOSIZE);
  addWidget(*textview, 0, 0);

  const gchar *long_text = "Lorem ipsum dolor sit amet, consectetur"
    "adipiscing elit. Duis dui dui, interdum eget tempor auctor, viverra"
    "suscipit velit. Phasellus vel magna odio. Duis rutrum tortor at nisi"
    "auctor tincidunt. Mauris libero neque, faucibus sit amet semper in, "
    "dictum ut tortor. Duis lacinia justo non lorem blandit ultrices."
    "Nullam vel purus erat, eget aliquam massa. Aenean eget mi a nunc"
    "lacinia consectetur sed a neque. Cras varius, dolor nec rhoncus"
    "ultricies, leo ipsum adipiscing mi, vel feugiat ipsum urna id "
    "metus. Cras non pulvinar nisi. Vivamus nisi lorem, tempor tristique"
    "cursus sit amet, ultricies interdum metus. Nullam tortor tortor, "
    "iaculis sed tempor non, tincidunt ac mi. Quisque id diam vitae diam"
    "dictum facilisis eget ac lacus. Vivamus at gravida felis. Curabitur"
    "fermentum mattis eros, ut auctor urna tincidunt vitae. Praesent"
    "tincidunt laoreet lobortis.";
  for (int i = 0; i < 128; i++)
    textview->append(long_text, i % 7 + 1);

  COLORSCHEME->setColorPair("textviewwindow", "textview", "color1",
      CppConsUI::Curses::Color::RED, CppConsUI::Curses::Color::BLACK);
  COLORSCHEME->setColorPair("textviewwindow", "textview", "color2",
      CppConsUI::Curses::Color::GREEN, CppConsUI::Curses::Color::BLACK);
  COLORSCHEME->setColorPair("textviewwindow", "textview", "color3",
      CppConsUI::Curses::Color::YELLOW, CppConsUI::Curses::Color::BLACK);
  COLORSCHEME->setColorPair("textviewwindow", "textview", "color4",
      CppConsUI::Curses::Color::BLUE, CppConsUI::Curses::Color::BLACK);
  COLORSCHEME->setColorPair("textviewwindow", "textview", "color5",
      CppConsUI::Curses::Color::MAGENTA, CppConsUI::Curses::Color::BLACK);
  COLORSCHEME->setColorPair("textviewwindow", "textview", "color6",
      CppConsUI::Curses::Color::CYAN, CppConsUI::Curses::Color::BLACK);
  COLORSCHEME->setColorPair("textviewwindow", "textview", "color7",
      CppConsUI::Curses::Color::WHITE, CppConsUI::Curses::Color::BLACK);

  declareBindable("textviewwindow", "toggle-scrollbar", sigc::mem_fun(this,
        &TestWindow::actionToggleScrollbar),
      InputProcessor::BINDABLE_NORMAL);
}

void TestWindow::actionToggleScrollbar()
{
  textview->setScrollBar(!textview->hasScrollBar());
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
  KEYCONFIG->bindKey("textviewwindow", "toggle-scrollbar", "F1");

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
