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

//TODO: configurable path using ./configure
#define CIM_CONFIG_PATH		".centerim"

#define CONF_PLUGIN_SAVE_PREF	"/centerim/plugins/loaded"

#define EXCEPTION_NONE			0
#define EXCEPTION_PURPLE_CORE_INIT	100

#define INPUT_BUF_SIZE			64	//TODO is this reasonable? (pasting needs a somewhat larger buffer to be efficient)

#include "CenterIM.h"
#include "AccountStatusMenu.h"
#include "GeneralMenu.h"

#include <cppconsui/WindowManager.h>

#include <libpurple/prefs.h>
#include <libpurple/core.h>
#include <libpurple/plugin.h>
#include <libpurple/util.h>
#include <libpurple/pounce.h>
#include <libpurple/debug.h>
#include <libpurple/savedstatuses.h>
#include <glib.h>

#include <libintl.h>

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
std::vector<CenterIM::logbuf_item>* CenterIM::logbuf = NULL;

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
	g_main_loop_run(gmainloop);
}

void CenterIM::Quit(void)
{
	g_main_loop_quit(gmainloop);
}

//TODO: move next two static structs inside the CenterIM object
static PurpleCoreUiOps centerim_core_ui_ops =
{
	NULL, //CenterIM::ui_prefs_init_,
	NULL, //CenterIM::debug_ui_init_,   
	NULL, //CenterIM::ui_init_,
	CenterIM::ui_uninit_,
};

static PurpleEventLoopUiOps centerim_glib_eventloops =
{
	CenterIM::timeout_add,
	CenterIM::timeout_remove,
	CenterIM::input_add,
	CenterIM::input_remove,
	NULL
};

CenterIM::CenterIM()
: channel(NULL)
, channel_id(0)
, locale(NULL)
, charset(NULL)
{
	/* Declaring bindables must be done in CenterIM::io_init()
	 * */

	/* store the currently used character set*/
	g_get_charset(&charset);

	char *path;
	/* set the configuration file location */
	path = g_build_filename(purple_home_dir(), CIM_CONFIG_PATH, NULL);
	purple_util_set_user_dir(path);
	g_free(path);

	/* This does not disable debugging, but rather it disables printing to stdout */
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
	windowmanager = WindowManager::Instance();
	ui_prefs_init();
	debug_ui_init();
	ui_init();
	io_init();

	/* create a new loop */
	//TODO perhaps replace by Glib::Main ?? (does that even exist?)
	gmainloop = g_main_loop_new(NULL, FALSE);
}

CenterIM::~CenterIM()
{
	/* clean up */
	io_uninit();
	purple_core_quit();
	log->Delete();
	windowmanager->Delete();
}

void CenterIM::ui_prefs_init(void)
{
	conf = Conf::Instance();
}

void CenterIM::debug_ui_init(void)
{
	std::vector<logbuf_item>::iterator i;
	logbuf_item *item;
	
	windowmanager->Add(log = Log::Instance());

	if (logbuf) {
		for (i = logbuf->begin(); i != logbuf->end(); i++) {
			item = &(*i);
			log->purple_print(item->level, item->category, item->arg_s);
			g_free(item->category);
			g_free(item->arg_s);
		}

		delete logbuf;
		logbuf = NULL;
	}
}

void CenterIM::ui_init(void)
{
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
		logbuf = new std::vector<logbuf_item>;

	logbuf_item item;
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
	cbreak();
	raw();
        g_io_channel_set_encoding(channel, locale, NULL); //TODO how to convert input to UTF-8 automatically? perhaps in io_input
        g_io_channel_set_buffered(channel, FALSE); //TODO not needed?
//        g_io_channel_set_flags(channel, G_IO_FLAG_NONBLOCK, NULL ); //TODO not needed?
//g_printerr("encoding = %s\n", g_io_channel_get_encoding(channel));        

	channel = g_io_channel_unix_new(STDIN_FILENO);
	g_io_channel_set_close_on_unref(channel, TRUE);

	channel_id = g_io_add_watch_full(channel,  G_PRIORITY_HIGH,
		(GIOCondition)(G_IO_IN | G_IO_HUP | G_IO_ERR | G_IO_PRI), io_input_, this, NULL);

	g_io_add_watch_full(channel, G_PRIORITY_HIGH,
		(G_IO_NVAL), io_input_error_, this, NULL);

	g_io_channel_unref(channel);

	//doing this here breaks things
	//g_printerr("gntmain: setting up IO\n"); //TODO is this an error??
}

void CenterIM::io_uninit(void)
{
	g_source_remove(channel_id);
	channel_id = 0;
	g_io_channel_unref(channel);
	channel = NULL;
	if (keys) keys->Delete(); keys = NULL;
}

gboolean CenterIM::io_input_error(GIOChannel *source, GIOCondition cond)
{
	g_source_remove(channel_id);
	g_io_channel_unref(source);

	//TODO log an error, apparantly we lost stdin, which shoudn't happen
	//we also try to reopen stdin

	channel = NULL;
	io_init();
	return TRUE;
}

gboolean CenterIM::io_input(GIOChannel *source, GIOCondition cond)
{
	static std::string input;
	char buf[INPUT_BUF_SIZE];
	int rd = read(STDIN_FILENO, buf, INPUT_BUF_SIZE-1), eaten;
	//TODO convert to UTF-8 here?

	/* Below this line we assume all input has been converted
	 * to and UTF-8 encoded multibyte string
	 * */

	if (rd < 0) {
		//TODO what to do on error?
	} else if (rd == 0) {
		//TODO this should not happen, should it?
	} else if (rd == INPUT_BUF_SIZE-1) {
		//TODO log an error about exausted read buffer
		//if this ever happens, it would be best to
		//implement a dynamically sized buffer
	}

	/* Fix the input string.
	 * Some keys generate bytestrings which are different
	 * from the strings  terminfo/ncurses expects
	 * */
	//keys->Refine(buf, rd);

	{
	buf[rd] = '\0'; //TODO remove all this debug stuff
	gunichar uc = g_utf8_get_char(buf);
	log->Write(Log::Type_cim, Log::Level_debug, "input: %s (%02x %02x %02x) %d utf8? %d uc: %d %s", buf, buf[0], buf[1], buf[2],
		rd, g_utf8_validate(buf, rd, NULL), uc, key_left); //TODO remove
	}

	input.append(buf, rd);

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
	windowmanager->Add(new GeneralMenu(40,0, 40, 20, LineStyle::LineStyleDefault()));
}

