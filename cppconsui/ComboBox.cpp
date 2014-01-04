/*
 * Copyright (C) 2007 by Mark Pustjens <pustjens@dds.nl>
 * Copyright (C) 2010-2013 by CenterIM developers
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
 */

/**
 * @file
 * ComboBox class implementation.
 *
 * @ingroup cppconsui
 */

#include "ComboBox.h"

#include <cassert>
#include <cstring>

namespace CppConsUI
{

ComboBox::ComboBox(int w, int h, const char *text)
: Button(w, h, text, FLAG_VALUE), dropdown(NULL), selected_entry(0)
, max_option_width(0)
{
  signal_activate.connect(sigc::mem_fun(this, &ComboBox::onDropDown));
}

ComboBox::ComboBox(const char *text)
: Button(text, FLAG_VALUE), dropdown(NULL), selected_entry(0)
, max_option_width(0)
{
  signal_activate.connect(sigc::mem_fun(this, &ComboBox::onDropDown));
}

ComboBox::~ComboBox()
{
  clearOptions();
  if (dropdown)
    dropdown->close();
}

void ComboBox::clearOptions()
{
  for (ComboBoxEntries::iterator i = options.begin(); i != options.end(); i++)
    delete [] i->title;

  options.clear();
  selected_entry = 0;
  max_option_width = 0;
}

int ComboBox::addOption(const char *text, intptr_t data)
{
  size_t size = 1;
  if (text)
    size += std::strlen(text);
  ComboBoxEntry e;
  e.title = new char[size];
  if (text)
    std::strcpy(e.title, text);
  else
    e.title[0] = '\0';
  e.data = data;

  int w = Curses::onscreen_width(e.title);
  if (w > max_option_width)
    max_option_width = w;

  // set this option as selected if there isn't any other yet
  if (options.empty()) {
    selected_entry = 0;
    setValue(text);
  }

  options.push_back(e);
  return options.size() - 1;
}

const char *ComboBox::getSelectedTitle() const
{
  if (options.empty())
    return NULL;

  return getTitle(selected_entry);
}

intptr_t ComboBox::getSelectedData() const
{
  if (options.empty())
    return 0;

  return getData(selected_entry);
}

void *ComboBox::getSelectedDataPtr() const
{
  return reinterpret_cast<void*>(getSelectedData());
}

const char *ComboBox::getTitle(int entry) const
{
  assert(entry >= 0);
  assert(static_cast<size_t>(entry) < options.size());

  return options[entry].title;
}

intptr_t ComboBox::getData(int entry) const
{
  assert(entry >= 0);
  assert(static_cast<size_t>(entry) < options.size());

  return options[entry].data;
}

void ComboBox::setSelected(int new_entry)
{
  assert(new_entry >= 0);
  assert(static_cast<size_t>(new_entry) < options.size());

  // selected option didn't change
  if (new_entry == selected_entry)
    return;

  selected_entry = new_entry;
  ComboBoxEntry e = options[new_entry];
  setValue(e.title);
  signal_selection_changed(*this, new_entry, e.title, e.data);
}

void ComboBox::setSelectedByData(intptr_t data)
{
  int i;
  ComboBoxEntries::iterator j;
  for (i = 0, j = options.begin(); j != options.end(); i++, j++)
    if (j->data == data) {
      setSelected(i);
      break;
    }
}

void ComboBox::onDropDown(Button& /*activator*/)
{
  if (options.empty())
    return;

  dropdown = new MenuWindow(*this, max_option_width + 2, AUTOSIZE);
  dropdown->signal_close.connect(sigc::mem_fun(this,
        &ComboBox::dropDownClose));

  int i;
  ComboBoxEntries::iterator j;
  for (i = 0, j = options.begin(); j != options.end(); i++, j++) {
    Button *b = dropdown->appendItem(j->title, sigc::bind(sigc::mem_fun(this,
            &ComboBox::dropDownOk), i));
    if (i == selected_entry)
      b->grabFocus();
  }

  dropdown->show();
}

void ComboBox::dropDownOk(Button& /*activator*/, int new_entry)
{
  dropdown->close();

  setSelected(new_entry);
}

void ComboBox::dropDownClose(FreeWindow& /*window*/)
{
  dropdown = NULL;
}

} // namespace CppConsUI

/* vim: set tabstop=2 shiftwidth=2 textwidth=78 expandtab : */
