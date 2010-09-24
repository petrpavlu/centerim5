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
 * SplitDialog class.
 *
 * @ingroup cppconsui
 */

#ifndef __SPLITDIALOG_H__
#define __SPLITDIALOG_H__

#include "Container.h"
#include "Dialog.h"

class SplitDialog
: public Dialog
{
	public:
		SplitDialog(int x, int y, int w, int h,
				LineStyle::Type ltype = LineStyle::DEFAULT);
		SplitDialog();
		virtual ~SplitDialog() {}

		// Container
		void MoveFocus(FocusDirection direction);

		void SetContainer(Container& cont);
		Container *GetContainer() { return container; }

	protected:
		Container *container;
		int container_index;
		int buttons_index;

	private:
		SplitDialog(const SplitDialog&);
		SplitDialog& operator=(const SplitDialog&);
};

#endif /* __SPLITDIALOG_H__ */
