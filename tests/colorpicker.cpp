#include <MainLoop.h>

#include <cppconsui/Button.h>
#include <cppconsui/ConsUICurses.h>
#include <cppconsui/CoreManager.h>
#include <cppconsui/ColorPicker.h>
#include <cppconsui/ColorPickerComboBox.h>
#include <cppconsui/ColorPickerDialog.h>
#include <cppconsui/KeyConfig.h>
#include <cppconsui/Window.h>

#include <iostream>
#include <sstream>
#include <string>

// TestWindow class
class TestWindow
: public CppConsUI::Window
{
public:
  TestWindow();
  virtual ~TestWindow() {}

protected:

private:
  CppConsUI::Label *label1;
  CppConsUI::Label *label2;
  int defaultcolor;
  CppConsUI::ColorPickerComboBox *combo;

  TestWindow(const TestWindow&);
  TestWindow& operator=(const TestWindow&);

  void onButtonActivate(CppConsUI::Button& activator, int flags);
  void onChangeColorResponseHandler(CppConsUI::ColorPickerDialog& activator,
      CppConsUI::AbstractDialog::ResponseType response, int color);
  void onColerPickerChanged(CppConsUI::ColorPicker& activator, int new_fg,
      int new_bg);
  void onComboColorChange(CppConsUI::ComboBox& activator, intptr_t color);
};

TestWindow::TestWindow()
: CppConsUI::Window(0, 0, AUTOSIZE, AUTOSIZE), defaultcolor(0)
{
  setClosable(false);

  CppConsUI::Button *button;

  addWidget(*(new CppConsUI::Label("Press F10 to quit.")), 1, 1);

  button = new CppConsUI::Button("Open Colorpicker...");
  addWidget(*button, 1, 3);
  button->signal_activate.connect(sigc::bind(sigc::mem_fun(this,
          &TestWindow::onButtonActivate), 0));

  button = new CppConsUI::Button("Open Colorpicker: ansi only...");
  addWidget(*button, 1, 4);
  button->signal_activate.connect(sigc::bind(sigc::mem_fun(this,
          &TestWindow::onButtonActivate),
        CppConsUI::ColorPickerPalette::FLAG_HIDE_GRAYSCALE
        | CppConsUI::ColorPickerPalette::FLAG_HIDE_COLORCUBE));

  button = new CppConsUI::Button("Open Colorpicker: ansi + Grayscale...");
  addWidget(*button, 1, 5);
  button->signal_activate.connect(sigc::bind(sigc::mem_fun(this,
          &TestWindow::onButtonActivate),
        CppConsUI::ColorPickerPalette::FLAG_HIDE_COLORCUBE));

  button = new CppConsUI::Button("Open Colorpicker: color cube only...");
  addWidget(*button, 1, 6);
  button->signal_activate.connect(sigc::bind(sigc::mem_fun(this,
          &TestWindow::onButtonActivate),
        CppConsUI::ColorPickerPalette::FLAG_HIDE_ANSI
        | CppConsUI::ColorPickerPalette::FLAG_HIDE_GRAYSCALE));

  std::string text = std::string("Supported nr of colors: ")
    + dynamic_cast<std::ostringstream*>(
        &(std::ostringstream() << CppConsUI::Curses::nrcolors()))->str();
  label1 = new CppConsUI::Label(text.c_str());
  addWidget(*label1, 1, 8);
  label2 = new CppConsUI::Label("...");
  addWidget(*label2, 1, 10);

  CppConsUI::Label *l = new CppConsUI::Label("ColorPickerComboBox:");
  addWidget(*l, 1, 12);

  l = new CppConsUI::Label;
  text = std::string("Supported nr of color pairs: ")
    + dynamic_cast<std::ostringstream*>(
        &(std::ostringstream() << CppConsUI::Curses::nrcolorpairs()))->str();
  l->setText(text.c_str());
  addWidget(*l, 1, 9);

  combo = new CppConsUI::ColorPickerComboBox (10, defaultcolor);
  combo->signal_color_changed.connect(sigc::mem_fun(this,
        &TestWindow::onComboColorChange));
  addWidget(*combo, 1, 13);

  CppConsUI::ColorPicker *picker;

  addWidget(*(new CppConsUI::Label("ColorPicker:")), 1, 15);
  picker = new CppConsUI::ColorPicker(7, 1, "Label:", false);
  picker->signal_colorpair_selected.connect(sigc::mem_fun(this,
        &TestWindow::onColerPickerChanged));
  addWidget(*picker, 1, 16);

  addWidget(*(new CppConsUI::Label("ColorPicker:")), 1, 18);
  picker = new CppConsUI::ColorPicker(15, 8, "(with sample)", true);
  picker->signal_colorpair_selected.connect(sigc::mem_fun(this,
        &TestWindow::onColerPickerChanged));
  addWidget(*picker, 1, 19);
}

void TestWindow::onButtonActivate(CppConsUI::Button& /*activator*/, int flags)
{
  CppConsUI::ColorPickerDialog *dialog = new CppConsUI::ColorPickerDialog(
      "Test Colorpicker", 0, flags);
  dialog->signal_response.connect(sigc::mem_fun(this,
        &TestWindow::onChangeColorResponseHandler));
  dialog->show();
}

void TestWindow::onColerPickerChanged(CppConsUI::ColorPicker& /*activator*/,
    int new_fg, int new_bg)
{
  std::string text = std::string("Chosen color (")
    + dynamic_cast<std::ostringstream*>(
        &(std::ostringstream() << new_fg))->str()
    + ","
    + dynamic_cast<std::ostringstream*>(
        &(std::ostringstream() << new_bg))->str()
    + ")";
  label2->setText(text.c_str());

  combo->setColor(new_fg);
}

void TestWindow::onChangeColorResponseHandler(
    CppConsUI::ColorPickerDialog& /*activator*/,
    CppConsUI::AbstractDialog::ResponseType response,
    int color)
{
  if (response != CppConsUI::AbstractDialog::RESPONSE_OK)
    return;

  combo->setColor(color);

  std::string text = std::string("Chosen color nr: ")
    + dynamic_cast<std::ostringstream*>(
        &(std::ostringstream() << color))->str();
  label2->setText(text.c_str());
}

void TestWindow::onComboColorChange(CppConsUI::ComboBox& /*activator*/,
    intptr_t color)
{
  std::string text = std::string("Chosen color nr: ")
    + dynamic_cast<std::ostringstream*>(
        &(std::ostringstream() << static_cast<int>(color)))->str();
  label2->setText(text.c_str());
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
