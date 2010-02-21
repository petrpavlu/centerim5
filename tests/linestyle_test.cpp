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
		/* This is a main window, make sure it can not be closed with ESC key
		 * by overriding Close() method. */
		static LineStyleWindow &Instance();
		virtual void Close() {}

		virtual void ScreenResized();
		virtual void Draw();

	protected:

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
: Window(0, 0, 0, 0)
{
	Label *label;

	label = new Label(*this, 1, 1, "Press F10 to quit.");
	AddWidget(label);

	label = new Label(*this, 1, 3, "ASCII");
	AddWidget(label);

	label = new Label(*this, 1, 6, "ASCII rounded");
	AddWidget(label);

	label = new Label(*this, 1, 9, "light");
	AddWidget(label);

	label = new Label(*this, 1, 12, "light rounded");
	AddWidget(label);

	label = new Label(*this, 1, 15, "heavy");
	AddWidget(label);
}

LineStyleWindow::~LineStyleWindow()
{
}

void LineStyleWindow::ScreenResized()
{
	MoveResize(0, 0,
			WindowManager::Instance()->getScreenW(),
			WindowManager::Instance()->getScreenH());
}

void LineStyleWindow::Draw()
{
	LineStyle ascii_style(LineStyle::ASCII);
	LineStyle ascii_rounded_style(LineStyle::ASCII_ROUNDED);
	LineStyle light_style(LineStyle::LIGHT);
	LineStyle light_rounded_style(LineStyle::LIGHT_ROUNDED);
	LineStyle heavy_style(LineStyle::HEAVY);

	int i;

	// ASCII
	i = 2;
	area->mvaddstring(i++, 5, 1, ascii_style.H());
	area->mvaddstring(i++, 5, 1, ascii_style.HBegin());
	area->mvaddstring(i++, 5, 1, ascii_style.HEnd());
	area->mvaddstring(i++, 5, 1, ascii_style.HUp());
	area->mvaddstring(i++, 5, 1, ascii_style.HDown());
	area->mvaddstring(i++, 5, 1, ascii_style.V());
	area->mvaddstring(i++, 5, 1, ascii_style.VBegin());
	area->mvaddstring(i++, 5, 1, ascii_style.VEnd());
	area->mvaddstring(i++, 5, 1, ascii_style.VLeft());
	area->mvaddstring(i++, 5, 1, ascii_style.VRight());
	area->mvaddstring(i++, 5, 1, ascii_style.Cross());
	area->mvaddstring(i++, 5, 1, ascii_style.CornerTL());
	area->mvaddstring(i++, 5, 1, ascii_style.CornerTR());
	area->mvaddstring(i++, 5, 1, ascii_style.CornerBL());
	area->mvaddstring(i++, 5, 1, ascii_style.CornerBR());

	// ASCII rounded
	i = 2;
	area->mvaddstring(i++, 8, 1, ascii_rounded_style.H());
	area->mvaddstring(i++, 8, 1, ascii_rounded_style.HBegin());
	area->mvaddstring(i++, 8, 1, ascii_rounded_style.HEnd());
	area->mvaddstring(i++, 8, 1, ascii_rounded_style.HUp());
	area->mvaddstring(i++, 8, 1, ascii_rounded_style.HDown());
	area->mvaddstring(i++, 8, 1, ascii_rounded_style.V());
	area->mvaddstring(i++, 8, 1, ascii_rounded_style.VBegin());
	area->mvaddstring(i++, 8, 1, ascii_rounded_style.VEnd());
	area->mvaddstring(i++, 8, 1, ascii_rounded_style.VLeft());
	area->mvaddstring(i++, 8, 1, ascii_rounded_style.VRight());
	area->mvaddstring(i++, 8, 1, ascii_rounded_style.Cross());
	area->mvaddstring(i++, 8, 1, ascii_rounded_style.CornerTL());
	area->mvaddstring(i++, 8, 1, ascii_rounded_style.CornerTR());
	area->mvaddstring(i++, 8, 1, ascii_rounded_style.CornerBL());
	area->mvaddstring(i++, 8, 1, ascii_rounded_style.CornerBR());

	// light
	i = 2;
	area->mvaddstring(i++, 11, 1, light_style.H());
	area->mvaddstring(i++, 11, 1, light_style.HBegin());
	area->mvaddstring(i++, 11, 1, light_style.HEnd());
	area->mvaddstring(i++, 11, 1, light_style.HUp());
	area->mvaddstring(i++, 11, 1, light_style.HDown());
	area->mvaddstring(i++, 11, 1, light_style.V());
	area->mvaddstring(i++, 11, 1, light_style.VBegin());
	area->mvaddstring(i++, 11, 1, light_style.VEnd());
	area->mvaddstring(i++, 11, 1, light_style.VLeft());
	area->mvaddstring(i++, 11, 1, light_style.VRight());
	area->mvaddstring(i++, 11, 1, light_style.Cross());
	area->mvaddstring(i++, 11, 1, light_style.CornerTL());
	area->mvaddstring(i++, 11, 1, light_style.CornerTR());
	area->mvaddstring(i++, 11, 1, light_style.CornerBL());
	area->mvaddstring(i++, 11, 1, light_style.CornerBR());

	// light rounded
	i = 2;
	area->mvaddstring(i++, 14, 1, light_rounded_style.H());
	area->mvaddstring(i++, 14, 1, light_rounded_style.HBegin());
	area->mvaddstring(i++, 14, 1, light_rounded_style.HEnd());
	area->mvaddstring(i++, 14, 1, light_rounded_style.HUp());
	area->mvaddstring(i++, 14, 1, light_rounded_style.HDown());
	area->mvaddstring(i++, 14, 1, light_rounded_style.V());
	area->mvaddstring(i++, 14, 1, light_rounded_style.VBegin());
	area->mvaddstring(i++, 14, 1, light_rounded_style.VEnd());
	area->mvaddstring(i++, 14, 1, light_rounded_style.VLeft());
	area->mvaddstring(i++, 14, 1, light_rounded_style.VRight());
	area->mvaddstring(i++, 14, 1, light_rounded_style.Cross());
	area->mvaddstring(i++, 14, 1, light_rounded_style.CornerTL());
	area->mvaddstring(i++, 14, 1, light_rounded_style.CornerTR());
	area->mvaddstring(i++, 14, 1, light_rounded_style.CornerBL());
	area->mvaddstring(i++, 14, 1, light_rounded_style.CornerBR());

	// heavy
	i = 2;
	area->mvaddstring(i++, 17, 1, heavy_style.H());
	area->mvaddstring(i++, 17, 1, heavy_style.HBegin());
	area->mvaddstring(i++, 17, 1, heavy_style.HEnd());
	area->mvaddstring(i++, 17, 1, heavy_style.HUp());
	area->mvaddstring(i++, 17, 1, heavy_style.HDown());
	area->mvaddstring(i++, 17, 1, heavy_style.V());
	area->mvaddstring(i++, 17, 1, heavy_style.VBegin());
	area->mvaddstring(i++, 17, 1, heavy_style.VEnd());
	area->mvaddstring(i++, 17, 1, heavy_style.VLeft());
	area->mvaddstring(i++, 17, 1, heavy_style.VRight());
	area->mvaddstring(i++, 17, 1, heavy_style.Cross());
	area->mvaddstring(i++, 17, 1, heavy_style.CornerTL());
	area->mvaddstring(i++, 17, 1, heavy_style.CornerTR());
	area->mvaddstring(i++, 17, 1, heavy_style.CornerBL());
	area->mvaddstring(i++, 17, 1, heavy_style.CornerBR());

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
	DeclareBindable(CONTEXT_TESTAPP, "quit", "Quit TestApp.",
			sigc::mem_fun(this, &TestApp::Quit),
			InputProcessor::Bindable_Override);
}

DEFINE_SIG_REGISTERKEYS(TestApp, RegisterKeys);
bool TestApp::RegisterKeys()
{
	RegisterKeyDef(CONTEXT_TESTAPP, "quit", Keys::FunctionTermKey(10));
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
