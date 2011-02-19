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
 * CheckBox class implementation.
 *
 * @ingroup cppconsui
 */

#include "CheckBox.h"

#include "Dialog.h"
#include "Keys.h"

#include "gettext.h"

#define CONTEXT_CHECKBOX "checkbox"

CheckBox::CheckBox(int w, int h, const char *text_, bool default_state)
: Widget(w, h)
, text(NULL)
, state(default_state)
, style(STYLE_DEFAULT)
{
  SetText(text_);

  can_focus = true;
  DeclareBindables();
}

CheckBox::CheckBox(const char *text_, bool default_state)
: Widget(AUTOSIZE, 1)
, text(NULL)
, state(default_state)
, style(STYLE_DEFAULT)
{
  SetText(text_);

  can_focus = true;
  DeclareBindables();
}

CheckBox::~CheckBox()
{
  if (text)
    g_free(text);
}

void CheckBox::DeclareBindables()
{
  DeclareBindable(CONTEXT_CHECKBOX, "toggle", sigc::mem_fun(this,
        &CheckBox::ActionToggle), InputProcessor::BINDABLE_NORMAL);
}

DEFINE_SIG_REGISTERKEYS(CheckBox, RegisterKeys);
bool CheckBox::RegisterKeys()
{
  RegisterKeyDef(CONTEXT_CHECKBOX, "toggle", _("Toggle the checkbox"),
      Keys::SymbolTermKey(TERMKEY_SYM_ENTER));
  return true;
}

void CheckBox::Draw()
{
  RealUpdateArea();

  if (!area || !text)
    return;

  int attrs;
  if (has_focus) {
    attrs = GetColorPair("checkbox", "focus");
    area->attron(attrs | Curses::Attr::REVERSE);
  }
  else {
    attrs = GetColorPair("checkbox", "normal");
    area->attron(attrs);
  }

  /**
   * @todo Though this is not a widget for long text there are some cases in
   * cim where we use it for a short but multiline text, so we should threat
   * LF specially here.
   */

  int max = area->getmaxx() * area->getmaxy();
  int x = area->mvaddstring(0, 0, max, text);
  if (style == STYLE_BOX) {
    if (state)
      x += area->mvaddstring(x, 0, max - x, " \xe2\x9c\x93");
    else
      x += area->mvaddstring(x, 0, max - x, " \xe2\x9c\x97");
  }
  else {
    x += area->mvaddstring(x, 0, max - x, ": ");
    area->mvaddstring(x, 0, max - x,
        state ? _(YES_BUTTON_TEXT) : _(NO_BUTTON_TEXT));
  }

  if (has_focus)
    area->attroff(attrs | Curses::Attr::REVERSE);
  else
    area->attroff(attrs);
}

void CheckBox::SetText(const char *new_text)
{
  if (text)
    g_free(text);

  if (new_text)
    text = g_strdup(new_text);
  else
    text = NULL;

  Redraw();
}

void CheckBox::SetState(bool new_state)
{
  bool old_state = state;
  state = new_state;

  if (state != old_state)
    signal_toggle(*this, state);
  Redraw();
}

void CheckBox::SetStyle(Style new_style)
{
  style = new_style;
  Redraw();
}

void CheckBox::ActionToggle()
{
  state = !state;
  signal_toggle(*this, state);
  Redraw();
}
