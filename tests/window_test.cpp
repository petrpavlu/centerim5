#include <cppconsui/Application.h>
#include <cppconsui/Window.h>
#include <cppconsui/Label.h>
#include <cppconsui/Keys.h>

// TestWindow class
class TestWindow
: public Window
{
	public:
		TestWindow(int number, int x, int y, int w, int h);
		virtual ~TestWindow() {}

		virtual void ScreenResized();

	protected:

	private:
		TestWindow(const TestWindow&);
		TestWindow& operator=(const TestWindow&);
};

TestWindow::TestWindow(int number, int x, int y, int w, int h)
: Window(x, y, w, h)
{
	Label *label;

	gchar *t = g_strdup_printf("Win %d", number);
	label = new Label(w - 4, 1, t);
	g_free(t);
	AddWidget(*label, 2, 1);

	if (number == 1) {
		label = new Label("Press F10 to quit.");
		AddWidget(*label, 2, 2);

		label = new Label("Press ESC to close a focused window.");
		AddWidget(*label, 2, 3);
	}
}

void TestWindow::ScreenResized()
{
	signal_redraw(*this);
}

// TestApp class

#define CONTEXT_TESTAPP "testapp"

class TestApp
: public Application
{
	public:
		static TestApp *Instance();

		virtual void Run();
		virtual void Quit();

		virtual void ScreenResized() {}

		// ignore every message
		static void g_log_func_(const gchar *log_domain, GLogLevelFlags log_level, const gchar *message, gpointer user_data)
			{}

	protected:

	private:
		TestApp();
		TestApp(const TestApp&);
		TestApp& operator=(const TestApp&);
		virtual ~TestApp() {}

		DECLARE_SIG_REGISTERKEYS();
		static bool RegisterKeys();
		void DeclareBindables();
};

TestApp *TestApp::Instance()
{
	static TestApp instance;
	return &instance;
}

TestApp::TestApp()
: Application()
{
	g_log_set_default_handler(g_log_func_, this);

	DeclareBindables();
}

void TestApp::Run()
{
	for (int i = 1; i <= 4; i++) {
		Window *w = new TestWindow(i, (i - 1) % 2 * 40, (i - 1) / 2 * 10, 40, 10);
		windowmanager->Add(w);
	}

	Application::Run();
}

void TestApp::DeclareBindables()
{
	DeclareBindable(CONTEXT_TESTAPP, "quit",
			sigc::mem_fun(this, &TestApp::Quit),
			InputProcessor::Bindable_Override);
}

DEFINE_SIG_REGISTERKEYS(TestApp, RegisterKeys);
bool TestApp::RegisterKeys()
{
	RegisterKeyDef(CONTEXT_TESTAPP, "quit",
			"Quit TestApp.", Keys::FunctionTermKey(10));
	return true;
}

void TestApp::Quit()
{
	Application::Quit();
}

// main function
int main()
{
	setlocale(LC_ALL, "");

	TestApp *app = TestApp::Instance();

	app->Run();

	return 0;
}
