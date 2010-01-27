#include <cppconsui/Application.h>
#include <cppconsui/Window.h>
#include <cppconsui/Label.h>
#include <cppconsui/Keys.h>
#include <cppconsui/ConsuiCurses.h>

// LineStyleWindow class
class LineStyleWindow
: public Window
{
	public:
		static LineStyleWindow &Instance();
		virtual void Close() {}

		virtual void ScreenResized();
		virtual void Draw();

	protected:
		LineStyle *ascii_style;
		LineStyle *ascii_rounded_style;
		LineStyle *light_style;
		LineStyle *light_rounded_style;
		LineStyle *heavy_style;

	private:
		LineStyleWindow();
		virtual ~LineStyleWindow();
		LineStyleWindow(const LineStyleWindow &);
		LineStyleWindow &operator=(const LineStyleWindow &);
};

LineStyleWindow &LineStyleWindow::Instance()
{
	static LineStyleWindow instance;
	return instance;
}

LineStyleWindow::LineStyleWindow()
: Window(0, 0, 0, 0, new Border())
{
	Label *label;

	ascii_style = new LineStyle(LineStyle::ASCII);
	ascii_rounded_style = new LineStyle(LineStyle::ASCII_ROUNDED);
	light_style = new LineStyle(LineStyle::LIGHT);
	light_rounded_style = new LineStyle(LineStyle::LIGHT_ROUNDED);
	heavy_style = new LineStyle(LineStyle::HEAVY);

	label = new Label(*this, 2, 2, "Press F10 to quit.");
	AddWidget(label);

	label = new Label(*this, 2, 4, "ASCII");
	AddWidget(label);

	label = new Label(*this, 2, 7, "ASCII rounded");
	AddWidget(label);

	label = new Label(*this, 2, 10, "light");
	AddWidget(label);

	label = new Label(*this, 2, 13, "light rounded");
	AddWidget(label);

	label = new Label(*this, 2, 16, "heavy");
	AddWidget(label);
}

LineStyleWindow::~LineStyleWindow()
{
	delete ascii_style;
	delete ascii_rounded_style;
	delete light_style;
	delete light_rounded_style;
	delete heavy_style;
}

void LineStyleWindow::ScreenResized()
{
	MoveResize(0, 0,
			WindowManager::Instance()->getScreenW(),
			WindowManager::Instance()->getScreenH());
}

