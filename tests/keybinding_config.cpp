#include <cppconsui/CoreManager.h>
#include <cppconsui/KeyConfig.h>
#include <cppconsui/Window.h>

#include <gettext.h>
#include <glib.h>

// TestWindow class
class TestWindow
: public CppConsUI::Window
{
public:
  /* This is a main window, make sure it can not be closed with ESC key by
   * overriding Close() method. */
  static TestWindow *Instance();
  virtual void Close() {}

  void SetConfigFile(const char *filename);
  bool Reconfig();
  void AddDefaultKeyBind(const char *context, const char *action,
      const char *key);
  void RegisterDefaultKeyBinds();
  bool ReconfigInternal();
  static void start_element_(GMarkupParseContext *context,
      const char *element_name, const char **attribute_names,
      const char **attribute_values, gpointer user_data,
      GError **error)
    { reinterpret_cast<TestWindow*>(user_data)->start_element(context,
        element_name, attribute_names, attribute_values, error); }
  void start_element(GMarkupParseContext *context,
      const char *element_name, const char **attribute_names,
      const char **attribute_values, GError **error);

protected:

private:
  struct DefaultKeyBind
  {
    std::string context;
    std::string action;
    std::string key;

    DefaultKeyBind(const char *context_, const char *action_,
        const char *key_) : context(context_), action(action_), key(key_) {}
  };
  typedef std::vector<DefaultKeyBind> DefaultKeyBinds;

  DefaultKeyBinds default_key_binds;

  // keybindings filename
  char *config;

  TestWindow();
  virtual ~TestWindow() {}
  TestWindow(const TestWindow&);
  TestWindow& operator=(const TestWindow&);
};

void TestWindow::SetConfigFile(const char *filename)
{
  if (config)
    g_free(config);

  config = g_strdup(filename);
}

bool TestWindow::Reconfig()
{
  g_assert(config);

  KEYCONFIG->Clear();

  if (!ReconfigInternal()) {
    // fallback to default key binds
    KEYCONFIG->Clear();
    RegisterDefaultKeyBinds();
    return false;
  }

  return true;
}

void TestWindow::AddDefaultKeyBind(const char *context, const char *action,
      const char *key)
{
  default_key_binds.push_back(DefaultKeyBind(context, action, key));
}

void TestWindow::RegisterDefaultKeyBinds()
{
  KEYCONFIG->Clear();

  for (DefaultKeyBinds::iterator i = default_key_binds.begin();
      i != default_key_binds.end(); i++)
    if (!KEYCONFIG->BindKey(i->context.c_str(), i->action.c_str(), i->key.c_str()))
      g_warning(_("Unrecognized key '%s' in default keybinds."),
          i->key.c_str());
}

