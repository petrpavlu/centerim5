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

#ifndef __ACOUNTSTATUSMENU_H__
#define __ACOUNTSTATUSMENU_H__

#include "Accounts.h"

#include <cppconsui/MenuWindow.h>
#include <cppconsui/WindowManager.h>

#include <libpurple/account.h>
#include <libpurple/savedstatuses.h>
#include <libpurple/status.h>

class AccountStatusMenu
: public MenuWindow
{
	public:
		AccountStatusMenu(int x, int y, int w, int h, LineStyle::Type ltype);
		virtual ~AccountStatusMenu();

		void Dummy(void) { ; } //TODO remove

		virtual void ScreenResized();
	protected:

	private:
		class StatusPopup
		: public MenuWindow
		{
			public:
				StatusPopup(int x, int y, int w, int h, LineStyle::Type ltype, PurpleAccount *account);
				~StatusPopup();

				void Dummy(void) { ; } //TODO remove

			protected:

			private:
				void SetStatus(PurpleAccount *account, PurpleStatusType *status_type, bool active);
				//void SetSavedStatus(PurpleAccount *account, PurpleSavedStatus *status);
				//void SetStatusPrimitive(PurpleAccount *account, PurpleStatusPrimitive primitive);

				PurpleAccount *account;
		};

		AccountStatusMenu();
		AccountStatusMenu(const AccountStatusMenu&);

		AccountStatusMenu& operator=(const AccountStatusMenu&);

		void OpenStatusPopup(PurpleAccount *account);

		WindowManager *windowmanager;
};

#endif /* __ACOUNTSTATUSMENU_H__ */
