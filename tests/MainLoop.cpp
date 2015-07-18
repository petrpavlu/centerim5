#include <MainLoop.h>

#include <glib.h>
// TODO Rewrite this code without use of GLib.

namespace MainLoop {

struct IOClosureCppConsUI {
  CppConsUI::InputFunction function;
  guint result;
  gpointer data;

  IOClosureCppConsUI() : function(NULL), result(0), data(NULL) {}
};

struct SourceClosureCppConsUI {
  CppConsUI::SourceFunction function;
  void *data;

  SourceClosureCppConsUI() : function(NULL), data(NULL) {}
};

GMainLoop *mainloop = NULL;

static void log_func_glib(const gchar * /*log_domain*/,
  GLogLevelFlags /*log_level*/, const gchar * /*message*/,
  gpointer /*user_data*/)
{
  // ignore all messages
}

static gboolean io_input_cppconsui(
  GIOChannel *source, GIOCondition condition, gpointer data)
{
  IOClosureCppConsUI *closure = static_cast<IOClosureCppConsUI *>(data);
  int cppconsui_cond = 0;

  if (condition & G_IO_IN)
    cppconsui_cond |= CppConsUI::INPUT_CONDITION_READ;
  if (condition & G_IO_OUT)
    cppconsui_cond |= CppConsUI::INPUT_CONDITION_WRITE;

  closure->function(g_io_channel_unix_get_fd(source),
    static_cast<CppConsUI::InputCondition>(cppconsui_cond), closure->data);

  return TRUE;
}

static void io_destroy_cppconsui(gpointer data)
{
  delete static_cast<IOClosureCppConsUI *>(data);
}

static gboolean timeout_function_cppconsui(gpointer data)
{
  SourceClosureCppConsUI *closure = static_cast<SourceClosureCppConsUI *>(data);
  return closure->function(closure->data);
}

static void timeout_destroy_cppconsui(gpointer data)
{
  delete static_cast<SourceClosureCppConsUI *>(data);
}

void init()
{
  g_assert(!mainloop);
  mainloop = g_main_loop_new(NULL, FALSE);

  g_log_set_default_handler(log_func_glib, NULL);
}

void run()
{
  g_assert(mainloop);
  g_main_loop_run(mainloop);
}

void quit()
{
  g_assert(mainloop);
  g_main_loop_quit(mainloop);
}

void finalize()
{
  g_assert(mainloop);
  g_main_loop_unref(mainloop);
}

unsigned input_add_cppconsui(int fd, CppConsUI::InputCondition condition,
  CppConsUI::InputFunction function, void *data)
{
  IOClosureCppConsUI *closure = new IOClosureCppConsUI;
  GIOChannel *channel;
  int cond = 0;

  closure->function = function;
  closure->data = data;

  if (condition & CppConsUI::INPUT_CONDITION_READ)
    cond |= G_IO_IN | G_IO_HUP | G_IO_ERR;
  if (condition & CppConsUI::INPUT_CONDITION_WRITE)
    cond |= G_IO_OUT | G_IO_HUP | G_IO_ERR | G_IO_NVAL;

  channel = g_io_channel_unix_new(fd);
  closure->result = g_io_add_watch_full(channel, G_PRIORITY_DEFAULT,
    static_cast<GIOCondition>(cond), io_input_cppconsui, closure,
    io_destroy_cppconsui);

  g_io_channel_unref(channel);
  return closure->result;
}

unsigned timeout_add_cppconsui(
  unsigned interval, CppConsUI::SourceFunction function, void *data)
{
  SourceClosureCppConsUI *closure = new SourceClosureCppConsUI;
  closure->function = function;
  closure->data = data;

  return g_timeout_add_full(G_PRIORITY_DEFAULT, interval,
    timeout_function_cppconsui, closure, timeout_destroy_cppconsui);
}

bool timeout_remove_cppconsui(unsigned handle)
{
  return g_source_remove(handle);
}

bool input_remove_cppconsui(unsigned handle)
{
  return g_source_remove(handle);
}

} // namespace MainLoop

/* vim: set tabstop=2 shiftwidth=2 textwidth=80 expandtab : */
