#include <cppconsui/ColorScheme.h>
#include <cppconsui/CoreManager.h>
#include <cppconsui/KeyConfig.h>
#include <cppconsui/Keys.h>
#include <cppconsui/TextView.h>
#include <cppconsui/Window.h>

// TextViewWindow class
class TextViewWindow
: public Window
{
  public:
    /* This is a main window, make sure it can not be closed with ESC key by
     * overriding Close() method. */
    static TextViewWindow *Instance();
    virtual void Close() {}

    virtual void ScreenResized();

  protected:
    TextView *textview;

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
: Window(0, 0, 0, 0)
{
  SetColorScheme("textviewwindow");

  textview = new TextView(-1, -1);
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

  COLORSCHEME->SetColorPair("textviewwindow", "textview", "color1", Curses::Color::RED, Curses::Color::BLACK);
  COLORSCHEME->SetColorPair("textviewwindow", "textview", "color2", Curses::Color::GREEN, Curses::Color::BLACK);
  COLORSCHEME->SetColorPair("textviewwindow", "textview", "color3", Curses::Color::YELLOW, Curses::Color::BLACK);
  COLORSCHEME->SetColorPair("textviewwindow", "textview", "color4", Curses::Color::BLUE, Curses::Color::BLACK);
  COLORSCHEME->SetColorPair("textviewwindow", "textview", "color5", Curses::Color::MAGENTA, Curses::Color::BLACK);
  COLORSCHEME->SetColorPair("textviewwindow", "textview", "color6", Curses::Color::CYAN, Curses::Color::BLACK);
  COLORSCHEME->SetColorPair("textviewwindow", "textview", "color7", Curses::Color::WHITE, Curses::Color::BLACK);

  DeclareBindable("textviewwindow", "toggle-scrollbar", sigc::mem_fun(this,
        &TextViewWindow::ActionToggleScrollbar),
      InputProcessor::BINDABLE_NORMAL);
  KEYCONFIG->RegisterKeyDef("textviewwindow", "toggle-scrollbar",
      Keys::FunctionTermKey(1));
}

void TextViewWindow::ScreenResized()
{
  MoveResize(0, 0, COREMANAGER->GetScreenWidth(),
      COREMANAGER->GetScreenHeight());
}

void TextViewWindow::ActionToggleScrollbar()
{
  textview->SetScrollBar(!textview->GetScrollBar());
}

// TestApp class
class TestApp
: public InputProcessor
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
    CoreManager *mngr;

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
: InputProcessor()
{
  mngr = CoreManager::Instance();

  g_log_set_default_handler(g_log_func_, this);

  DeclareBindable("testapp", "quit", sigc::mem_fun(mngr,
        &CoreManager::QuitMainLoop), InputProcessor::BINDABLE_OVERRIDE);
  KEYCONFIG->RegisterKeyDef("testapp", "quit", Keys::FunctionTermKey(10));
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
