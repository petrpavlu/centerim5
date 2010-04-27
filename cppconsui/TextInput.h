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

#ifndef __TEXT_INPUT_H__
#define __TEXT_INPUT_H__

#include "TextBrowser.h"

#include <glibmm/ustring.h>
#include <vector>

class TextInput
: public TextBrowser
{
	public:
		TextInput(int x, int y, int w, int h);
		TextInput(int x, int y, int w, int h, std::vector<Glib::ustring> &lines);
		virtual ~TextInput();

		virtual void Draw(void);

		virtual int ProcessInputText(const char *input, const int bytes);

	protected:

	private:
		TextInput();
		TextInput(const TextInput&);

		TextInput& operator=(const TextInput&);
};

#endif /* __TEXT_INPUT_H__ */
