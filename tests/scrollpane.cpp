#include <cppconsui/Button.h>
#include <cppconsui/CoreManager.h>
#include <cppconsui/KeyConfig.h>
#include <cppconsui/Label.h>
#include <cppconsui/ScrollPane.h>
#include <cppconsui/Window.h>

#include <stdio.h>

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

// TestWindow class
class TestWindow
: public CppConsUI::Window
{
public:
  TestWindow();
  virtual ~TestWindow() {}

protected:
  MyScrollPane *pane;

private:
  TestWindow(const TestWindow&);
  TestWindow& operator=(const TestWindow&);

  void actionScrollUp();
  void actionScrollDown();
  void actionScrollLeft();
  void actionScrollRight();
};

TestWindow::TestWindow()
: CppConsUI::Window(0, 0, AUTOSIZE, AUTOSIZE)
{
  addWidget(*(new CppConsUI::Label(25, 1, "Press F10 to quit.")), 1, 1);
  addWidget(*(new CppConsUI::Label(25, 1, "WASD to move the picture.")),
      1, 2);

  pane = new MyScrollPane(20, 10, 111, 23);
  addWidget(*pane, 1, 4);

  declareBindable("scrollpanewindow", "scroll-up",
      sigc::mem_fun(this, &TestWindow::actionScrollUp),
      InputProcessor::BINDABLE_NORMAL);
  declareBindable("scrollpanewindow", "scroll-down",
      sigc::mem_fun(this, &TestWindow::actionScrollDown),
      InputProcessor::BINDABLE_NORMAL);
  declareBindable("scrollpanewindow", "scroll-left",
      sigc::mem_fun(this, &TestWindow::actionScrollLeft),
      InputProcessor::BINDABLE_NORMAL);
  declareBindable("scrollpanewindow", "scroll-right",
      sigc::mem_fun(this, &TestWindow::actionScrollRight),
      InputProcessor::BINDABLE_NORMAL);
}

void TestWindow::actionScrollUp()
{
  pane->adjustScroll(pane->getScrollPositionX(),
      pane->getScrollPositionY() - 1);
}

void TestWindow::actionScrollDown()
{
  pane->adjustScroll(pane->getScrollPositionX(),
      pane->getScrollPositionY() + 1);
}

void TestWindow::actionScrollLeft()
{
  pane->adjustScroll(pane->getScrollPositionX() - 1,
      pane->getScrollPositionY());
}

void TestWindow::actionScrollRight()
{
  pane->adjustScroll(pane->getScrollPositionX() + 1,
      pane->getScrollPositionY());
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
  KEYCONFIG->bindKey("scrollpanewindow", "scroll-up", "w");
  KEYCONFIG->bindKey("scrollpanewindow", "scroll-down", "s");
  KEYCONFIG->bindKey("scrollpanewindow", "scroll-left", "a");
  KEYCONFIG->bindKey("scrollpanewindow", "scroll-right", "d");

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
