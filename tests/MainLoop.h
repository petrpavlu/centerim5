#include <cppconsui/CppConsUI.h>

namespace MainLoop
{

void init();
void run();
void quit();
void finalize();

unsigned input_add_cppconsui(int fd, CppConsUI::InputCondition condition,
    CppConsUI::InputFunction function, void *data);
unsigned timeout_add_cppconsui(unsigned interval,
    CppConsUI::SourceFunction function, void *data);
bool timeout_remove_cppconsui(unsigned handle);
bool input_remove_cppconsui(unsigned handle);

} // namespace MainLoop

/* vim: set tabstop=2 shiftwidth=2 textwidth=78 expandtab : */
