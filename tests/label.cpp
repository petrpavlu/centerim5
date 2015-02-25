#include <MainLoop.h>

#include <cppconsui/CoreManager.h>
#include <cppconsui/KeyConfig.h>
#include <cppconsui/Label.h>
#include <cppconsui/Window.h>

#include <iostream>

// TestWindow class
class TestWindow
: public CppConsUI::Window
{
public:
  TestWindow();
  virtual ~TestWindow() {}

protected:

private:
  CONSUI_DISABLE_COPY(TestWindow);
};

TestWindow::TestWindow()
: CppConsUI::Window(0, 0, AUTOSIZE, AUTOSIZE)
{
  setClosable(false);

  CppConsUI::Label *label;

  label = new CppConsUI::Label(
    20, // width
    1, // height
    "Press F10 to quit."); // text
  /* Add label to container, container takes widget ownership and deletes it
   * when necessary. */
  addWidget(*label, 1, 1);

  label = new CppConsUI::Label(20, 1,
      "Too wide string, too wide string, too wide string");
  addWidget(*label, 1, 3);

  label = new CppConsUI::Label(20, 3,
      "Multiline label, multiline label, multiline label");
  addWidget(*label, 1, 5);

  label = new CppConsUI::Label(
      "Auto multiline label,\nauto multiline label,\nauto multiline label");
  addWidget(*label, 1, 9);

  // unicode test
  label = new CppConsUI::Label(30, 3,
      "\x56\xc5\x99\x65\xc5\xa1\x74\xc3\xad\x63\xc3\xad\x20\x70\xc5\x99"
      "\xc3\xad\xc5\xa1\x65\x72\x79\x20\x73\x65\x20\x64\x6f\xc5\xbe\x61"
      "\x64\x6f\x76\x61\x6c\x79\x20\xc3\xba\x70\x6c\x6e\xc4\x9b\x20\xc4"
      "\x8d\x65\x72\x73\x74\x76\xc3\xbd\x63\x68\x20\xc5\x99\xc3\xad\x7a"
      "\x65\xc4\x8d\x6b\xc5\xaf\x2e\x0a");
  addWidget(*label, 1, 13);

  label = new CppConsUI::Label("Autosize");
  addWidget(*label, 1, 17);

  const char *long_text = "Lorem ipsum dolor sit amet, consectetur"
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

  label = new CppConsUI::Label(AUTOSIZE, 10, long_text);
  addWidget(*label, 42, 17);

  label = new CppConsUI::Label(40, AUTOSIZE, long_text);
  addWidget(*label, 1, 28);

  label = new CppConsUI::Label(AUTOSIZE, AUTOSIZE, long_text);
  addWidget(*label, 42, 28);
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
