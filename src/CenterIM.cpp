/*
 * Copyright (C) 2007 by Mark Pustjens <pustjens@dds.nl>
 *
 * This file is part of CenterIM.
 *
 * CenterIM is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * CenterIM is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * */

#include "CenterIM.h"
#include "AccountStatusMenu.h"
#include "GeneralMenu.h"
#include "Defines.h"

#include "CIMWindowManager.h"

#include <libpurple/prefs.h>
#include <libpurple/core.h>
#include <libpurple/plugin.h>
#include <libpurple/util.h>
#include <libpurple/pounce.h>
#include <libpurple/debug.h>
#include <libpurple/savedstatuses.h>
#include <glibmm/main.h>
#include <signal.h>

#include <libintl.h>
#include <sys/ioctl.h>
#include <cstring>
#include <cerrno>

//TODO move inside CenterIM object
static PurpleDebugUiOps logbuf_debug_ui_ops =
{
	CenterIM::tmp_purple_print_,
	CenterIM::tmp_isenabled_,
	NULL,
	NULL,
	NULL,
	NULL
};

CenterIM* CenterIM::instance = NULL;
std::vector<CenterIM::LogBufferItem> *CenterIM::logbuf = NULL;

typedef void (*signal_t)(int);
signal_t old_handler = NULL;

void CenterIM::signal_handler(int signum)
{
	struct winsize size;

	if (signum == SIGWINCH)
	{
		if(ioctl(fileno(stdout), TIOCGWINSZ, &size) == 0) {
			resizeterm(size.ws_row, size.ws_col);
			wrefresh(curscr);
			CenterIM::Instance()->ScreenResized();
		}
	}
}


CenterIM* CenterIM::Instance()
{
	if (!instance) instance = new CenterIM();
	return instance;
}

void CenterIM::Delete(void)
{
	if (instance) {
		delete instance;
		instance = NULL;
	}
}

void CenterIM::Run(void)
{
  register_resize_handler();
  gmainloop->run();
}

void CenterIM::Quit(void)
{
  unregister_resize_handler();
  gmainloop->quit();
}

void CenterIM::ScreenResized(void)
{
	log->Write(Log::Level_debug, "CenterIM::ScreenResized()\n");
	windowmanager->ScreenResized();
}

void CenterIM::SetLocale(const char *locale)
{
	this->locale = locale;
	log->Write(Log::Level_debug, "locale: %s\n", locale);
}

//TODO: move next two static structs inside the CenterIM object
static PurpleCoreUiOps centerim_core_ui_ops =
{
	NULL, //CenterIM::ui_prefs_init_,
	NULL, //CenterIM::debug_ui_init_,   
	NULL, //CenterIM::ui_init_,
	CenterIM::ui_uninit_,
	NULL, //CenterIM::get_ui_info,
	NULL,
	NULL,
	NULL,
};

static PurpleEventLoopUiOps centerim_glib_eventloops =
{
	CenterIM::timeout_add,
	CenterIM::timeout_remove,
	CenterIM::input_add,
	CenterIM::input_remove,
	NULL, //CenterIM::input_get_error,
	NULL, //CenteRIM::timeout_add_seconds,
	NULL,
	NULL,
	NULL,
};

CenterIM::CenterIM()
: locale(NULL)
, channel(NULL)
, channel_id(0)
{
	/* Declaring bindables must be done in CenterIM::io_init()
	 * */

	char *path;
	/* set the configuration file location */
	path = g_build_filename(purple_home_dir(), CIM_CONFIG_PATH, NULL);
	purple_util_set_user_dir(path);
	g_free(path);

	/* This does not disable debugging, but rather it disables printing to
	 * stdout. Don't change this to TRUE or things will get messy. */
	purple_debug_set_enabled(FALSE);

	/* This catches and buffers libpurple debug messages until the Log object
	 * can be instantiated
	 * */
	purple_debug_set_ui_ops(&logbuf_debug_ui_ops);

	/* Set the core-uiops, which is used to
	 *      - initialize the ui specific preferences.
	 *      - initialize the debug ui.
	 *      - initialize the ui components for all the modules.
	 *      - uninitialize the ui components for all the modules when the core terminates.
	 * */
	purple_core_set_ui_ops(&centerim_core_ui_ops);
	/* Set the uiops for the eventloop. */
	purple_eventloop_set_ui_ops(&centerim_glib_eventloops);

	/* In case we ever write centerim specific plugins */
	path = g_build_filename(purple_user_dir(), "plugins", NULL);
	purple_plugins_add_search_path(path);
	g_free(path);

	if (!purple_core_init("centerim")) {
		/* can't do much without libpurple */
		throw (EXCEPTION_PURPLE_CORE_INIT);
	}

	/* normally these three functions are called 
	 * by centerim_core_ui_ops(), but since we need
	 * a CenterIM object for them we have to call them
	 * manually after the core has been initialized
	 * */
	windowmanager = CIMWindowManager::Instance();
	ui_prefs_init();
	debug_ui_init();
	ui_init();
	io_init();

	/* create a new loop */
	gmainloop = Glib::MainLoop::create();
}

