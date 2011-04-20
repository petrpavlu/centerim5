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

Footer *Footer::instance = NULL;

Footer *Footer::Instance()
{
  return instance;
}

void Footer::ScreenResized()
{
  MoveResizeRect(CENTERIM->GetScreenAreaSize(CenterIM::FOOTER_AREA));
}

void Footer::SetText(const char *text)
{
  label->SetText(text);
}

const char *Footer::GetText() const
{
  return label->GetText();
}

Footer::Footer()
: FreeWindow(0, 24, 80, 1, TYPE_NON_FOCUSABLE)
{
  SetColorScheme("footer");

  label = new Label;
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