bool TestWindow::ReconfigInternal()
{
  g_assert(config);

  // read the file contents
  char *contents;
  gsize length;
  GError *err = NULL;
  if (!g_file_get_contents(config, &contents, &length, &err)) {
    // generate default keybinding file
    err = NULL;
    GIOChannel *chan;
    if (!(chan = g_io_channel_new_file(config, "w", &err))) {
      if (err) {
        g_warning(_("Error opening keybinding file '%s' (%s)."),
            config, err->message);
        g_error_free(err);
        err = NULL;
      }
      else
        g_warning(_("Error opening keybinding file '%s'."), config);
      return false;
    }

#define ERROR()                                                   \
do {                                                              \
if (err) {                                                        \
  g_warning(_("Error writing to keybinding file '%s' (%s)."),     \
      config, err->message);                                      \
  g_error_free(err);                                              \
  err = NULL;                                                     \
}                                                                 \
else                                                              \
  g_warning(_("Error writing to keybinding file '%s'."), config); \
g_io_channel_unref(chan);                                         \
return false;                                                     \
} while (0)

    const char *buf = "<?xml version='1.0' encoding='UTF-8' ?>\n\n"
                      "<keyconfig version='1.0'>\n";
    if (g_io_channel_write_chars(chan, buf, -1, NULL, &err)
        != G_IO_STATUS_NORMAL)
      ERROR();

    for (DefaultKeyBinds::iterator i = default_key_binds.begin();
        i != default_key_binds.end(); i++) {
      char *buf2 = g_strdup_printf(
          "\t<bind context='%s' action='%s' key='%s'/>\n",
          i->context.c_str(), i->action.c_str(), i->key.c_str());
      GIOStatus s = g_io_channel_write_chars(chan, buf2, -1, NULL, &err);
      g_free(buf2);

      if (s != G_IO_STATUS_NORMAL)
        ERROR();
    }

    buf = "</keyconfig>\n";
    if (g_io_channel_write_chars(chan, buf, -1, NULL, &err)
        != G_IO_STATUS_NORMAL)
      ERROR();

#undef ERROR

    g_io_channel_unref(chan);

    if (!g_file_get_contents(config, &contents, &length, &err)) {
      if (err) {
        g_warning(_("Error reading keybinding file '%s' (%s)."), config,
            err->message);
        g_error_free(err);
        err = NULL;
      }
      else
        g_warning(_("Error reading keybinding file '%s'."), config);
      return false;
    }
  }

  // parse the file
  bool res = true;
  GMarkupParser parser = {start_element_, NULL /*end_element*/, NULL /*text*/,
    NULL /*passthrough*/, NULL /*error*/};
  GMarkupParseContext *context = g_markup_parse_context_new(&parser,
      G_MARKUP_PREFIX_ERROR_POSITION, this, NULL);
  if (!g_markup_parse_context_parse(context, contents, length, &err)
      || !g_markup_parse_context_end_parse(context, &err)) {
    if (err) {
      g_warning(_("Error parsing keybinding file '%s' (%s)."), config,
          err->message);
      g_error_free(err);
      err = NULL;
    }
    else
      g_warning(_("Error parsing keybinding file '%s'."), config);
    res = false;
  }
  g_markup_parse_context_free(context);
  g_free(contents);

  return res;
}

void TestWindow::start_element(GMarkupParseContext *context,
    const char *element_name, const char **attribute_names,
    const char **attribute_values, GError **error)
{
  const GSList *stack = g_markup_parse_context_get_element_stack(context);
  guint size = g_slist_length(const_cast<GSList*>(stack));
  if (size == 1) {
    if (strcmp(element_name, "keyconfig")) {
      *error = g_error_new(G_MARKUP_ERROR, G_MARKUP_ERROR_PARSE,
          _("Expected 'keyconfig' element, found '%s'"), element_name);
    }
  }
  else if (size == 2) {
    if (strcmp(element_name, "bind")) {
      *error = g_error_new(G_MARKUP_ERROR, G_MARKUP_ERROR_PARSE,
          _("Expected 'bind' element, found '%s'"), element_name);
      return;
    }

    const char *context;
    const char *action;
    const char *key;
    if (!g_markup_collect_attributes(element_name, attribute_names,
          attribute_values, error,
          G_MARKUP_COLLECT_STRING, "context", &context,
          G_MARKUP_COLLECT_STRING, "action", &action,
          G_MARKUP_COLLECT_STRING, "key", &key,
          G_MARKUP_COLLECT_INVALID))
      return;

    if (!KEYCONFIG->BindKey(context, action, key)) {
      *error = g_error_new(G_MARKUP_ERROR, G_MARKUP_ERROR_INVALID_CONTENT,
          _("Unrecognized key '%s'"), key);
      return;
    }
  }
  else
    *error = g_error_new(G_MARKUP_ERROR, G_MARKUP_ERROR_PARSE,
        _("Unexpected element '%s'"), element_name);
}

TestWindow *TestWindow::Instance()
{
  static TestWindow instance;
  return &instance;
}

