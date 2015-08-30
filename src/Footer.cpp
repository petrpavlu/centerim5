// Copyright (C) 2011-2015 Petr Pavlu <setup@dagobah.cz>
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
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "Footer.h"

#include <cppconsui/KeyConfig.h>
#include <cstring>
#include <string>

Footer *Footer::my_instance_ = NULL;

Footer *Footer::instance()
{
  return my_instance_;
}

void Footer::onScreenResized()
{
  moveResizeRect(CENTERIM->getScreenArea(CenterIM::FOOTER_AREA));
}

void Footer::setText(const char *fmt, ...)
{
  values_.clear();

  if (fmt == NULL) {
    updateText();
    return;
  }

  values_.push_back(std::string(fmt));

  va_list args;
  va_start(args, fmt);
  while (*fmt != '\0') {
    if (*fmt == '%') {
      if (*(fmt + 1) == '%')
        ++fmt;
      else if (*(fmt + 1) == 's') {
        const char *v = va_arg(args, const char *);
        values_.push_back(std::string(v));
        ++fmt;
      }
    }
    ++fmt;
  }
  va_end(args);

  updateText();
}

Footer::Footer() : Window(0, 24, 80, 1, TYPE_NON_FOCUSABLE, false)
{
  setColorScheme(CenterIM::SCHEME_FOOTER);

  label_ = new CppConsUI::Label;
  addWidget(*label_, 0, 0);

  onScreenResized();
}

void Footer::init()
{
  g_assert(my_instance_ == NULL);

  my_instance_ = new Footer;
  my_instance_->show();
}

void Footer::finalize()
{
  g_assert(my_instance_ != NULL);

  delete my_instance_;
  my_instance_ = NULL;
}

void Footer::updateText()
{
  if (values_.empty()) {
    label_->setText(NULL);
    return;
  }

  Values::iterator i = values_.begin();
  const char *fmt = i->c_str();
  ++i;

  char out[1024];
  char *cur_out = out;

  while (*fmt != '\0' && cur_out < out + sizeof(out) - 1) {
    if (*fmt == '%') {
      if (*(fmt + 1) == '%')
        ++fmt;
      else if (*(fmt + 1) == 's') {
        char *con = g_strdup(i->c_str());
        ++i;
        char *act = std::strstr(con, "|");
        g_assert(act);
        *act++ = '\0';
        const char *key = KEYCONFIG->getKeyBind(con, act);
        g_free(con);

        std::strncpy(cur_out, key, out + sizeof(out) - 1 - cur_out);
        cur_out += std::strlen(key);

        fmt += 2;
        continue;
      }
    }
    *cur_out++ = *fmt++;
  }
  *cur_out = '\0';

  label_->setText(out);
}

// vim: set tabstop=2 shiftwidth=2 textwidth=80 expandtab:
