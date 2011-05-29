/*
 * Copyright (C) 2011 by CenterIM developers
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

#include "Footer.h"

#include "CenterIM.h"

#include <cppconsui/KeyConfig.h>
#include "gettext.h"

Footer *Footer::instance = NULL;

Footer *Footer::Instance()
{
  return instance;
}

void Footer::ScreenResized()
{
  MoveResizeRect(CENTERIM->GetScreenAreaSize(CenterIM::FOOTER_AREA));
}

void Footer::SetText(const char *text_)
{
  if (text)
    g_free(text);
  if (text_)
    text = g_strdup(text_);
  else
    text = NULL;

  UpdateText();
}

Footer::Footer()
: FreeWindow(0, 24, 80, 1, TYPE_NON_FOCUSABLE), text(NULL)
{
  SetColorScheme("footer");

  label = new CppConsUI::Label;
  AddWidget(*label, 0, 0);
}

Footer::~Footer()
{
  if (text)
    g_free(text);
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
  if (!text) {
    label->SetText(NULL);
    return;
  }

  char out[1024];
  char *cur_out = out;

  const char *cur_text = text;
  char *context = NULL;
  char *action = NULL;

#define ACCEPT(ch)       \
do {                     \
  if (*cur_text != (ch)) \
    goto error;          \
  cur_text++;            \
} while(0)

  while (*cur_text && cur_out < out + sizeof(out) - 1) {
    if (*cur_text != '<') {
      *cur_out++ = *cur_text++;
      continue;
    }

    cur_text++;
    context = ParseName(&cur_text);
    ACCEPT('|');
    action = ParseName(&cur_text);
    ACCEPT('>');

    char *key = KEYCONFIG->GetKeyBind(context, action);
    if (key) {
      strncpy(cur_out, key, out + sizeof(out) - 1 - cur_out);
      cur_out += strlen(key);
      g_free(key);
    }
    else {
      const char *unbind = _("<unbind>");
      strncpy(cur_out, unbind, out + sizeof(out) - 1 - cur_out);
      cur_out += strlen(unbind);
    }

    g_free(context);
    context = NULL;
    g_free(action);
    action = NULL;

  }
  *cur_out = '\0';

  label->SetText(out);
  return;

error:
  if (context)
    g_free(context);
  if (action)
    g_free(action);
  label->SetText(_("Malformed help message"));
}

char *Footer::ParseName(const char **text)
{
  const char *start = *text;

  while (isalpha((unsigned char) **text) || **text == '-')
    (*text)++;
  return g_strndup(start, *text - start);
}
