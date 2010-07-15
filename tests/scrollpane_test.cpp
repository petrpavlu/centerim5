#include <cppconsui/CoreManager.h>
#include <cppconsui/Window.h>
#include <cppconsui/Label.h>
#include <cppconsui/Button.h>
#include <cppconsui/ScrollPane.h>
#include <cppconsui/Keys.h>

static const char *pic[] = 
{
" ______                                __          __         __   __                       ",
"|      |.-----.-----.-----.----.---.-.|  |_.--.--.|  |.---.-.|  |_|__|.-----.-----.-----.   ",
"|   ---||  _  |     |  _  |   _|  _  ||   _|  |  ||  ||  _  ||   _|  ||  _  |     |__ --|__ ",
"|______||_____|__|__|___  |__| |___._||____|_____||__||___._||____|__||_____|__|__|_____|  |",
"                    |_____|                                                              |_|",
"                      __               __     __               __     _______ __            ",
".--.--.-----.--.--.  |__|.--.--.-----.|  |_  |  |.-----.-----.|  |_  |_     _|  |--.-----.  ",
"|  |  |  _  |  |  |  |  ||  |  |__ --||   _| |  ||  _  |__ --||   _|   |   | |     |  -__|  ",
"|___  |_____|_____|  |  ||_____|_____||____| |__||_____|_____||____|   |___| |__|__|_____|  ",
"|_____|             |___|                                                                   ",
" _______                                                                                    ",
"|     __|.---.-.--------.-----.                                                             ",
"|    |  ||  _  |        |  -__|__                                                           ",
"|_______||___._|__|__|__|_____|__|                                                          "
};


// MyScrollPane class
class MyScrollPane
: public ScrollPane
{
	public:
		MyScrollPane(int w, int h, int scrollw, int scrollh);
		virtual ~MyScrollPane() {}

		// Widget
		virtual void Draw();

	protected:

	private:
		MyScrollPane(const MyScrollPane&);
		MyScrollPane& operator=(const MyScrollPane&);
};

MyScrollPane::MyScrollPane(int w, int h, int scrollw, int scrollh)
: ScrollPane(w, h, scrollw, scrollh)
{
}

void MyScrollPane::Draw()
{
	if (!area || !scrollarea)
		return;

	int real_height = area->getmaxy();
	for (int i = 0; i < real_height && i < (int) (sizeof(pic) / sizeof(pic[0])); i++)
		area->mvaddstring(0, i, pic[i]);

	ScrollPane::Draw();
}

// ScrollPaneWindow class

#define CONTEXT_SCROLLPANEWINDOW "scrollpanewindow"

class ScrollPaneWindow
: public Window
{
	public:
		/* This is a main window, make sure it can not be closed with ESC key
		 * by overriding Close() method. */
		static ScrollPaneWindow *Instance();
		virtual void Close() {}

		virtual void ScreenResized();

	protected:
		MyScrollPane *pane;

	private:
		ScrollPaneWindow();
		virtual ~ScrollPaneWindow() {}
		ScrollPaneWindow(const ScrollPaneWindow&);
		ScrollPaneWindow& operator=(const ScrollPaneWindow&);

		void ScrollUp();
		void ScrollDown();
		void ScrollLeft();
		void ScrollRight();

		DECLARE_SIG_REGISTERKEYS();
		static bool RegisterKeys();
		void DeclareBindables();
};

ScrollPaneWindow *ScrollPaneWindow::Instance()
{
	static ScrollPaneWindow instance;
	return &instance;
}

ScrollPaneWindow::ScrollPaneWindow()
: Window(0, 0, 0, 0)
{
	AddWidget(*(new Label(25, 1, "Press F10 to quit.")), 1, 1);
	AddWidget(*(new Label(25, 1, "WASD to move the picture.")), 1, 2);

	pane = new MyScrollPane(20, 10, 111, 23);
	AddWidget(*pane, 1, 4);

	DeclareBindables();
}

