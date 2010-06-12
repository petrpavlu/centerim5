/*
 * Copyright (C) 2010 by CenterIM developers
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

#include "Notify.h"

#include <cstring>

Notify *Notify::Instance()
{
	static Notify instance;
	return &instance;
}

Notify::Notify()
{
	memset(&centerim_notify_ui_ops, 0, sizeof(centerim_notify_ui_ops));

	// set the purple notify callbacks
	//centerim_notify_ui_ops.notify_message = notify_message_;
	//centerim_notify_ui_ops.notify_email = notify_email_;
	//centerim_notify_ui_ops.notify_emails = notify_emails_;
	//centerim_notify_ui_ops.notify_formatted = notify_formatted_;
	//centerim_notify_ui_ops.notify_searchresults = notify_searchresults_;
	//centerim_notify_ui_ops.notify_searchresults_new_rows = notify_searchresults_new_rows_;
	//centerim_notify_ui_ops.notify_userinfo = notify_userinfo_;
	//centerim_notify_ui_ops.notify_uri = notify_uri_;
	//centerim_notify_ui_ops.close_notify = close_notify_;
	purple_notify_set_ui_ops(&centerim_notify_ui_ops);
}

Notify::~Notify()
{
}

void *Notify::notify_message(PurpleNotifyMsgType type,
		const char *title, const char *primary,
		const char *secondary)
{
}
