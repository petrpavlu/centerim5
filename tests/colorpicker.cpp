#include <cppconsui/Button.h>
#include <cppconsui/CoreManager.h>
#include <cppconsui/KeyConfig.h>
#include <cppconsui/ColorPickerDialog.h>
#include <cppconsui/ColorPicker.h>
#include <cppconsui/ColorPickerComboBox.h>
#include <cppconsui/ConsuiCurses.h>
#include <cppconsui/Window.h>

// TestWindow class
class TestWindow
: public CppConsUI::Window
{
  public:
    /* This is a main window, make sure it can not be closed with ESC key by
     * overriding Close() method. */
    static TestWindow *Instance();
    virtual void Close() {}

  protected:

  private:
    CppConsUI::Label *label1;
    CppConsUI::Label *label2;
    int defaultcolor;
    CppConsUI::ColorPickerComboBox *combo;

    TestWindow();
    virtual ~TestWindow();
    TestWindow(const TestWindow&);
    TestWindow& operator=(const TestWindow&);

    void OnButtonActivate(CppConsUI::Button& activator, int flags);
    void OnChangeColorResponseHandler(
        CppConsUI::ColorPickerDialog& activator,
        CppConsUI::AbstractDialog::ResponseType response,
        int color);
    void OnColerPickerChanged(CppConsUI::ColorPicker& activator,
        int new_fg, int new_bg);
    void OnComboColorChange(
        CppConsUI::ComboBox& activator,
        intptr_t color);

};

TestWindow *TestWindow::Instance()
{
  static TestWindow instance;
  return &instance;
}

TestWindow::TestWindow()
: CppConsUI::Window(0, 0, AUTOSIZE, AUTOSIZE)
, defaultcolor(0)
{
  CppConsUI::Button *button;

  AddWidget(*(new CppConsUI::Label("Press F10 to quit.")), 1, 1);

  button = new CppConsUI::Button("Open Colorpicker...");
  AddWidget(*button, 1, 3);
  button->signal_activate.connect(
      sigc::bind(sigc::mem_fun(this, &TestWindow::OnButtonActivate), 0));

  button = new CppConsUI::Button("Open Colorpicker: ansi only...");
  AddWidget(*button, 1, 4);
  button->signal_activate.connect(
      sigc::bind(sigc::mem_fun(this, &TestWindow::OnButtonActivate),
          CppConsUI::ColorPickerPalette::FLAG_HIDE_GRAYSCALE
          | CppConsUI::ColorPickerPalette::FLAG_HIDE_COLORCUBE));

  button = new CppConsUI::Button("Open Colorpicker: ansi + Grayscale...");
  AddWidget(*button, 1, 5);
  button->signal_activate.connect(
      sigc::bind(sigc::mem_fun(this, &TestWindow::OnButtonActivate),
          CppConsUI::ColorPickerPalette::FLAG_HIDE_COLORCUBE));

  button = new CppConsUI::Button("Open Colorpicker: color cube only...");
  AddWidget(*button, 1, 6);
  button->signal_activate.connect(
      sigc::bind(sigc::mem_fun(this, &TestWindow::OnButtonActivate),
          CppConsUI::ColorPickerPalette::FLAG_HIDE_ANSI
          | CppConsUI::ColorPickerPalette::FLAG_HIDE_GRAYSCALE));

  label1 = new CppConsUI::Label; AddWidget(*label1, 1, 8);
  label2 = new CppConsUI::Label; AddWidget(*label2, 1, 10);

  label2->SetText("...");

  char *text = g_strdup_printf("Supported nr of colors: %d",  
      CppConsUI::Curses::nrcolors());
  label1->SetText(text);
  g_free(text);

  CppConsUI::Label *l = new CppConsUI::Label("ColorPickerComboBox:");
  AddWidget(*l, 1, 12);

  l = new CppConsUI::Label();
  text = g_strdup_printf("Supported nr of color pairs: %d",  
      CppConsUI::Curses::nrcolorpairs());
  l->SetText(text);
  g_free(text);
  AddWidget(*l, 1, 9);

  combo = new CppConsUI::ColorPickerComboBox (10, defaultcolor);
  combo->signal_color_changed.connect(
      sigc::mem_fun(this, &TestWindow::OnComboColorChange));
  AddWidget(*combo, 1, 13);

  CppConsUI::ColorPicker *picker;

  AddWidget(*(new CppConsUI::Label("ColorPicker: ")), 1, 15);
  AddWidget(*(picker = new CppConsUI::ColorPicker(7, 1, true)), 1, 16);
  picker->signal_colorpair_selected.connect(sigc::mem_fun(this,
      &TestWindow::OnColerPickerChanged));

  AddWidget(*(new CppConsUI::Label("ColorPicker (with sample): ")), 1, 18);
  AddWidget(*(picker = new CppConsUI::ColorPicker(15, 8, true)), 1, 19);
  picker->signal_colorpair_selected.connect(sigc::mem_fun(this,
      &TestWindow::OnColerPickerChanged));
}

TestWindow::~TestWindow()
{
}

void TestWindow::OnButtonActivate(CppConsUI::Button& activator, int flags)
{
  CppConsUI::ColorPickerDialog *dlg =
      new CppConsUI::ColorPickerDialog("Test Colorpicker", 0, flags);

  dlg->signal_response.connect(sigc::mem_fun(this,
      &TestWindow::OnChangeColorResponseHandler));

  dlg->Show();
}

void TestWindow::OnColerPickerChanged(CppConsUI::ColorPicker& activator,
    int new_fg, int new_bg)
{
  char *text = g_strdup_printf("Chosen color (%d,%d)", new_fg, new_bg);
  label2->SetText(text);
  g_free(text);

  combo->SetColor(new_fg);
}

void TestWindow::OnChangeColorResponseHandler(
    CppConsUI::ColorPickerDialog& activator,
    CppConsUI::AbstractDialog::ResponseType response,
    int color)
{
  if (response != CppConsUI::AbstractDialog::RESPONSE_OK)
      return;

  combo->SetColor(color);

  char *text = g_strdup_printf("Chosen color nr: %d", color);
  label2->SetText(text);
  g_free(text);

}

void TestWindow::OnComboColorChange(
    CppConsUI::ComboBox& activator,
    intptr_t color)
{
  char *text = g_strdup_printf("Chosen color nr: %d", (int)color);
  label2->SetText(text);
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
  mngr->AddWindow(*TestWindow::Instance());
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
