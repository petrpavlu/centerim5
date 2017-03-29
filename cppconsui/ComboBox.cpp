// Copyright (C) 2007 Mark Pustjens <pustjens@dds.nl>
// Copyright (C) 2010-2015 Petr Pavlu <setup@dagobah.cz>
//
// This file is part of CenterIM.
//
// CenterIM is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// CenterIM is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with CenterIM.  If not, see <http://www.gnu.org/licenses/>.

/// @file
/// ComboBox class implementation.
///
/// @ingroup cppconsui

#include "ComboBox.h"

#include <cassert>
#include <cstring>

namespace CppConsUI {

ComboBox::ComboBox(int w, int h, const char *text)
  : Button(w, h, text, FLAG_VALUE), dropdown_(nullptr), selected_entry_(0),
    max_option_width_(0)
{
  signal_activate.connect(sigc::mem_fun(this, &ComboBox::onDropDown));
}

ComboBox::ComboBox(const char *text)
  : Button(text, FLAG_VALUE), dropdown_(nullptr), selected_entry_(0),
    max_option_width_(0)
{
  signal_activate.connect(sigc::mem_fun(this, &ComboBox::onDropDown));
}

ComboBox::~ComboBox()
{
  clearOptions();
  if (dropdown_ != nullptr)
    dropdown_->close();
}

void ComboBox::clearOptions()
{
  for (ComboBoxEntry &entry : options_)
    delete[] entry.title;

  options_.clear();
  selected_entry_ = 0;
  max_option_width_ = 0;
}

int ComboBox::addOption(const char *text, intptr_t data)
{
  std::size_t size = 1;
  if (text != nullptr)
    size += std::strlen(text);
  ComboBoxEntry e;
  e.title = new char[size];
  if (text != nullptr)
    std::strcpy(e.title, text);
  else
    e.title[0] = '\0';
  e.data = data;

  int w = Curses::onScreenWidth(e.title);
  if (w > max_option_width_)
    max_option_width_ = w;

  // Set this option as selected if it is the first one.
  if (options_.empty()) {
    selected_entry_ = 0;
    setValue(text);
  }

  options_.push_back(e);
  return options_.size() - 1;
}

const char *ComboBox::getSelectedTitle() const
{
  if (options_.empty())
    return nullptr;

  return getTitle(selected_entry_);
}

intptr_t ComboBox::getSelectedData() const
{
  if (options_.empty())
    return 0;

  return getData(selected_entry_);
}

void *ComboBox::getSelectedDataPtr() const
{
  return reinterpret_cast<void *>(getSelectedData());
}

const char *ComboBox::getTitle(int entry) const
{
  assert(entry >= 0);
  assert(static_cast<std::size_t>(entry) < options_.size());

  return options_[entry].title;
}

intptr_t ComboBox::getData(int entry) const
{
  assert(entry >= 0);
  assert(static_cast<std::size_t>(entry) < options_.size());

  return options_[entry].data;
}

void ComboBox::setSelected(int new_entry)
{
  assert(new_entry >= 0);
  assert(static_cast<std::size_t>(new_entry) < options_.size());

  // Selected option did not change.
  if (new_entry == selected_entry_)
    return;

  selected_entry_ = new_entry;
  ComboBoxEntry e = options_[new_entry];
  setValue(e.title);
  signal_selection_changed(*this, new_entry, e.title, e.data);
}

void ComboBox::setSelectedByData(intptr_t data)
{
  int i = 0;
  for (ComboBoxEntry &entry : options_) {
    if (entry.data == data) {
      setSelected(i);
      break;
    }
    ++i;
  }
}

void ComboBox::onDropDown(Button & /*activator*/)
{
  if (options_.empty())
    return;

  dropdown_ = new MenuWindow(*this, max_option_width_ + 2, AUTOSIZE);
  dropdown_->signal_close.connect(
    sigc::mem_fun(this, &ComboBox::dropDownClose));

  int i = 0;
  for (ComboBoxEntry &entry : options_) {
    Button *b = dropdown_->appendItem(
      entry.title, sigc::bind(sigc::mem_fun(this, &ComboBox::dropDownOk), i));
    if (i == selected_entry_)
      b->grabFocus();
    ++i;
  }

  dropdown_->show();
}

void ComboBox::dropDownOk(Button & /*activator*/, int new_entry)
{
  dropdown_->close();

  setSelected(new_entry);
}

void ComboBox::dropDownClose(Window & /*window*/)
{
  dropdown_ = nullptr;
}

} // namespace CppConsUI

// vim: set tabstop=2 shiftwidth=2 textwidth=80 expandtab:
