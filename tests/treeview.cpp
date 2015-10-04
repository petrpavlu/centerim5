#include <cppconsui/Button.h>
#include <cppconsui/Label.h>
#include <cppconsui/TreeView.h>
#include <cppconsui/Window.h>

// TestWindow class
class TestWindow : public CppConsUI::Window {
public:
  TestWindow();
  virtual ~TestWindow() override {}

private:
  CONSUI_DISABLE_COPY(TestWindow);
};

TestWindow::TestWindow() : Window(0, 0, AUTOSIZE, AUTOSIZE)
{
  setClosable(false);

  addWidget(*(new CppConsUI::Label(20, 1, "Press F10 to quit.")), 1, 1);

  auto tree = new CppConsUI::TreeView(30, 12);
  addWidget(*tree, 1, 3);
  setInputChild(*tree);

  CppConsUI::TreeView::NodeReference node, node2;

  node = tree->appendNode(
    tree->getRootNode(), *(new CppConsUI::Button("Button node A")));
  node2 = tree->appendNode(node, *(new CppConsUI::Button("Button node A-1")));
  tree->appendNode(node2, *(new CppConsUI::Button("Button node A-1-a")));
  tree->appendNode(node2, *(new CppConsUI::Button("Button node A-1-b")));
  tree->appendNode(node2, *(new CppConsUI::Button("Button node A-1-c")));
  tree->appendNode(node, *(new CppConsUI::Button("Button node A-2")));
  tree->appendNode(node, *(new CppConsUI::Button("Button node A-3")));

  node = tree->appendNode(
    tree->getRootNode(), *(new CppConsUI::Label("Label node B")));
  tree->appendNode(node, *(new CppConsUI::Label("Label node B-1")));
  tree->appendNode(node, *(new CppConsUI::Label("Label node B-2")));
  tree->appendNode(node, *(new CppConsUI::Label("Label node B-3")));

  node = tree->appendNode(
    tree->getRootNode(), *(new CppConsUI::Button("Button node C")));
  tree->appendNode(node, *(new CppConsUI::Button("Button node C-1")));
  tree->appendNode(node, *(new CppConsUI::Button("Button node C-2")));
  tree->appendNode(node, *(new CppConsUI::Button("Button node C-3")));
}

void setupTest()
{
  // Create the main window.
  auto win = new TestWindow;
  win->show();
}

// vim: set tabstop=2 shiftwidth=2 textwidth=80 expandtab
