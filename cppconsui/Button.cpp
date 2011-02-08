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

AbstractButton::AbstractButton(int w, int h, const gchar *text_)
: Widget(w, h), text(NULL)
{
  SetText(text_);

  can_focus = true;
  DeclareBindables();
}

AbstractButton::AbstractButton(const gchar *text_)
: Widget(AUTOSIZE, 1), text(NULL)
{
  SetText(text_);

  can_focus = true;
  DeclareBindables();
}

AbstractButton::~AbstractButton()
{
  if (text)
    g_free(text);
}

void AbstractButton::DeclareBindables()
{
  DeclareBindable(CONTEXT_BUTTON, "activate", sigc::mem_fun(this,
        &AbstractButton::ActionActivate), InputProcessor::BINDABLE_NORMAL);
}

DEFINE_SIG_REGISTERKEYS(AbstractButton, RegisterKeys);
bool AbstractButton::RegisterKeys()
{
  RegisterKeyDef(CONTEXT_BUTTON, "activate", _("Activate the button"),
      Keys::SymbolTermKey(TERMKEY_SYM_ENTER));
  return true;
}

void AbstractButton::SetText(const gchar *new_text)
{
  if (text)
    g_free(text);

  if (new_text)
    text = g_strdup(new_text);
  else
    text = NULL;

  Redraw();
}

Button::Button(int w, int h, const gchar *text_)
: AbstractButton(w, h, text_)
{
}

Button::Button(const gchar *text_)
: AbstractButton(text_)
{
}

void Button::Draw()
{
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
  area->mvaddstring(0, 0, max, text);

  if (has_focus)
    area->attroff(attrs | Curses::Attr::REVERSE);
  else
    area->attroff(attrs);
}

void Button::ActionActivate()
{
  signal_activate(*this);
}

Button2::Button2(int w, int h, const gchar *text_, const gchar *value_)
: AbstractButton(w, h, text_), value(NULL)
{
  SetValue(value_);
}

Button2::Button2(const gchar *text_, const gchar *value_)
: AbstractButton(text_), value(NULL)
{
  SetValue(value_);
}

Button2::~Button2()
{
  if (value)
    g_free(value);
}

void Button2::Draw()
{
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
  l += area->mvaddstring(l, 0, max - l, ": ");
  area->mvaddstring(l, 0, max - l, value);

  if (has_focus)
    area->attroff(attrs | Curses::Attr::REVERSE);
  else
    area->attroff(attrs);
}

void Button2::SetValue(const gchar *new_value)
{
  if (value)
    g_free(value);

  if (new_value)
    value = g_strdup(new_value);
  else
    value = NULL;

  Redraw();
}

void Button2::ActionActivate()
{
  signal_activate(*this);
}
