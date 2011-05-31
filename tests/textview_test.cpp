#include <cppconsui/ColorScheme.h>
#include <cppconsui/CoreManager.h>
#include <cppconsui/KeyConfig.h>
#include <cppconsui/Keys.h>
#include <cppconsui/TextView.h>
#include <cppconsui/Window.h>

// TextViewWindow class
class TextViewWindow
: public CppConsUI::Window
{
  public:
    /* This is a main window, make sure it can not be closed with ESC key by
     * overriding Close() method. */
    static TextViewWindow *Instance();
    virtual void Close() {}

    virtual void ScreenResized();

  protected:
    CppConsUI::TextView *textview;

  private:
    TextViewWindow();
    virtual ~TextViewWindow() {}
    TextViewWindow(const TextViewWindow&);
    TextViewWindow& operator=(const TextViewWindow&);

    void ActionToggleScrollbar();
};

TextViewWindow *TextViewWindow::Instance()
{
  static TextViewWindow instance;
  return &instance;
}

TextViewWindow::TextViewWindow()
: CppConsUI::Window(0, 0, 0, 0)
{
  SetColorScheme("textviewwindow");

  textview = new CppConsUI::TextView(-1, -1);
  AddWidget(*textview, 0, 0);

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
    textview->Append(long_text, i % 7 + 1);

  COLORSCHEME->SetColorPair("textviewwindow", "textview", "color1",
      CppConsUI::Curses::Color::RED, CppConsUI::Curses::Color::BLACK);
  COLORSCHEME->SetColorPair("textviewwindow", "textview", "color2",
      CppConsUI::Curses::Color::GREEN, CppConsUI::Curses::Color::BLACK);
  COLORSCHEME->SetColorPair("textviewwindow", "textview", "color3",
      CppConsUI::Curses::Color::YELLOW, CppConsUI::Curses::Color::BLACK);
  COLORSCHEME->SetColorPair("textviewwindow", "textview", "color4",
      CppConsUI::Curses::Color::BLUE, CppConsUI::Curses::Color::BLACK);
  COLORSCHEME->SetColorPair("textviewwindow", "textview", "color5",
      CppConsUI::Curses::Color::MAGENTA, CppConsUI::Curses::Color::BLACK);
  COLORSCHEME->SetColorPair("textviewwindow", "textview", "color6",
      CppConsUI::Curses::Color::CYAN, CppConsUI::Curses::Color::BLACK);
  COLORSCHEME->SetColorPair("textviewwindow", "textview", "color7",
      CppConsUI::Curses::Color::WHITE, CppConsUI::Curses::Color::BLACK);

  DeclareBindable("textviewwindow", "toggle-scrollbar", sigc::mem_fun(this,
        &TextViewWindow::ActionToggleScrollbar),
      InputProcessor::BINDABLE_NORMAL);
}

void TextViewWindow::ScreenResized()
{
  MoveResize(0, 0, CppConsUI::Curses::getmaxx(),
      CppConsUI::Curses::getmaxy());
}

void TextViewWindow::ActionToggleScrollbar()
{
  textview->SetScrollBar(!textview->GetScrollBar());
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
  KEYCONFIG->AddDefaultKeyBind("testapp", "quit", "F10");
  KEYCONFIG->AddDefaultKeyBind("textviewwindow", "toggle-scrollbar", "F1");
  KEYCONFIG->RegisterDefaultKeyBinds();

  g_log_set_default_handler(g_log_func_, this);

  DeclareBindable("testapp", "quit", sigc::mem_fun(mngr,
        &CppConsUI::CoreManager::QuitMainLoop),
      InputProcessor::BINDABLE_OVERRIDE);
}

void TestApp::Run()
{
  mngr->AddWindow(*TextViewWindow::Instance());
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