CenterIM::~CenterIM()
{
	/* clean up */
	io_uninit();
	purple_core_quit();
	windowmanager->Delete();
}

void CenterIM::register_resize_handler(void)
{
	old_handler = signal(SIGWINCH, &signal_handler);
}

void CenterIM::unregister_resize_handler(void)
{
	signal(SIGWINCH, old_handler);
	old_handler = SIG_DFL;
}

void CenterIM::ui_prefs_init(void)
{
	conf = Conf::Instance();
}

void CenterIM::debug_ui_init(void)
{
	std::vector<LogBufferItem>::iterator i;
	
	windowmanager->Add(log = Log::Instance());

	if (logbuf) {
		for (i = logbuf->begin(); i != logbuf->end(); i++) {
			log->purple_print(i->level, i->category, i->arg_s);
			g_free(i->category);
			g_free(i->arg_s);
		}

		delete logbuf;
		logbuf = NULL;
	}
}

void CenterIM::ui_init(void)
{
	ScreenResized();
	//TODO when these objecs are windows, add them to the windowmanager
	accounts = Accounts::Instance();
//	windowmanager->Add(accounts = new Accounts());
//	windowmanager->Add(connections = new Connections());
	windowmanager->Add(buddylist = BuddyList::Instance());
//	windowmanager->Add(conversations = Conversations::Instance());
//	windowmanager->Add(transfers = new Transfers());

	connections = new Connections();
	conversations = Conversations::Instance();
	transfers = new Transfers();
}

void CenterIM::ui_uninit(void)
{
	//TODO when these objecs are windows, remove them from the windowmanager
//	windowmanager->Remove(accounts);
	if (accounts) { accounts->Delete(); accounts = NULL; }

//	windowmanager->Remove(connections);
	delete connections; connections = NULL;

	windowmanager->Remove(buddylist);
	if (buddylist) { buddylist->Delete(); buddylist = NULL; }

//	windowmanager->Remove(conversations);
	if (conversations) conversations->Delete(); conversations = NULL;

//	windowmanager->Remove(transfers);
	delete transfers; transfers = NULL;

	if (conf) conf->Delete(); conf = NULL;
}

guint CenterIM::timeout_add(guint interval, GSourceFunc function, gpointer data)
{
	return g_timeout_add(interval, function, data);
}

gboolean CenterIM::timeout_remove(guint handle)
{
	return g_source_remove(handle);
}

#define PURPLE_GLIB_READ_COND  (G_IO_IN | G_IO_HUP | G_IO_ERR)
#define PURPLE_GLIB_WRITE_COND (G_IO_OUT | G_IO_HUP | G_IO_ERR | G_IO_NVAL)

//TODO move inside CenterIM object
typedef struct _CenterIMGLibIOClosure {
	PurpleInputFunction function;
	guint result;
	gpointer data;
} CenterIMGLibIOClosure;


