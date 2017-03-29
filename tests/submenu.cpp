#include <cppconsui/Button.h>
#include <cppconsui/Label.h>
#include <cppconsui/MenuWindow.h>
#include <cppconsui/Window.h>

#include <string>

// TestWindow class
class TestWindow : public CppConsUI::Window {
public:
  TestWindow();
  virtual ~TestWindow() override;

private:
  CppConsUI::MenuWindow *menu;

  CONSUI_DISABLE_COPY(TestWindow);
};

TestWindow::TestWindow() : CppConsUI::Window(0, 0, AUTOSIZE, AUTOSIZE)
{
  setClosable(false);

  addWidget(*(new CppConsUI::Label("Press F10 to quit.")), 1, 1);

  CppConsUI::Button *button = new CppConsUI::Button("Open Menu...");
  addWidget(*button, 1, 3);

  menu = new CppConsUI::MenuWindow(*button, AUTOSIZE, AUTOSIZE);
  menu->setHideOnClose(true);

  button->signal_activate.connect(
    sigc::hide(sigc::mem_fun(menu, &CppConsUI::MenuWindow::show)));

  menu->appendItem(
    "Item 1", sigc::hide(sigc::mem_fun(menu, &CppConsUI::MenuWindow::hide)));
  menu->appendItem(
    "Item 2", sigc::hide(sigc::mem_fun(menu, &CppConsUI::MenuWindow::hide)));
  menu->appendItem(
    "Item 3", sigc::hide(sigc::mem_fun(menu, &CppConsUI::MenuWindow::hide)));
  menu->appendItem(
    "Item 4", sigc::hide(sigc::mem_fun(menu, &CppConsUI::MenuWindow::hide)));
  menu->appendSeparator();

  auto submenu = new CppConsUI::MenuWindow(*button, AUTOSIZE, AUTOSIZE);
  submenu->appendItem(
    "Item 1", sigc::hide(sigc::mem_fun(submenu, &CppConsUI::MenuWindow::hide)));
  submenu->appendItem(
    "Item 2", sigc::hide(sigc::mem_fun(submenu, &CppConsUI::MenuWindow::hide)));
  submenu->appendItem(
    "Item 3", sigc::hide(sigc::mem_fun(submenu, &CppConsUI::MenuWindow::hide)));
  submenu->appendItem(
    "Item 3", sigc::hide(sigc::mem_fun(submenu, &CppConsUI::MenuWindow::hide)));
  menu->appendSubMenu("First submenu", *submenu);

  submenu = new CppConsUI::MenuWindow(*button, AUTOSIZE, AUTOSIZE);
  submenu->appendItem(
    "Item 1", sigc::hide(sigc::mem_fun(menu, &CppConsUI::MenuWindow::hide)));
  submenu->appendItem(
    "Item 2", sigc::hide(sigc::mem_fun(menu, &CppConsUI::MenuWindow::hide)));
  submenu->appendItem(
    "Item 3", sigc::hide(sigc::mem_fun(menu, &CppConsUI::MenuWindow::hide)));
  submenu->appendItem(
    "Item 4", sigc::hide(sigc::mem_fun(menu, &CppConsUI::MenuWindow::hide)));
  menu->appendSubMenu("Second submenu", *submenu);
}

TestWindow::~TestWindow()
{
  delete menu;
}

void setupTest()
{
  // Create the main window.
  auto win = new TestWindow;
  win->show();
}

// vim: set tabstop=2 shiftwidth=2 textwidth=80 expandtab
