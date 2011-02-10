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
 * Button class implementation.
 *
 * @ingroup cppconsui
 */

#include "Button.h"

#include "Keys.h"

#include "gettext.h"

#define CONTEXT_BUTTON "button"

Button::Button(int w, int h, const gchar *text_, const gchar *value_)
: Widget(w, h), text(NULL), value(NULL), value_visible(false)
{
  SetText(text_);
  SetValue(value_);

  can_focus = true;
  DeclareBindables();
}

Button::Button(const gchar *text_, const gchar *value_)
: Widget(AUTOSIZE, 1), text(NULL), value(NULL), value_visible(false)
{
  SetText(text_);
  SetValue(value_);

  can_focus = true;
  DeclareBindables();
}

Button::~Button()
{
  if (text)
    g_free(text);
  if (value)
    g_free(value);
}

void Button::DeclareBindables()
{
  DeclareBindable(CONTEXT_BUTTON, "activate", sigc::mem_fun(this,
        &Button::ActionActivate), InputProcessor::BINDABLE_NORMAL);
}

DEFINE_SIG_REGISTERKEYS(Button, RegisterKeys);
bool Button::RegisterKeys()
{
  RegisterKeyDef(CONTEXT_BUTTON, "activate", _("Activate the button"),
      Keys::SymbolTermKey(TERMKEY_SYM_ENTER));
  return true;
}


void Button::Draw()
{
  RealUpdateArea();

  if (!area || !text)
    return;

  int attrs;
  if (has_focus) {
    attrs = GetColorPair("button", "focus");
    area->attron(attrs | Curses::Attr::REVERSE);
  }
  else {
    attrs = GetColorPair("button", "normal");
    area->attron(attrs);
  }

  /**
   * @todo Though this is not a widget for long text there are some cases in
   * cim where we use it for a short but multiline text, so we should threat
   * LF specially here.
   */

  int max = area->getmaxx() * area->getmaxy();
  int l = area->mvaddstring(0, 0, max, text);
  if (value_visible) {
    l += area->mvaddstring(l, 0, max - l, ": ");
    if (value)
      area->mvaddstring(l, 0, max - l, value);
  }

  if (has_focus)
    area->attroff(attrs | Curses::Attr::REVERSE);
  else
    area->attroff(attrs);
}

void Button::SetText(const gchar *new_text)
{
  if (text)
    g_free(text);

  if (new_text)
    text = g_strdup(new_text);
  else
    text = NULL;

  Redraw();
}

void Button::SetValue(const gchar *new_value)
{
  if (value)
    g_free(value);

  if (new_value)
    value = g_strdup(new_value);
  else
    value = NULL;

  Redraw();
}

void Button::SetValue(int new_value)
{
  if (value)
    g_free(value);

  value = g_strdup_printf("%d", new_value);
  Redraw();
}

void Button::SetValueVisibility(bool visible)
{
  value_visible = visible;
}

void Button::ActionActivate()
{
  signal_activate(*this);
}
