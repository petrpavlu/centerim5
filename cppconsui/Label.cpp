/*
 * Copyright (C) 2007 by Mark Pustjens <pustjens@dds.nl>
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
 * Label class implementation.
 *
 * @ingroup cppconsui
 */

#include "Label.h"

Label::Label(int w, int h, const gchar *text_)
: Widget(w, h)
, text(NULL)
{
  SetText(text_);
}

Label::Label(const gchar *text_)
: Widget(AUTOSIZE, 1)
, text(NULL)
{
  SetText(text_);
}

Label::~Label()
{
  if (text)
    g_free(text);
}

void Label::Draw()
{
  RealUpdateArea();

  if (!area || !text)
    return;

  /**
   * @todo Though this is not a widget for long text there are some cases in
   * cim where we use it for a short but multiline text, so we should threat
   * LF specially here.
   */

  int attrs = GetColorPair("label", "text");
  area->attron(attrs);

  int max = area->getmaxx() * area->getmaxy();
  area->mvaddstring(0, 0, max, text);

  area->attroff(attrs);
}

void Label::SetText(const gchar *new_text)
{
  if (text)
    g_free(text);

  if (new_text)
    text = g_strdup(new_text);
  else
    text = NULL;

  Redraw();
}
