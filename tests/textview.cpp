#include <cppconsui/ColorScheme.h>
#include <cppconsui/KeyConfig.h>
#include <cppconsui/TextView.h>
#include <cppconsui/Window.h>

// TestWindow class
class TestWindow : public CppConsUI::Window {
public:
  TestWindow();
  virtual ~TestWindow() override {}

protected:
  const int SCHEME_TEXTVIEWWINDOW = 1;
  CppConsUI::TextView *textview;

private:
  void actionToggleScrollbar();

  CONSUI_DISABLE_COPY(TestWindow);
};

TestWindow::TestWindow() : CppConsUI::Window(0, 0, AUTOSIZE, AUTOSIZE)
{
  setClosable(false);
  setColorScheme(SCHEME_TEXTVIEWWINDOW);

  textview = new CppConsUI::TextView(AUTOSIZE, AUTOSIZE);
  addWidget(*textview, 0, 0);

  const char *long_text =
    "Lorem ipsum dolor sit amet, consectetur"
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
    textview->append(long_text, i % 7 + 1);

  COLORSCHEME->setAttributesExt(SCHEME_TEXTVIEWWINDOW,
    CppConsUI::ColorScheme::PROPERTY_TEXTVIEW_TEXT, 1,
    CppConsUI::Curses::Color::RED, CppConsUI::Curses::Color::BLACK);
  COLORSCHEME->setAttributesExt(SCHEME_TEXTVIEWWINDOW,
    CppConsUI::ColorScheme::PROPERTY_TEXTVIEW_TEXT, 2,
    CppConsUI::Curses::Color::GREEN, CppConsUI::Curses::Color::BLACK);
  COLORSCHEME->setAttributesExt(SCHEME_TEXTVIEWWINDOW,
    CppConsUI::ColorScheme::PROPERTY_TEXTVIEW_TEXT, 3,
    CppConsUI::Curses::Color::YELLOW, CppConsUI::Curses::Color::BLACK);
  COLORSCHEME->setAttributesExt(SCHEME_TEXTVIEWWINDOW,
    CppConsUI::ColorScheme::PROPERTY_TEXTVIEW_TEXT, 4,
    CppConsUI::Curses::Color::BLUE, CppConsUI::Curses::Color::BLACK);
  COLORSCHEME->setAttributesExt(SCHEME_TEXTVIEWWINDOW,
    CppConsUI::ColorScheme::PROPERTY_TEXTVIEW_TEXT, 5,
    CppConsUI::Curses::Color::MAGENTA, CppConsUI::Curses::Color::BLACK);
  COLORSCHEME->setAttributesExt(SCHEME_TEXTVIEWWINDOW,
    CppConsUI::ColorScheme::PROPERTY_TEXTVIEW_TEXT, 6,
    CppConsUI::Curses::Color::CYAN, CppConsUI::Curses::Color::BLACK);
  COLORSCHEME->setAttributesExt(SCHEME_TEXTVIEWWINDOW,
    CppConsUI::ColorScheme::PROPERTY_TEXTVIEW_TEXT, 7,
    CppConsUI::Curses::Color::WHITE, CppConsUI::Curses::Color::BLACK);

  declareBindable("textviewwindow", "toggle-scrollbar",
    sigc::mem_fun(this, &TestWindow::actionToggleScrollbar),
    InputProcessor::BINDABLE_NORMAL);
}

void TestWindow::actionToggleScrollbar()
{
  textview->setScrollBar(!textview->hasScrollBar());
}

void setupTest()
{
  KEYCONFIG->bindKey("textviewwindow", "toggle-scrollbar", "F1");

  // Create the main window.
  auto win = new TestWindow;
  win->show();
}

// vim: set tabstop=2 shiftwidth=2 textwidth=80 expandtab
