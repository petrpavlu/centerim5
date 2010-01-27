#include <cppconsui/Application.h>
#include <cppconsui/Window.h>
#include <cppconsui/Label.h>
#include <cppconsui/Keys.h>

// LabelWindow class
class LabelWindow
: public Window
{
	public:
		/* This is a main window, we make sure it can not be closed with ESC
		 * key by overriding Close() method. */
		static LabelWindow &Instance();
		virtual void Close() {}

		virtual void ScreenResized();

	protected:

	private:
		LabelWindow();
		virtual ~LabelWindow() {}
		LabelWindow(const LabelWindow &);
		LabelWindow &operator=(const LabelWindow &);
};

LabelWindow &LabelWindow::Instance()
{
	static LabelWindow instance;
	return instance;
}

LabelWindow::LabelWindow()
: Window(0, 0, 0, 0, LineStyle::DEFAULT)
{
	Label *label;

	label = new Label(*this,       // parent
			2,                     // x
			2,                     // y
			20,                    // width
			1,                     // height
			"Press F10 to quit."); // text
	/* Add label to container, container takes widget ownership and deletes it
	 * when necessary.
	 */
	AddWidget(label);

	label = new Label(*this, 2, 4, 20, 1, "Too wide string, too wide string, too wide string");
	AddWidget(label);

	label = new Label(*this, 2, 6, 20, 3, "Multiline label, multiline label, multiline label");
	AddWidget(label);

	label = new Label(*this, 2, 10, "Autosize");
	AddWidget(label);

	// unicode test
	label = new Label(*this, 2, 12, 30, 3,
			"\x56\xc5\x99\x65\xc5\xa1\x74\xc3\xad\x63\xc3\xad\x20\x70\xc5\x99"
			"\xc3\xad\xc5\xa1\x65\x72\x79\x20\x73\x65\x20\x64\x6f\xc5\xbe\x61"
			"\x64\x6f\x76\x61\x6c\x79\x20\xc3\xba\x70\x6c\x6e\xc4\x9b\x20\xc4"
			"\x8d\x65\x72\x73\x74\x76\xc3\xbd\x63\x68\x20\xc5\x99\xc3\xad\x7a"
			"\x65\xc4\x8d\x6b\xc5\xaf\x2e\x0a");
	AddWidget(label);
}

void LabelWindow::ScreenResized()
{
	MoveResize(0, 0,
			WindowManager::Instance()->getScreenW(),
			WindowManager::Instance()->getScreenH());
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

// main function
int main(void)
{
	setlocale(LC_ALL, "");

	TestApp *app = &TestApp::Instance();

	app->Run();

	return 0;
}
