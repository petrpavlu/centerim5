/*
 * Copyright (C) 2007 by Mark Pustjens <pustjens@dds.nl>
 * Copyright (C) 2010-2011 by CenterIM developers
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

#include <cppconsui/MenuWindow.h>
#include <libpurple/purple.h>

class AccountStatusMenu
: public CppConsUI::MenuWindow
{
public:
  AccountStatusMenu();
  virtual ~AccountStatusMenu() {}

  // FreeWindow
  virtual void OnScreenResized();

protected:

private:
  class StatusPopup
  : public MenuWindow
  {
  public:
    StatusPopup(PurpleAccount *account);
    virtual ~StatusPopup() {}

    // FreeWindow
    virtual void OnScreenResized();

  protected:
    PurpleAccount *account;

    void SetStatus(CppConsUI::Button& activator, PurpleAccount *account,
        PurpleStatusType *status_type, bool active);

  private:
  };

  AccountStatusMenu(const AccountStatusMenu&);
  AccountStatusMenu& operator=(const AccountStatusMenu&);

  void OpenStatusPopup(CppConsUI::Button& activator, PurpleAccount *account);
};

#endif // __ACOUNTSTATUSMENU_H__

/* vim: set tabstop=2 shiftwidth=2 expandtab */