void ScrollPaneWindow::ScrollUp()
{
	pane->AdjustScroll(pane->GetScrollPositionX(),
			pane->GetScrollPositionY() - 1);
}

void ScrollPaneWindow::ScrollDown()
{
	pane->AdjustScroll(pane->GetScrollPositionX(),
			pane->GetScrollPositionY() + 1);
}

void ScrollPaneWindow::ScrollLeft()
{
	pane->AdjustScroll(pane->GetScrollPositionX() - 1,
			pane->GetScrollPositionY());
}

void ScrollPaneWindow::ScrollRight()
{
	pane->AdjustScroll(pane->GetScrollPositionX() + 1,
			pane->GetScrollPositionY());
}

void ScrollPaneWindow::DeclareBindables()
{
	DeclareBindable(CONTEXT_SCROLLPANEWINDOW, "scroll-up",
			sigc::mem_fun(this, &ScrollPaneWindow::ScrollUp),
			InputProcessor::Bindable_Normal);
	DeclareBindable(CONTEXT_SCROLLPANEWINDOW, "scroll-down",
			sigc::mem_fun(this, &ScrollPaneWindow::ScrollDown),
			InputProcessor::Bindable_Normal);
	DeclareBindable(CONTEXT_SCROLLPANEWINDOW, "scroll-left",
			sigc::mem_fun(this, &ScrollPaneWindow::ScrollLeft),
			InputProcessor::Bindable_Normal);
	DeclareBindable(CONTEXT_SCROLLPANEWINDOW, "scroll-right",
			sigc::mem_fun(this, &ScrollPaneWindow::ScrollRight),
			InputProcessor::Bindable_Normal);
}

DEFINE_SIG_REGISTERKEYS(ScrollPaneWindow, RegisterKeys);
bool ScrollPaneWindow::RegisterKeys()
{
	RegisterKeyDef(CONTEXT_SCROLLPANEWINDOW, "scroll-up",
			"Scroll up.", Keys::UnicodeTermKey("w"));
	RegisterKeyDef(CONTEXT_SCROLLPANEWINDOW, "scroll-down",
			"Scroll down.", Keys::UnicodeTermKey("s"));
	RegisterKeyDef(CONTEXT_SCROLLPANEWINDOW, "scroll-left",
			"Scroll left.", Keys::UnicodeTermKey("a"));
	RegisterKeyDef(CONTEXT_SCROLLPANEWINDOW, "scroll-right",
			"Scroll right.", Keys::UnicodeTermKey("d"));
	return true;
}

void ScrollPaneWindow::ScreenResized()
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
		static void g_log_func_(const gchar *log_domain, GLogLevelFlags log_level, const gchar *message, gpointer user_data)
			{}

	protected:

	private:
		CoreManager *mngr;

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
: InputProcessor()
{
	mngr = CoreManager::Instance();

	g_log_set_default_handler(g_log_func_, this);

	DeclareBindables();
}

void TestApp::Run()
{
	mngr->AddWindow(*ScrollPaneWindow::Instance());
	mngr->SetTopInputProcessor(*this);
	mngr->EnableResizing();
	mngr->StartMainLoop();
}

void TestApp::DeclareBindables()
{
	DeclareBindable(CONTEXT_TESTAPP, "quit",
			sigc::mem_fun(mngr, &CoreManager::QuitMainLoop),
			InputProcessor::Bindable_Override);
}

DEFINE_SIG_REGISTERKEYS(TestApp, RegisterKeys);
bool TestApp::RegisterKeys()
{
	RegisterKeyDef(CONTEXT_TESTAPP, "quit",
			"Quit TestApp.", Keys::FunctionTermKey(10));
	return true;
}

// main function
int main()
{
	setlocale(LC_ALL, "");

	TestApp *app = TestApp::Instance();

	app->Run();

	return 0;
}