guint CenterIM::input_add(int fd, PurpleInputCondition condition,
	PurpleInputFunction function, gpointer data)
{
	CenterIMGLibIOClosure *closure = g_new0(CenterIMGLibIOClosure, 1);
	GIOChannel *channel;
	int cond = 0;

	closure->function = function;
	closure->data = data;

	if (condition & PURPLE_INPUT_READ)
		cond |= PURPLE_GLIB_READ_COND;
	if (condition & PURPLE_INPUT_WRITE)
		cond |= PURPLE_GLIB_WRITE_COND;

	channel = g_io_channel_unix_new(fd);
	closure->result = g_io_add_watch_full(channel, G_PRIORITY_DEFAULT, (GIOCondition)cond,
				purple_glib_io_input, closure, purple_glib_io_destroy);

	g_io_channel_unref(channel);
	return closure->result;
}

gboolean CenterIM::input_remove(guint handle)
{
	return g_source_remove(handle);
}

gboolean CenterIM::purple_glib_io_input(GIOChannel *source, GIOCondition condition, gpointer data)
{
	CenterIMGLibIOClosure *closure = (CenterIMGLibIOClosure*)data;
	int purple_cond = 0;

	if (condition & PURPLE_GLIB_READ_COND)
		purple_cond |= PURPLE_INPUT_READ;
	if (condition & PURPLE_GLIB_WRITE_COND)
		purple_cond |= PURPLE_INPUT_WRITE;

	closure->function(closure->data, g_io_channel_unix_get_fd(source),
			  (PurpleInputCondition)purple_cond);

	return TRUE;
}

void CenterIM::purple_glib_io_destroy(gpointer data)
{
	g_free(data);
}

/* This catches and buffers libpurple debug messages until the Log object
 * can be instantiated
 * */
void CenterIM::tmp_purple_print_(PurpleDebugLevel level, const char *category, const char *arg_s)
{
	if (!logbuf)
		logbuf = new std::vector<LogBufferItem>;

	LogBufferItem item;
	item.level = level;
	item.category = g_strdup(category);
	item.arg_s = g_strdup(arg_s);
	logbuf->push_back(item);
}

/* Xerox (finch saves us a little of work here) */
void CenterIM::io_init(void)
{
	const gchar *context = "centerim";
	keys = Keys::Instance();

	/* Key combinations */
	DeclareBindable(context, "quit", sigc::mem_fun(this, &CenterIM::Quit),
		_("Quit CenterIM."), InputProcessor::Bindable_Override);
	DeclareBindable(context, "accountstatusmenu", sigc::mem_fun(this, &CenterIM::OpenAccountStatusMenu),
		_("Open the account status menu."), InputProcessor::Bindable_Override);
	DeclareBindable(context, "generalmenu", sigc::mem_fun(this, &CenterIM::OpenGeneralMenu),
		_("Open the general menu."), InputProcessor::Bindable_Override);

	//TODO get real binding from config
	BindAction(context, "quit", Keys::Instance()->Key_ctrl_q(), true);
	BindAction(context, "accountstatusmenu", Keys::Instance()->Key_f3(), true);
	BindAction(context, "generalmenu", Keys::Instance()->Key_f4(), true);

	SetInputChild(windowmanager);

	curs_set(0);
	keypad(stdscr, 1); /* without this, some keys are not translated correctly */
	nonl();
	raw();

	// print the currently used character set
	g_get_charset(&charset);
	log->Write(Log::Level_debug, "charset: %s\n", charset);
	if ((converter = g_iconv_open("UTF-8", charset)) == (GIConv) -1) {
		log->Write(Log::Level_error, "IConv initialization failed (%s)\n", g_strerror(errno));
		throw EXCEPTION_ICONV_INIT;
	}

	channel = g_io_channel_unix_new(STDIN_FILENO);
	// set channel encoding to NULL so it can be unbuffered
	g_io_channel_set_encoding(channel, NULL, NULL);
	g_io_channel_set_buffered(channel, FALSE);
	g_io_channel_set_close_on_unref(channel, TRUE);

	channel_id = g_io_add_watch_full(channel, G_PRIORITY_HIGH,
		(GIOCondition)(G_IO_IN | G_IO_HUP | G_IO_ERR | G_IO_PRI), io_input_, this, NULL);

	g_io_add_watch_full(channel, G_PRIORITY_HIGH,
		(G_IO_NVAL), io_input_error_, this, NULL);

	g_io_channel_unref(channel);

	log->Write(Log::Level_debug, "IO initialized\n");
}