TestWindow::TestWindow()
: CppConsUI::Window(0, 0, AUTOSIZE, AUTOSIZE)
{
  AddWidget(*(new CppConsUI::Label("Press F10 to quit.")), 1, 1);

  AddDefaultKeyBind("testapp", "quit", "F10");

  AddDefaultKeyBind("button", "activate", "Enter");

  AddDefaultKeyBind("checkbox", "toggle", "Enter");

  AddDefaultKeyBind("container", "focus-previous", "Shift-Tab");
  AddDefaultKeyBind("container", "focus-next", "Tab");
  AddDefaultKeyBind("container", "focus-up", "Up");
  AddDefaultKeyBind("container", "focus-down", "Down");
  AddDefaultKeyBind("container", "focus-left", "Left");
  AddDefaultKeyBind("container", "focus-right", "Right");
  AddDefaultKeyBind("container", "focus-page-up", "PageUp");
  AddDefaultKeyBind("container", "focus-page-down", "PageDown");
  AddDefaultKeyBind("container", "focus-begin", "Home");
  AddDefaultKeyBind("container", "focus-end", "End");

  AddDefaultKeyBind("coremanager", "redraw-screen", "Ctrl-l");

  AddDefaultKeyBind("textentry", "cursor-right", "Right");
  AddDefaultKeyBind("textentry", "cursor-left", "Left");
  AddDefaultKeyBind("textentry", "cursor-down", "Down");
  AddDefaultKeyBind("textentry", "cursor-up", "Up");
  AddDefaultKeyBind("textentry", "cursor-right-word", "Ctrl-Right");
  AddDefaultKeyBind("textentry", "cursor-left-word", "Ctrl-Left");
  AddDefaultKeyBind("textentry", "cursor-end", "End");
  AddDefaultKeyBind("textentry", "cursor-begin", "Home");
  AddDefaultKeyBind("textentry", "delete-char", "Delete");
  AddDefaultKeyBind("textentry", "backspace", "Backspace");

  AddDefaultKeyBind("textentry", "delete-word-end", "Ctrl-Delete");
  AddDefaultKeyBind("textentry", "delete-word-begin", "Ctrl-Backspace");
  /// @todo enable
  /*
  AddDefaultKeyBind("textentry", "toggle-overwrite", "Insert");
  */

  AddDefaultKeyBind("textentry", "activate", "Enter");

  AddDefaultKeyBind("textview", "scroll-up", "PageUp");
  AddDefaultKeyBind("textview", "scroll-down", "PageDown");

  AddDefaultKeyBind("treeview", "fold-subtree", "-");
  AddDefaultKeyBind("treeview", "unfold-subtree", "+");

  AddDefaultKeyBind("window", "close-window", "Escape");

  // Set the file where to load/save the binds
  SetConfigFile("binds.xml");

  // Load the binds from the config file. If no binds file exists, then
  // write one with the default settings.
  Reconfig();
}

// TestApp class
class TestApp
: public CppConsUI::InputProcessor
{
public:
  static TestApp *Instance();

  void Run();

  // ignore every message
  static void g_log_func_(const gchar * /*log_domain*/,
      GLogLevelFlags /*log_level*/, const gchar * /*message*/,
      gpointer /*user_data*/)
    {}

protected:

private:
  CppConsUI::CoreManager *mngr;

  TestApp();
  TestApp(const TestApp&);
  TestApp& operator=(const TestApp&);
  virtual ~TestApp() {}
};

TestApp *TestApp::Instance()
{
  static TestApp instance;
  return &instance;
}

TestApp::TestApp()
{
  mngr = CppConsUI::CoreManager::Instance();

  g_log_set_default_handler(g_log_func_, this);

  DeclareBindable("testapp", "quit", sigc::mem_fun(mngr,
        &CppConsUI::CoreManager::QuitMainLoop),
      InputProcessor::BINDABLE_OVERRIDE);
}

void TestApp::Run()
{
  mngr->AddWindow(*TestWindow::Instance());
  mngr->SetTopInputProcessor(*this);
  mngr->EnableResizing();
  mngr->StartMainLoop();
}

// main function
int main()
{
  setlocale(LC_ALL, "");

  TestApp *app = TestApp::Instance();
  app->Run();

  return 0;
}
