#include <cppconsui/CoreManager.h>
#include <cppconsui/ConsuiCurses.h>
#include <cppconsui/KeyConfig.h>
#include <cppconsui/Keys.h>
#include <cppconsui/Label.h>
#include <cppconsui/Window.h>

// LineStyleWindow class
class LineStyleWindow
: public Window
{
  public:
    /* This is a main window, make sure it can not be closed with ESC key by
     * overriding Close() method. */
    static LineStyleWindow *Instance();
    virtual void Close() {}

    virtual void ScreenResized();

  protected:

  private:
    LineStyleWindow();
    virtual ~LineStyleWindow() {}
    LineStyleWindow(const LineStyleWindow&);
    LineStyleWindow& operator=(const LineStyleWindow&);
};

LineStyleWindow *LineStyleWindow::Instance()
{
  static LineStyleWindow instance;
  return &instance;
}

LineStyleWindow::LineStyleWindow()
: Window(0, 0, 0, 0)
{
  char *text;

  AddWidget(*(new Label("Press F10 to quit.")), 1, 1);

  AddWidget(*(new Label("ASCII")), 1, 3);
  LineStyle ascii_style(LineStyle::ASCII);
  text = g_strdup_printf("%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s",
      ascii_style.H(),
      ascii_style.HBegin(),
      ascii_style.HEnd(),
      ascii_style.HUp(),
      ascii_style.HDown(),
      ascii_style.V(),
      ascii_style.VBegin(),
      ascii_style.VEnd(),
      ascii_style.VLeft(),
      ascii_style.VRight(),
      ascii_style.Cross(),
      ascii_style.CornerTL(),
      ascii_style.CornerTR(),
      ascii_style.CornerBL(),
      ascii_style.CornerBR());
  AddWidget(*(new Label(text)), 1, 4);
  g_free(text);

  AddWidget(*(new Label("ASCII rounded")), 1, 6);
  LineStyle ascii_rounded_style(LineStyle::ASCII_ROUNDED);
  text = g_strdup_printf("%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s",
      ascii_rounded_style.H(),
      ascii_rounded_style.HBegin(),
      ascii_rounded_style.HEnd(),
      ascii_rounded_style.HUp(),
      ascii_rounded_style.HDown(),
      ascii_rounded_style.V(),
      ascii_rounded_style.VBegin(),
      ascii_rounded_style.VEnd(),
      ascii_rounded_style.VLeft(),
      ascii_rounded_style.VRight(),
      ascii_rounded_style.Cross(),
      ascii_rounded_style.CornerTL(),
      ascii_rounded_style.CornerTR(),
      ascii_rounded_style.CornerBL(),
      ascii_rounded_style.CornerBR());
  AddWidget(*(new Label(text)), 1, 7);
  g_free(text);

  AddWidget(*(new Label("light")), 1, 9);
  LineStyle light_style(LineStyle::LIGHT);
  text = g_strdup_printf("%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s",
      light_style.H(),
      light_style.HBegin(),
      light_style.HEnd(),
      light_style.HUp(),
      light_style.HDown(),
      light_style.V(),
      light_style.VBegin(),
      light_style.VEnd(),
      light_style.VLeft(),
      light_style.VRight(),
      light_style.Cross(),
      light_style.CornerTL(),
      light_style.CornerTR(),
      light_style.CornerBL(),
      light_style.CornerBR());
  AddWidget(*(new Label(text)), 1, 10);
  g_free(text);

  AddWidget(*(new Label("light rounded")), 1, 12);
  LineStyle light_rounded_style(LineStyle::LIGHT_ROUNDED);
  text = g_strdup_printf("%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s",
      light_rounded_style.H(),
      light_rounded_style.HBegin(),
      light_rounded_style.HEnd(),
      light_rounded_style.HUp(),
      light_rounded_style.HDown(),
      light_rounded_style.V(),
      light_rounded_style.VBegin(),
      light_rounded_style.VEnd(),
      light_rounded_style.VLeft(),
      light_rounded_style.VRight(),
      light_rounded_style.Cross(),
      light_rounded_style.CornerTL(),
      light_rounded_style.CornerTR(),
      light_rounded_style.CornerBL(),
      light_rounded_style.CornerBR());
  AddWidget(*(new Label(text)), 1, 13);
  g_free(text);

  AddWidget(*(new Label("heavy")), 1, 15);
  LineStyle heavy_style(LineStyle::HEAVY);
  text = g_strdup_printf("%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s",
      heavy_style.H(),
      heavy_style.HBegin(),
      heavy_style.HEnd(),
      heavy_style.HUp(),
      heavy_style.HDown(),
      heavy_style.V(),
      heavy_style.VBegin(),
      heavy_style.VEnd(),
      heavy_style.VLeft(),
      heavy_style.VRight(),
      heavy_style.Cross(),
      heavy_style.CornerTL(),
      heavy_style.CornerTR(),
      heavy_style.CornerBL(),
      heavy_style.CornerBR());
  AddWidget(*(new Label(text)), 1, 16);
  g_free(text);
}

void LineStyleWindow::ScreenResized()
{
  MoveResize(0, 0, COREMANAGER->GetScreenWidth(),
      COREMANAGER->GetScreenHeight());
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
  mngr->AddWindow(*LineStyleWindow::Instance());
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
