#include <MainLoop.h>

#include <cppconsui/ColorScheme.h>
#include <cppconsui/CoreManager.h>
#include <cppconsui/KeyConfig.h>
#include <cppconsui/TextView.h>
#include <cppconsui/Window.h>

#include <iostream>

// TestWindow class
class TestWindow : public CppConsUI::Window {
public:
  TestWindow();
  virtual ~TestWindow() {}

protected:
  CppConsUI::TextView *textview;

private:
  void actionToggleScrollbar();

  CONSUI_DISABLE_COPY(TestWindow);
};

TestWindow::TestWindow() : CppConsUI::Window(0, 0, AUTOSIZE, AUTOSIZE)
{
  setClosable(false);
  setColorScheme("textviewwindow");

  textview = new CppConsUI::TextView(AUTOSIZE, AUTOSIZE);
  addWidget(*textview, 0, 0);

  const char *long_text =
    "Lorem ipsum dolor sit amet, consectetur"
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

  declareBindable("textviewwindow", "toggle-scrollbar",
    sigc::mem_fun(this, &TestWindow::actionToggleScrollbar),
    InputProcessor::BINDABLE_NORMAL);
}

void TestWindow::actionToggleScrollbar()
{
  textview->setScrollBar(!textview->hasScrollBar());
}

// TestApp class
class TestApp : public CppConsUI::InputProcessor {
public:
  static int run();

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
  CppConsUI::AppInterface interface = {MainLoop::timeout_add_cppconsui,
    MainLoop::timeout_remove_cppconsui, MainLoop::input_add_cppconsui,
    MainLoop::input_remove_cppconsui, log_error_cppconsui};
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
  KEYCONFIG->bindKey("textviewwindow", "toggle-scrollbar", "F1");

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

/* vim: set tabstop=2 shiftwidth=2 textwidth=80 expandtab : */