void CenterIM::io_uninit(void)
{
	g_source_remove(channel_id);
	channel_id = 0;
	g_io_channel_unref(channel);
	channel = NULL;

	g_iconv_close(converter);

	if (keys) {
		keys->Delete();
		keys = NULL;
	}
}

gboolean CenterIM::io_input_error(GIOChannel *source, GIOCondition cond)
{
	// log an error and bail out if we lost stdin
	log->Write(Log::Level_error, "stdin lost!\n");
	Quit();

	return TRUE;
}

gboolean CenterIM::io_input(GIOChannel *source, GIOCondition cond)
{
	// TODO is this reasonable? (pasting needs a somewhat larger buffer to be efficient)
	gchar buf[64];
	gsize rd;
	GError *err = NULL;
	// buffer for saving a part of char from a previous reading, max char len
	// in bytes (currently 5 bytes for UTF-EBCDIC), size must be always
	// <= sizeof(buf)
	static gchar buf_part[5];
	static gsize buf_part_len;
	// every character in UTF-8 can be encoded with 4 bytes so this is enough
	// room for any conversion
	gchar converted[4 * sizeof(buf) + 1];
	gsize converted_left = sizeof(converted);
	gchar *pbuf = buf;
	gchar *pconverted = converted;
	static std::string input;
	int eaten;

	if (buf_part_len) {
		memcpy(buf, buf_part, buf_part_len);
	}

	if (g_io_channel_read_chars(source, buf + buf_part_len,
				sizeof(buf) - buf_part_len, &rd, &err) != G_IO_STATUS_NORMAL) {
		log->Write(Log::Level_error, "%s\n", err->message);
		g_error_free(err);
		return TRUE;
	}
	rd += buf_part_len;
	buf_part_len = 0;

	// we don't need to care much about this, GLib will notice us again that
	// there are still bytes left
	if (sizeof(buf) == rd) {
		log->Write(Log::Level_debug, "input buffer full\n");
	}

	/* TODO Fix the input string.
	 * Some keys generate bytestrings which are different from the strings
	 * terminfo/ncurses expects
	 * */
	//keys->Refine(buf, rd);

	// convert data from user charset to UTF-8
	g_iconv(converter, NULL, NULL, NULL, NULL);
	errno = 0;
	if (g_iconv(converter, &pbuf, &rd, &pconverted, &converted_left) == (gsize) -1) {
		switch (errno) {
			case EILSEQ:
				log->Write(Log::Level_error, _("IConv error: %s\n"), g_strerror(errno));
				return TRUE;
			case EINVAL:
				// incomplete multibyte sequence is encountered in the input,
				// save these bytes for further reading
				memcpy(buf_part, pbuf, rd);
				buf_part_len = rd;
				break;
			default:
				log->Write(Log::Level_error, _("Unexcepted IConv error: %s\n"), g_strerror(errno));
				return TRUE;
		}
	}
	*pconverted = '\0';

	/* Below this line we assume all input has been converted to UTF-8 encoded
	 * multibyte string
	 * */

	// too noisy even for debug level
	/*
	for (gchar *iter = converted; *iter != '\0'; iter = g_utf8_next_char(iter)) {
		log->Write(Log::Level_debug, "input: U+%04"G_GINT32_FORMAT"X\n", g_utf8_get_char(iter));
	}
	*/

	input.append(converted);

	while (input.size()) {
		eaten = ProcessInput(input.c_str(), input.size());
		if (eaten < 0) {
			return TRUE;
		} else if (eaten == 0) {
			//TODO find out if there is a more intelligent way
			//to discard input. eg: key_left is 6 bytes, so
			//remove 6 bytes (*must* be *fast*)
			eaten = 1;
		}
		input.erase(0, eaten);
	}

	return TRUE;
}

void CenterIM::OpenAccountStatusMenu(void)
{
	//TODO get coords from config
	windowmanager->Add(new AccountStatusMenu(40,0, 40, 20, LineStyle::LineStyleDefault()));
}

void CenterIM::OpenGeneralMenu(void)
{
	//TODO get coords from config
	windowmanager->Add(new GeneralMenu(40, 0, 40, 22, LineStyle::LineStyleDefault()));
}

