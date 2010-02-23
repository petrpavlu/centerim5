/*
 * Copyright (C) 2009 by CenterIM developers
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

#include <cstring>
#include <libintl.h>
#include <cerrno>

#include "ConsuiCurses.h"

#include "Application.h"

#include "CppConsUIInternal.h"

Application::Application()
: windowmanager(NULL), channel(NULL), channel_id(0), tk(NULL), utf8(false)
, gmainloop(NULL)
{
	StdinInputInit();

	windowmanager = WindowManager::Instance();
	SetInputChild(*windowmanager);

	/* Application always needs to be the first resize handler because users
	 * usually want to recalculate sizes of all windows in ScreenResized
	 * (virtual method) first and then resize the windows appropriately. */
	resize = windowmanager->signal_resize.connect(sigc::mem_fun(this, &Application::ScreenResized));

	// create a new loop
	gmainloop = g_main_loop_new(NULL, FALSE);
}

Application::~Application()
{
	resize.disconnect();

	windowmanager->Delete();

	StdinInputUnInit();
}

void Application::Run()
{
	windowmanager->EnableResizing();
	windowmanager->ScreenResized();

	g_main_loop_run(gmainloop);
}

void Application::Quit()
{
	g_main_loop_quit(gmainloop);
}

gboolean Application::io_input_error(GIOChannel *source, GIOCondition cond)
{
	// log a critical warning and bail out if we lost stdin
	g_critical("Stdin lost!\n");
	Quit();

	return TRUE;
}

gboolean Application::io_input(GIOChannel *source, GIOCondition cond)
{
	termkey_advisereadable(tk);

	TermKeyKey key;
	/**
	 * @todo Actually we should call termkey_getkey() instead of
	 * termkey_getkey_force(). See libtermkey async demo.
	 */
	while (termkey_getkey_force(tk, &key) == TERMKEY_RES_KEY) {
		if (key.type == TERMKEY_TYPE_UNICODE && !utf8) {
			gsize bwritten;
			GError *err = NULL;
			gchar *utf8;

			// convert data from user charset to UTF-8
			if (!(utf8 = g_locale_to_utf8(key.utf8, -1, NULL, &bwritten, &err))) {
				if (err) {
					g_warning(_("Error converting input to UTF-8 (%s).\n"), err->message);
					g_error_free(err);
					err = NULL;
				}
				else
					g_warning(_("Error converting input to UTF-8.\n"));
				continue;
			}

			memcpy(key.utf8, utf8, bwritten + 1);
			g_free(utf8);

			key.code.codepoint = g_utf8_get_char(key.utf8);
		}

		ProcessInput(key);
	}

	return TRUE;
}

void Application::StdinInputInit()
{
	// init libtermkey
	TERMKEY_CHECK_VERSION;
	if (!(tk = termkey_new(STDIN_FILENO, 0))) {
		g_critical(_("Libtermkey initialization failed.\n"));
		exit(1);
	}
	utf8 = g_get_charset(NULL);

	channel = g_io_channel_unix_new(STDIN_FILENO);
	// set channel encoding to NULL so it can be unbuffered
	g_io_channel_set_encoding(channel, NULL, NULL);
	g_io_channel_set_buffered(channel, FALSE);
	g_io_channel_set_close_on_unref(channel, TRUE);

	channel_id = g_io_add_watch_full(channel, G_PRIORITY_HIGH,
			(GIOCondition)(G_IO_IN | G_IO_HUP | G_IO_ERR | G_IO_PRI), io_input_, this, NULL);

	g_io_add_watch_full(channel, G_PRIORITY_HIGH, (G_IO_NVAL), io_input_error_, this, NULL);

	g_io_channel_unref(channel);
}

void Application::StdinInputUnInit()
{
	termkey_destroy(tk);

	g_source_remove(channel_id);
	channel_id = 0;
	g_io_channel_unref(channel);
	channel = NULL;
}
