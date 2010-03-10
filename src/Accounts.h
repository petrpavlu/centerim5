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

#ifndef __ACCOUNTS_H__
#define __ACCOUNTS_H__

#include "Log.h"

#include <libpurple/connection.h>
#include <libpurple/savedstatuses.h>

class Accounts
{
	public:
		static Accounts* Instance(void);
		static void Delete(void);

		void SetStatus(PurpleAccount *account, PurpleStatusType *status_type, bool active);

		static void status_changed_(PurpleAccount *account, PurpleStatus *status);
		void status_changed(PurpleAccount *account, PurpleStatus *status);
	protected:

	private:
		Accounts();
		~Accounts();

		static Accounts* instance;

		/* callbacks */
		static void signed_on_(PurpleConnection *gc, gpointer p);
		void signed_on(PurpleConnection *gc);

		void *accounts_handle;
};

#endif /* __ACCOUNTS_H__ */
