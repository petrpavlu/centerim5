#include <cppconsui/Button.h>
#include <cppconsui/CoreManager.h>
#include <cppconsui/KeyConfig.h>
#include <cppconsui/Label.h>
#include <cppconsui/ScrollPane.h>
#include <cppconsui/Window.h>

static const char *pic[] =
{
" ______                                __          __         __   __                       ",
"|      |.-----.-----.-----.----.---.-.|  |_.--.--.|  |.---.-.|  |_|__|.-----.-----.-----.   ",
"|   ---||  _  |     |  _  |   _|  _  ||   _|  |  ||  ||  _  ||   _|  ||  _  |     |__ --|__ ",
"|______||_____|__|__|___  |__| |___._||____|_____||__||___._||____|__||_____|__|__|_____|  |",
"                    |_____|                                                              |_|",
"                      __               __     __               __     _______ __            ",
".--.--.-----.--.--.  |__|.--.--.-----.|  |_  |  |.-----.-----.|  |_  |_     _|  |--.-----.  ",
"|  |  |  _  |  |  |  |  ||  |  |__ --||   _| |  ||  _  |__ --||   _|   |   | |     |  -__|  ",
"|___  |_____|_____|  |  ||_____|_____||____| |__||_____|_____||____|   |___| |__|__|_____|  ",
"|_____|             |___|                                                                   ",
" _______                                                                                    ",
"|     __|.---.-.--------.-----.                                                             ",
"|    |  ||  _  |        |  -__|__                                                           ",
"|_______||___._|__|__|__|_____|__|                                                          "
};


// MyScrollPane class
class MyScrollPane
: public CppConsUI::ScrollPane
{
public:
  MyScrollPane(int w, int h, int scrollw, int scrollh);
  virtual ~MyScrollPane() {}

  // Widget
  virtual void draw();

protected:

private:
  MyScrollPane(const MyScrollPane&);
  MyScrollPane& operator=(const MyScrollPane&);
};

MyScrollPane::MyScrollPane(int w, int h, int scrollw, int scrollh)
: CppConsUI::ScrollPane(w, h, scrollw, scrollh)
{
}

void MyScrollPane::draw()
{
  proceedUpdateArea();
  proceedUpdateVirtualArea();

  if (!area) {
    // scrollpane will clear the scroll (real) area
    ScrollPane::draw();
    return;
  }

  area->fill(getColorPair("container", "background"));

  int real_height = area->getmaxy();
  for (int i = 0; i < real_height && i < (int) (sizeof(pic) / sizeof(pic[0]));
      i++)
    area->mvaddstring(0, i, pic[i]);

  ScrollPane::drawEx(false);
}

// ScrollPaneWindow class
class ScrollPaneWindow
: public CppConsUI::Window
{
public:
  /* This is a main window, make sure it can not be closed with ESC key by
   * overriding Close() method. */
  static ScrollPaneWindow *instance();
  virtual void close() {}

protected:
  MyScrollPane *pane;

private:
  ScrollPaneWindow();
  virtual ~ScrollPaneWindow() {}
  ScrollPaneWindow(const ScrollPaneWindow&);
  ScrollPaneWindow& operator=(const ScrollPaneWindow&);

  void actionScrollUp();
  void actionScrollDown();
  void actionScrollLeft();
  void actionScrollRight();
};

ScrollPaneWindow *ScrollPaneWindow::instance()
{
  static ScrollPaneWindow instance;
  return &instance;
}

ScrollPaneWindow::ScrollPaneWindow()
: CppConsUI::Window(0, 0, AUTOSIZE, AUTOSIZE)
{
  addWidget(*(new CppConsUI::Label(25, 1, "Press F10 to quit.")), 1, 1);
  addWidget(*(new CppConsUI::Label(25, 1, "WASD to move the picture.")),
      1, 2);

  pane = new MyScrollPane(20, 10, 111, 23);
  addWidget(*pane, 1, 4);

  declareBindable("scrollpanewindow", "scroll-up",
      sigc::mem_fun(this, &ScrollPaneWindow::actionScrollUp),
      InputProcessor::BINDABLE_NORMAL);
  declareBindable("scrollpanewindow", "scroll-down",
      sigc::mem_fun(this, &ScrollPaneWindow::actionScrollDown),
      InputProcessor::BINDABLE_NORMAL);
  declareBindable("scrollpanewindow", "scroll-left",
      sigc::mem_fun(this, &ScrollPaneWindow::actionScrollLeft),
      InputProcessor::BINDABLE_NORMAL);
  declareBindable("scrollpanewindow", "scroll-right",
      sigc::mem_fun(this, &ScrollPaneWindow::actionScrollRight),
      InputProcessor::BINDABLE_NORMAL);
}

void ScrollPaneWindow::actionScrollUp()
{
  pane->adjustScroll(pane->getScrollPositionX(),
      pane->getScrollPositionY() - 1);
}

void ScrollPaneWindow::actionScrollDown()
{
  pane->adjustScroll(pane->getScrollPositionX(),
      pane->getScrollPositionY() + 1);
}

void ScrollPaneWindow::actionScrollLeft()
{
  pane->adjustScroll(pane->getScrollPositionX() - 1,
      pane->getScrollPositionY());
}

void ScrollPaneWindow::actionScrollRight()
{
  pane->adjustScroll(pane->getScrollPositionX() + 1,
      pane->getScrollPositionY());
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
  KEYCONFIG->bindKey("scrollpanewindow", "scroll-up", "w");
  KEYCONFIG->bindKey("scrollpanewindow", "scroll-down", "s");
  KEYCONFIG->bindKey("scrollpanewindow", "scroll-left", "a");
  KEYCONFIG->bindKey("scrollpanewindow", "scroll-right", "d");

  g_log_set_default_handler(g_log_func_, this);

  declareBindable("testapp", "quit", sigc::mem_fun(mngr,
        &CppConsUI::CoreManager::quitMainLoop),
      InputProcessor::BINDABLE_OVERRIDE);
}

void TestApp::run()
{
  mngr->addWindow(*ScrollPaneWindow::instance());
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
