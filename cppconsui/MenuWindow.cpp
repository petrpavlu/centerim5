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

#include "ConsuiCurses.h"
#include "Keys.h"
#include "CppConsUIInternal.h"

#include "MenuWindow.h"

#include <glibmm/ustring.h>

#define CONTEXT_MENUWINDOW "menuwindow"

MenuWindow::MenuWindow(int x_, int y_, int w_, int h_, LineStyle::Type ltype)
: Window(x_, y_, w_, h_, ltype)
{
	listbox = new ListBox(*this, 0, 0, w, h);
	SetFocusChild(listbox);
	Window::AddWidget(listbox);
	DeclareBindables();
}

MenuWindow::~MenuWindow()
{
}

void MenuWindow::DeclareBindables()
{
	//DeclareBindable(CONTEXT_MENUWINDOW, "focus-previous", sigc::mem_fun(listbox, &Container::FocusCyclePrevious),
	//	InputProcessor::Bindable_Normal);
	//DeclareBindable(CONTEXT_MENUWINDOW, "focus-next", sigc::mem_fun(listbox, &Container::FocusCycleNext),
	//	InputProcessor::Bindable_Normal);
	DeclareBindable(CONTEXT_MENUWINDOW, "close-window", sigc::mem_fun(this, &Window::Close),
					InputProcessor::Bindable_Normal);
}

DEFINE_SIG_REGISTERKEYS(MenuWindow, RegisterKeys);
bool MenuWindow::RegisterKeys()
{
	//RegisterKeyDef(CONTEXT_MENUWINDOW, "focus-previous", _("Focusses the previous menu item"), KEYS->Key_up());
	//RegisterKeyDef(CONTEXT_MENUWINDOW, "focus-next", _("Focusses the next menu item"), KEYS->Key_down());
	RegisterKeyDef(CONTEXT_MENUWINDOW, "close-window", _("Close the window"), KEYS->Key_esc());
	return true;
}


void MenuWindow::MoveResize(int newx, int newy, int neww, int newh)
{
	/* Let parent's Resize() renew data structures (including
	 * the area's of child widgets which will thus be done
	 * twice)
	 * */
	Window::MoveResize(newx, newy, neww, newh);

	/* resize all our widgets, in this case its only one widget
	 * here, w and h are the size of the container, which is 
	 * what we want. in most cases you would need to recalculate
	 * widget sizes based on window and/or container size.
	 * */
	listbox->MoveResize(0, 0, w, h);
}
