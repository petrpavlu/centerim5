/*
 * Copyright (C) 2008 by Mark Pustjens <pustjens@dds.nl>
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
 * InputDialog class.
 *
 * @ingroup cppconsui
 */

#ifndef __INPUTDIALOG_H__
#define __INPUTDIALOG_H__

#include "Dialog.h"

#include "Label.h"
#include "HorizontalLine.h"
#include "TextEntry.h"

class InputDialog
: public Dialog
{
	public:
		InputDialog(const gchar* text, const gchar* defaultvalue);
		virtual ~InputDialog() {}

		const gchar *GetText() const;

		int GetFlags() { return entry->GetFlags(); }
		void SetFlags(int flags) { entry->SetFlags(flags); }

	protected:
		Label *label;
		TextEntry *entry;
		HorizontalLine *separator;

	private:
		InputDialog(const InputDialog&);
		InputDialog& operator=(const InputDialog&);
};

#endif /* __INPUTDIALOG_H__ */
