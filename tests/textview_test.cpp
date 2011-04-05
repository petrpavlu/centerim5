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

  private:
    TextViewWindow();
    virtual ~TextViewWindow() {}
    TextViewWindow(const TextViewWindow&);
    TextViewWindow& operator=(const TextViewWindow&);
};

TextViewWindow *TextViewWindow::Instance()
{
  static TextViewWindow instance;
  return &instance;
}

TextViewWindow::TextViewWindow()
: Window(0, 0, 0, 0)
{
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

  TextView *textview = new TextView(-1, -1);
  AddWidget(*textview, 0, 0);
  textview->Append(long_text);

  char wide[13];
  int l;
  l = g_unichar_to_utf8(0x1100, wide);
  l += g_unichar_to_utf8(0x40, wide + l);
  wide[l] = '\0';
  textview->Append(wide);
}

void TextViewWindow::ScreenResized()
{
  MoveResize(0, 0, COREMANAGER->GetScreenWidth(),
      COREMANAGER->GetScreenHeight());
}

// TestApp class

#define CONTEXT_TESTAPP "testapp"

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

  DeclareBindable(CONTEXT_TESTAPP, "quit", sigc::mem_fun(mngr,
        &CoreManager::QuitMainLoop), InputProcessor::BINDABLE_OVERRIDE);
  KEYCONFIG->RegisterKeyDef(CONTEXT_TESTAPP, "quit",
      Keys::FunctionTermKey(10));
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
