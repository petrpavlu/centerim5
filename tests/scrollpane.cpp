#include <MainLoop.h>

#include <cppconsui/Button.h>
#include <cppconsui/CoreManager.h>
#include <cppconsui/KeyConfig.h>
#include <cppconsui/Label.h>
#include <cppconsui/ScrollPane.h>
#include <cppconsui/Window.h>

#include <iostream>

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
  setClosable(false);

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
  static int run();

protected:

private:
  static TestApp *my_instance;

  static void log_error_cppconsui(const char *message);

  TestApp() {}
  virtual ~TestApp() {}
  int runAll();

  CONSUI_DISABLE_COPY(TestApp);
};

TestApp *TestApp::my_instance = NULL;

int TestApp::run()
{
  // init my instance
  assert(!my_instance);
  my_instance = new TestApp;

  // run the program
  int res = my_instance->runAll();

  // finalize my instance
  assert(my_instance);

  delete my_instance;
  my_instance = NULL;

  return res;
}

void TestApp::log_error_cppconsui(const char * /*message*/)
{
  // ignore all messages
}

int TestApp::runAll()
{
  int res = 1;
  bool mainloop_initialized = false;
  bool cppconsui_initialized = false;
  TestWindow *win;

  // init locale support
  setlocale(LC_ALL, "");

  // init mainloop
  MainLoop::init();
  mainloop_initialized = true;

  // initialize CppConsUI
  CppConsUI::AppInterface interface = {
    MainLoop::timeout_add_cppconsui,
    MainLoop::timeout_remove_cppconsui,
    MainLoop::input_add_cppconsui,
    MainLoop::input_remove_cppconsui,
    log_error_cppconsui
  };
  int consui_res = CppConsUI::initializeConsUI(interface);
  if (consui_res) {
    std::cerr << "CppConsUI initialization failed." << std::endl;
    goto out;
  }
  cppconsui_initialized = true;

  // declare local bindables
  declareBindable("testapp", "quit", sigc::ptr_fun(MainLoop::quit),
      InputProcessor::BINDABLE_OVERRIDE);

  // create the main window
  win = new TestWindow;
  win->show();

  // setup key binds
  KEYCONFIG->loadDefaultKeyConfig();
  KEYCONFIG->bindKey("testapp", "quit", "F10");
  KEYCONFIG->bindKey("scrollpanewindow", "scroll-up", "w");
  KEYCONFIG->bindKey("scrollpanewindow", "scroll-down", "s");
  KEYCONFIG->bindKey("scrollpanewindow", "scroll-left", "a");
  KEYCONFIG->bindKey("scrollpanewindow", "scroll-right", "d");

  // run the main loop
  COREMANAGER->setTopInputProcessor(*this);
  COREMANAGER->enableResizing();
  MainLoop::run();

  // everything went ok
  res = 0;

out:
  // finalize CppConsUI
  if (cppconsui_initialized) {
    if (CppConsUI::finalizeConsUI())
      std::cerr << "CppConsUI finalization failed." << std::endl;
  }

  // finalize mainloop
  if (mainloop_initialized)
    MainLoop::finalize();

  return res;
}

// main function
int main()
{
  return TestApp::run();
}

/* vim: set tabstop=2 shiftwidth=2 textwidth=78 expandtab : */
