/*
 * Copyright (C) 2011-2012 by CenterIM developers
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

#include "Footer.h"

#include "CenterIM.h"

#include <cppconsui/KeyConfig.h>
#include "gettext.h"

Footer *Footer::instance = NULL;

Footer *Footer::Instance()
{
  return instance;
}

void Footer::OnScreenResized()
{
  MoveResizeRect(CENTERIM->GetScreenAreaSize(CenterIM::FOOTER_AREA));
}

void Footer::SetText(const char *fmt, ...)
{
  values.clear();

  if (!fmt) {
    UpdateText();
    return;
  }

  values.push_back(std::string(fmt));

  va_list args;

  va_start(args, fmt);
  while (*fmt) {
    if (*fmt == '%') {
      if (*(fmt + 1) == '%')
        fmt++;
      else if (*(fmt + 1) == 's') {
        const char *v = va_arg(args, const char*);
        values.push_back(std::string(v));
        fmt++;
      }
    }
    fmt++;
  }
  va_end(args);

  UpdateText();
}

Footer::Footer()
: FreeWindow(0, 24, 80, 1, TYPE_NON_FOCUSABLE)
{
  SetColorScheme("footer");

  label = new CppConsUI::Label;
  AddWidget(*label, 0, 0);
}

void Footer::Init()
{
  g_assert(!instance);

  instance = new Footer;
  instance->Show();
}

void Footer::Finalize()
{
  g_assert(instance);

  delete instance;
  instance = NULL;
}

void Footer::UpdateText()
{
  if (values.empty()) {
    label->SetText(NULL);
    return;
  }

  Values::iterator i = values.begin();
  const char *fmt = i->c_str();
  i++;

  char out[1024];
  char *cur_out = out;

  while (*fmt && cur_out < out + sizeof(out) - 1) {
    if (*fmt == '%') {
      if (*(fmt + 1) == '%')
        fmt++;
      else if (*(fmt + 1) == 's') {
        char *con = g_strdup(i->c_str());
        i++;
        char *act = strstr(con, "|");
        g_assert(act);
        *act++ = '\0';
        const char *key = KEYCONFIG->GetKeyBind(con, act);
        g_free(con);

        strncpy(cur_out, key, out + sizeof(out) - 1 - cur_out);
        cur_out += strlen(key);

        fmt += 2;
        continue;
      }
    }
    *cur_out++ = *fmt++;
  }
  *cur_out = '\0';

  label->SetText(out);
}

/* vim: set tabstop=2 shiftwidth=2 tw=78 expandtab : */
