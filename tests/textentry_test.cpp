#include <cppconsui/CoreManager.h>
#include <cppconsui/KeyConfig.h>
#include <cppconsui/Keys.h>
#include <cppconsui/Label.h>
#include <cppconsui/Panel.h>
#include <cppconsui/TextEntry.h>
#include <cppconsui/Window.h>

// TextEntryWindow class
class TextEntryWindow
: public CppConsUI::Window
{
  public:
    /* This is a main window, make sure it can not be closed with ESC key by
     * overriding Close() method. */
    static TextEntryWindow *Instance();
    virtual void Close() {}

    virtual void ScreenResized();

  protected:

  private:
    TextEntryWindow();
    virtual ~TextEntryWindow() {}
    TextEntryWindow(const TextEntryWindow&);
    TextEntryWindow& operator=(const TextEntryWindow&);
};

TextEntryWindow *TextEntryWindow::Instance()
{
  static TextEntryWindow instance;
  return &instance;
}

TextEntryWindow::TextEntryWindow()
: CppConsUI::Window(0, 0, 0, 0)
{
  AddWidget(*(new CppConsUI::Label("Press F10 to quit.")), 1, 1);
  AddWidget(*(new CppConsUI::Label(
          "Press TAB or up/down arrow keys to move focus.")), 1, 2);
  AddWidget(*(new CppConsUI::Label("All TextEntry widgets are surrouned by "
          "Panel widget in this test (except the autosize example).")), 1, 3);

  AddWidget(*(new CppConsUI::Panel(22, 3)), 1, 5);
  AddWidget(*(new CppConsUI::TextEntry(20, 1, "Edit me.")), 2, 6);

  AddWidget(*(new CppConsUI::Panel(22, 3)), 1, 9);
  AddWidget(*(new CppConsUI::TextEntry(20, 1,
          "Too wide string, too wide string, too wide string")), 2, 10);

  AddWidget(*(new CppConsUI::Panel(22, 5)), 1, 13);
  AddWidget(*(new CppConsUI::TextEntry(20, 3,
          "Multiline textentry, multiline textentry")), 2, 14);

  // unicode test
  AddWidget(*(new CppConsUI::Panel(32, 5)), 1, 19);
  AddWidget(*(new CppConsUI::TextEntry(30, 3,
      "\x56\xc5\x99\x65\xc5\xa1\x74\xc3\xad\x63\xc3\xad\x20\x70\xc5\x99"
      "\xc3\xad\xc5\xa1\x65\x72\x79\x20\x73\x65\x20\x64\x6f\xc5\xbe\x61"
      "\x64\x6f\x76\x61\x6c\x79\x20\xc3\xba\x70\x6c\x6e\xc4\x9b\x20\xc4"
      "\x8d\x65\x72\x73\x74\x76\xc3\xbd\x63\x68\x20\xc5\x99\xc3\xad\x7a"
      "\x65\xc4\x8d\x6b\xc5\xaf\x2e\x0a")), 2, 20);

  AddWidget(*(new CppConsUI::TextEntry("Autosize")), 2, 25);
}

void TextEntryWindow::ScreenResized()
{
  MoveResize(0, 0, COREMANAGER->GetScreenWidth(),
      COREMANAGER->GetScreenHeight());
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
  KEYCONFIG->RegisterDefaultKeys();

  g_log_set_default_handler(g_log_func_, this);

  DeclareBindable("testapp", "quit", sigc::mem_fun(mngr,
        &CppConsUI::CoreManager::QuitMainLoop),
      InputProcessor::BINDABLE_OVERRIDE);
  KEYCONFIG->BindKey("testapp", "quit", "F10");
}

void TestApp::Run()
{
  mngr->AddWindow(*TextEntryWindow::Instance());
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