void LineStyleWindow::Draw()
{
	int i;

	// ASCII
	i = 2;
	Curses::mvwaddstring(area, 5, i++, 1, ascii_style->H());
	Curses::mvwaddstring(area, 5, i++, 1, ascii_style->HBegin());
	Curses::mvwaddstring(area, 5, i++, 1, ascii_style->HEnd());
	Curses::mvwaddstring(area, 5, i++, 1, ascii_style->HUp());
	Curses::mvwaddstring(area, 5, i++, 1, ascii_style->HDown());
	Curses::mvwaddstring(area, 5, i++, 1, ascii_style->V());
	Curses::mvwaddstring(area, 5, i++, 1, ascii_style->VBegin());
	Curses::mvwaddstring(area, 5, i++, 1, ascii_style->VEnd());
	Curses::mvwaddstring(area, 5, i++, 1, ascii_style->VLeft());
	Curses::mvwaddstring(area, 5, i++, 1, ascii_style->VRight());
	Curses::mvwaddstring(area, 5, i++, 1, ascii_style->Cross());
	Curses::mvwaddstring(area, 5, i++, 1, ascii_style->CornerTL());
	Curses::mvwaddstring(area, 5, i++, 1, ascii_style->CornerTR());
	Curses::mvwaddstring(area, 5, i++, 1, ascii_style->CornerBL());
	Curses::mvwaddstring(area, 5, i++, 1, ascii_style->CornerBR());

	// ASCII rounded
	i = 2;
	Curses::mvwaddstring(area, 8, i++, 1, ascii_rounded_style->H());
	Curses::mvwaddstring(area, 8, i++, 1, ascii_rounded_style->HBegin());
	Curses::mvwaddstring(area, 8, i++, 1, ascii_rounded_style->HEnd());
	Curses::mvwaddstring(area, 8, i++, 1, ascii_rounded_style->HUp());
	Curses::mvwaddstring(area, 8, i++, 1, ascii_rounded_style->HDown());
	Curses::mvwaddstring(area, 8, i++, 1, ascii_rounded_style->V());
	Curses::mvwaddstring(area, 8, i++, 1, ascii_rounded_style->VBegin());
	Curses::mvwaddstring(area, 8, i++, 1, ascii_rounded_style->VEnd());
	Curses::mvwaddstring(area, 8, i++, 1, ascii_rounded_style->VLeft());
	Curses::mvwaddstring(area, 8, i++, 1, ascii_rounded_style->VRight());
	Curses::mvwaddstring(area, 8, i++, 1, ascii_rounded_style->Cross());
	Curses::mvwaddstring(area, 8, i++, 1, ascii_rounded_style->CornerTL());
	Curses::mvwaddstring(area, 8, i++, 1, ascii_rounded_style->CornerTR());
	Curses::mvwaddstring(area, 8, i++, 1, ascii_rounded_style->CornerBL());
	Curses::mvwaddstring(area, 8, i++, 1, ascii_rounded_style->CornerBR());

	// light
	i = 2;
	Curses::mvwaddstring(area, 11, i++, 1, light_style->H());
	Curses::mvwaddstring(area, 11, i++, 1, light_style->HBegin());
	Curses::mvwaddstring(area, 11, i++, 1, light_style->HEnd());
	Curses::mvwaddstring(area, 11, i++, 1, light_style->HUp());
	Curses::mvwaddstring(area, 11, i++, 1, light_style->HDown());
	Curses::mvwaddstring(area, 11, i++, 1, light_style->V());
	Curses::mvwaddstring(area, 11, i++, 1, light_style->VBegin());
	Curses::mvwaddstring(area, 11, i++, 1, light_style->VEnd());
	Curses::mvwaddstring(area, 11, i++, 1, light_style->VLeft());
	Curses::mvwaddstring(area, 11, i++, 1, light_style->VRight());
	Curses::mvwaddstring(area, 11, i++, 1, light_style->Cross());
	Curses::mvwaddstring(area, 11, i++, 1, light_style->CornerTL());
	Curses::mvwaddstring(area, 11, i++, 1, light_style->CornerTR());
	Curses::mvwaddstring(area, 11, i++, 1, light_style->CornerBL());
	Curses::mvwaddstring(area, 11, i++, 1, light_style->CornerBR());

	// light rounded
	i = 2;
	Curses::mvwaddstring(area, 14, i++, 1, light_rounded_style->H());
	Curses::mvwaddstring(area, 14, i++, 1, light_rounded_style->HBegin());
	Curses::mvwaddstring(area, 14, i++, 1, light_rounded_style->HEnd());
	Curses::mvwaddstring(area, 14, i++, 1, light_rounded_style->HUp());
	Curses::mvwaddstring(area, 14, i++, 1, light_rounded_style->HDown());
	Curses::mvwaddstring(area, 14, i++, 1, light_rounded_style->V());
	Curses::mvwaddstring(area, 14, i++, 1, light_rounded_style->VBegin());
	Curses::mvwaddstring(area, 14, i++, 1, light_rounded_style->VEnd());
	Curses::mvwaddstring(area, 14, i++, 1, light_rounded_style->VLeft());
	Curses::mvwaddstring(area, 14, i++, 1, light_rounded_style->VRight());
	Curses::mvwaddstring(area, 14, i++, 1, light_rounded_style->Cross());
	Curses::mvwaddstring(area, 14, i++, 1, light_rounded_style->CornerTL());
	Curses::mvwaddstring(area, 14, i++, 1, light_rounded_style->CornerTR());
	Curses::mvwaddstring(area, 14, i++, 1, light_rounded_style->CornerBL());
	Curses::mvwaddstring(area, 14, i++, 1, light_rounded_style->CornerBR());

	// heavy
	i = 2;
	Curses::mvwaddstring(area, 17, i++, 1, heavy_style->H());
	Curses::mvwaddstring(area, 17, i++, 1, heavy_style->HBegin());
	Curses::mvwaddstring(area, 17, i++, 1, heavy_style->HEnd());
	Curses::mvwaddstring(area, 17, i++, 1, heavy_style->HUp());
	Curses::mvwaddstring(area, 17, i++, 1, heavy_style->HDown());
	Curses::mvwaddstring(area, 17, i++, 1, heavy_style->V());
	Curses::mvwaddstring(area, 17, i++, 1, heavy_style->VBegin());
	Curses::mvwaddstring(area, 17, i++, 1, heavy_style->VEnd());
	Curses::mvwaddstring(area, 17, i++, 1, heavy_style->VLeft());
	Curses::mvwaddstring(area, 17, i++, 1, heavy_style->VRight());
	Curses::mvwaddstring(area, 17, i++, 1, heavy_style->Cross());
	Curses::mvwaddstring(area, 17, i++, 1, heavy_style->CornerTL());
	Curses::mvwaddstring(area, 17, i++, 1, heavy_style->CornerTR());
	Curses::mvwaddstring(area, 17, i++, 1, heavy_style->CornerBL());
	Curses::mvwaddstring(area, 17, i++, 1, heavy_style->CornerBR());

	Window::Draw();
}

// TestApp class

#define CONTEXT_TESTAPP "testapp"

class TestApp
: public Application
{
	public:
		static TestApp &Instance();

		virtual void Run();
		virtual void Quit();

		virtual void ScreenResized() {}

		// ignore every message
		static void g_log_func_(const gchar *log_domain, GLogLevelFlags log_level, const gchar *message, gpointer user_data)
			{}

	protected:

	private:
		TestApp();
		TestApp(const TestApp &);
		TestApp &operator=(const TestApp &);
		virtual ~TestApp() {}

		DECLARE_SIG_REGISTERKEYS();
		static bool RegisterKeys();
		void DeclareBindables();
};

TestApp &TestApp::Instance()
{
	static TestApp instance;
	return instance;
}

TestApp::TestApp()
: Application()
{
	g_log_set_default_handler(g_log_func_, this);

	DeclareBindables();
}

void TestApp::Run()
{
	// TODO comment what happens here, who takes ownership etc.
	windowmanager->Add(&LineStyleWindow::Instance());

	Application::Run();
}

void TestApp::DeclareBindables()
{
	DeclareBindable(CONTEXT_TESTAPP, "quit", sigc::mem_fun(this, &TestApp::Quit),
			InputProcessor::Bindable_Override);
}

DEFINE_SIG_REGISTERKEYS(TestApp, RegisterKeys);
bool TestApp::RegisterKeys()
{
	RegisterKeyDef(CONTEXT_TESTAPP, "quit", _("Quit TestApp."), Keys::Instance()->Key_f10());
	return true;
}

void TestApp::Quit()
{
	Application::Quit();
}

// main function
int main(void)
{
	setlocale(LC_ALL, "");

	TestApp *app = &TestApp::Instance();

	app->Run();

	return 0;
}
