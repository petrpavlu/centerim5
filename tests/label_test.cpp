#include <cppconsui/Application.h>
#include <cppconsui/Window.h>
#include <cppconsui/Label.h>
#include <cppconsui/Keys.h>

/** LabelWindow class. */
class LabelWindow
: public Window
{
	public:
		static LabelWindow &Instance();

		virtual void ScreenResized();

	protected:
		Label *label;

	private:
		LabelWindow();
		LabelWindow(const LabelWindow &);
		LabelWindow &operator=(const LabelWindow &);
		virtual ~LabelWindow() {}
};

LabelWindow &LabelWindow::Instance()
{
	static LabelWindow instance;
	return instance;
}

LabelWindow::LabelWindow()
: Window(0, 0, 0, 0, new Border())
{
	label = new Label(*this, 2, 2, 20, 1, "Basic test");
	AddWidget(label);

	label = new Label(*this, 2, 5, 20, 1, "Too wide string, too wide string, too wide string");
	AddWidget(label);

	label = new Label(*this, 2, 7, 20, 3, "Multiline label, multiline label, multiline label");
	AddWidget(label);
}

void LabelWindow::ScreenResized()
{
	MoveResize(0, 0,
			WindowManager::Instance()->getScreenW(),
			WindowManager::Instance()->getScreenH());
}

/** TestApp class. */

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
		virtual ~TestApp() {};

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
	windowmanager->Add(&LabelWindow::Instance());

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

/** Main function. */
int main(void)
{
	setlocale(LC_ALL, "");

	TestApp *app = &TestApp::Instance();

	app->Run();

	return 0;
}
