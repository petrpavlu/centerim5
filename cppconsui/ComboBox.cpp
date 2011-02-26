/*
 * Copyright (C) 2007 by Mark Pustjens <pustjens@dds.nl>
 * Copyright (C) 2010-2011 by CenterIM developers
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
 * ComboBox class implementation.
 *
 * @ingroup cppconsui
 */

#include "ComboBox.h"

#include "Keys.h"

#include "gettext.h"

ComboBox::ComboBox(int w, int h, const char *text)
: Button(TYPE_DOUBLE, w, h, text)
, dropdown(NULL)
, selected_entry(0)
, max_option_width(0)
{
  signal_activate.connect(sigc::mem_fun(this, &ComboBox::OnDropDown));
}

ComboBox::ComboBox(const char *text)
: Button(TYPE_DOUBLE, text)
, dropdown(NULL)
, selected_entry(0)
, max_option_width(0)
{
  signal_activate.connect(sigc::mem_fun(this, &ComboBox::OnDropDown));
}

ComboBox::~ComboBox()
{
  ClearOptions();
  // XXX
  // WindowManager will take care about freeing dropdown menu
}

void ComboBox::ClearOptions()
{
  for (ComboBoxEntries::iterator i = options.begin(); i != options.end(); i++)
    if (i->title)
      g_free(i->title);

  options.clear();
  selected_entry = 0;
  max_option_width = 0;
}

int ComboBox::AddOption(const char *text, intptr_t data)
{
  ComboBoxEntry e;
  int w = 0;

  if (text) {
    e.title = g_strdup(text);
    w = Curses::onscreen_width(text);
  }
  else
    e.title = NULL;
  e.data = data;

  if (w > max_option_width)
    max_option_width = w;

  // set this option as selected if there isn't any other yet
  if (options.empty()) {
    selected_entry = 0;
    SetValue(text);
  }

  options.push_back(e);
  return options.size() - 1;
}

const char *ComboBox::GetSelectedTitle() const
{
  if (options.empty())
    return NULL;

  return GetTitle(selected_entry);
}

const char *ComboBox::GetTitle(int entry) const
{
  g_assert(entry < (int) options.size());
  g_return_val_if_fail(entry >= 0, NULL);

  return options[entry].title;
}

intptr_t ComboBox::GetData(int entry) const
{
  g_assert(entry < (int) options.size());
  g_return_val_if_fail(entry >= 0, 0);

  return options[entry].data;
}

void ComboBox::SetSelected(int new_entry)
{
  g_assert(new_entry >= 0);
  g_assert(new_entry < (int) options.size());

  selected_entry = new_entry;
  SetValue(options[new_entry].title);

  /// @todo Emit signal here?
  //signal_selection_changed(*this, new_entry, e.title, e.data);
}

void ComboBox::SetSelectedByData(intptr_t data)
{
  int i;
  ComboBoxEntries::iterator j;
  for (i = 0, j = options.begin(); j != options.end(); i++, j++)
    if (j->data == data)
      SetSelected(i);
}

void ComboBox::OnDropDown(Button& activator)
{
  if (options.empty())
    return;

  /// @todo Make sure that requested MenuWindow size can fit into the screen.
  Point p = GetAbsolutePosition();
  dropdown = new MenuWindow(p.X(), p.Y() + 1, max_option_width + 2,
      options.size() + 2);
  dropdown->signal_close.connect(sigc::mem_fun(this,
        &ComboBox::DropDownClose));

  int i;
  ComboBoxEntries::iterator j;
  for (i = 0, j = options.begin(); j != options.end(); i++, j++) {
    Button *b = dropdown->AppendItem(j->title, sigc::bind(sigc::mem_fun(this,
            &ComboBox::DropDownOk), i));
    if (i == selected_entry)
      b->GrabFocus();
  }

  dropdown->Show();
}

void ComboBox::DropDownOk(Button& activator, int new_entry)
{
  dropdown->Close();

  // selected option didn't change
  if (selected_entry == new_entry)
    return;

  selected_entry = new_entry;
  ComboBoxEntry e = options[new_entry];
  SetValue(e.title);
  signal_selection_changed(*this, new_entry, e.title, e.data);
}

void ComboBox::DropDownClose(FreeWindow& window)
{
  dropdown = NULL;
}
