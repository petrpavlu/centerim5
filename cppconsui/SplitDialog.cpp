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

/**
 * @file
 * SplitDialog class implementation.
 *
 * @ingroup cppconsui
 */

#include "SplitDialog.h"

#include "gettext.h"

SplitDialog::SplitDialog(int x, int y, int w, int h, LineStyle::Type ltype)
: Dialog(x, y, w, h, ltype)
, container(NULL)
, container_index(0)
, buttons_index(0)
{
	buttons->SetFocusCycle(Container::FOCUS_CYCLE_LOCAL);
}

SplitDialog::SplitDialog()
: Dialog()
, container(NULL)
, container_index(0)
, buttons_index(0)
{
	buttons->SetFocusCycle(Container::FOCUS_CYCLE_LOCAL);
}

void SplitDialog::MoveFocus(FocusDirection direction)
{
	if (!container) {
		Dialog::MoveFocus(direction);
		return;
	}

	switch (direction) {
		case FOCUS_LEFT:
		case FOCUS_RIGHT:
			if (layout->GetFocusChild() != buttons) {
				container_index = container->GetActive();
				buttons->SetActive(buttons_index);
			}
			else
				Dialog::MoveFocus(direction);

			break;
		case FOCUS_UP:
		case FOCUS_DOWN:
			if (layout->GetFocusChild() != container) {
				buttons_index = buttons->GetActive();
				container->SetActive(container_index);
			}
			else
				Dialog::MoveFocus(direction);

			break;
		default:
			Dialog::MoveFocus(direction);
			break;
	}
}

void SplitDialog::SetContainer(Container& cont)
{
	g_assert(!container);
	g_warn_if_fail(cont.Width() == AUTOSIZE);
	g_warn_if_fail(cont.Height() == AUTOSIZE);

	container = &cont;
	cont.SetFocusCycle(Container::FOCUS_CYCLE_LOCAL);
	layout->InsertWidget(0, cont);
}
