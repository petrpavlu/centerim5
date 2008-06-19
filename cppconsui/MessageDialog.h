/*
 * Copyright (C) 2008 by Mark Pustjens <pustjens@dds.nl>
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

#ifndef __MESSAGEDIALOG_H__
#define __MESSAGEDIALOG_H__

#include "Dialog.h"

#include "HorizontalListBox.h"
#include "HorizontalLine.h"
#include "Label.h"

class MessageDialog
: public Dialog
{
	public:
		MessageDialog(const gchar* label);
		virtual ~MessageDialog();

		virtual void Resize(int neww, int newh);

	protected:
		Label *label;

	private:
		MessageDialog();
		MessageDialog(const MessageDialog&);

		MessageDialog& operator=(const MessageDialog&);
};

#endif /* __MESSAGEDIALOG_H__ */
