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

#include "AccountStatusMenu.h"

#include <cppconsui/Label.h>
#include <cppconsui/HorizontalLine.h>
#include <cppconsui/LineStyle.h>

#include <libpurple/account.h>

AccountStatusMenu::AccountStatusMenu(int x, int y, int w, int h, LineStyle *linestyle)
: MenuWindow(x, y, w, h, linestyle)
{
	Label *label;
	HorizontalLine *line;
	GList *iter;
	gchar *text;

	label = new Label(*listbox, 0, 0, _(" All protocols"));
	label->CanFocus(true);
	AddWidget(label);

	label = new Label(*listbox, 0, 0, _(" Already logged in only"));
	label->CanFocus(true);
	AddWidget(label);

	line = new HorizontalLine(*listbox, 0, 0, listbox->Width());
	AddWidget(line);

	for (iter = purple_accounts_get_all(); iter; iter = iter->next) {

		PurpleAccount *account = (PurpleAccount*)iter->data;

		text = g_strdup_printf(" [%s] %s",
			purple_account_get_protocol_name(account),
			purple_account_get_username(account));

		label = new Label(*listbox, 0, 0, text);
		label->CanFocus(true);

		g_free(text);

		AddWidget(label);
	}
}

AccountStatusMenu::~AccountStatusMenu()
{
}
