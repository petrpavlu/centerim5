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
 * Widget class.
 *
 * @ingroup cppconsui
 */

#ifndef __WIDGET_H__
#define __WIDGET_H__

#define AUTOSIZE -1024
#define UNSETPOS -2048

#include "ConsUICurses.h"
#include "CppConsUI.h"
#include "InputProcessor.h"

namespace CppConsUI
{

class Container;
class Window;

class Widget
: public sigc::trackable
, public InputProcessor
{
public:
  Widget(int w, int h);
  virtual ~Widget();

  /**
   * Moves and resizes the widget.
   */
  virtual void moveResize(int newx, int newy, int neww, int newh);
  /**
   * Recreates the widget area. It is called whenever coordinates of the
   * widget change.
   */
  virtual void updateArea();
  /**
   * The draw() method does the actual drawing on a (virtual) area of the
   * screen. The @ref CoreManager singleton calls draw() on all on-screen
   * Windows. This causes all draw() implementations needed to draw the screen
   * to be called.
   */
  virtual void draw() = 0;
  /**
   * Finds the widget that could be the focus widget from the focus chain
   * starting with this widget:
   */
  virtual Widget *getFocusWidget();
  /**
   * Deletes the focus (and input) chain starting from this widget.
   */
  virtual void cleanFocus();
  /**
   * @todo
   */
  virtual bool restoreFocus();
  /**
   * Takes focus from the widget. Used when this window is no longer a top
   * window.
   */
  virtual void ungrabFocus();
  /**
   * Makes this widget the widget that has the focus. This operation is
   * successful if the widget is visible and all predecessors are visible too.
   */
  virtual bool grabFocus();

  virtual bool canFocus() const { return can_focus; }
  virtual bool hasFocus() const { return has_focus; }

  virtual void setVisibility(bool new_visible);
  virtual bool isVisible() const { return visible; }
  virtual bool isVisibleRecursive() const;

  virtual void setParent(Container& new_parent);
  virtual Container *getParent() const { return parent; }

  virtual int getLeft() const { return xpos; }
  virtual int getTop() const { return ypos; }
  virtual int getWidth() const { return width; }
  virtual int getHeight() const { return height; }

  /**
   * Convenient moveResize() wrapper.
   */
  virtual void move(int newx, int newy);
  /**
   * Convenient moveResize() wrapper.
   */
  virtual void resize(int neww, int newh);
  /**
   * Convenient moveResize() wrapper.
   */
  virtual void setLeft(int newx);
  /**
   * Convenient moveResize() wrapper.
   */
  virtual void setTop(int newy);
  /**
   * Convenient moveResize() wrapper.
   */
  virtual void setWidth(int neww);
  /**
   * Convenient moveResize() wrapper.
   */
  virtual void setHeight(int newh);

  /**
   * Returns an absolute position of the widget. Note: special care has to be
   * taken when calling this method because CppConsUI uses deferred drawing.
   * The returned value is correct only if the drawing of all parent widgets
   * already occured.
   */
  virtual Point getAbsolutePosition() const;
  /**
   * Returns a relative position of the widget to a given predecessor. See
   * note in getAbsolutePosition() method.
   */
  virtual Point getRelativePosition(const Container& ref) const;
  /**
   * Returns a real (on-screen) width. See note in getAbsolutePosition()
   * method.
   */
  virtual int getRealWidth() const;
  /**
   * Returns a real (on-screen) height. See note in getAbsolutePosition()
   * method.
   */
  virtual int getRealHeight() const;

  /**
   * Returns an area width that is requested by the widget. This method can
   * be used by a parent widget in getSubPad() if width is set to AUTOSIZE.
   * The AUTOSIZE returned value means "as much as possible".
   */
  virtual int getWishWidth() const { return wish_width; }
  /**
   * Returns an area height that is requested by the widget. This method can
   * be used by a parent widget in getSubPad() if height is set to AUTOSIZE.
   * The AUTOSIZE returned value means "as much as possible".
   */
  virtual int getWishHeight() const { return wish_height; }

  virtual void setAutoSize(int neww, int newh);
  virtual void setAutoWidth(int neww) { setAutoSize(neww, auto_height); }
  virtual void setAutoHeight(int newh) { setAutoSize(auto_width, newh); }
  virtual int getAutoWidth() const { return auto_width; }
  virtual int getAutoHeight() const { return auto_height; }

  virtual void startPositioning();
  virtual void finishPositioning();

  virtual void setColorScheme(const char *new_color_scheme);
  virtual const char *getColorScheme() const;

  /**
   * Signal emitted whenever a widget grabs or looses the focus.
   */
  sigc::signal<void, Widget&, bool> signal_focus;
  /**
   * Signal emmited whenever a visible property is changed.
   */
  sigc::signal<void, Widget&, bool> signal_visible;

protected:
  /**
   * Screen area relative to parent area. Note that this is a requested area
   * if parent window is big enough, real width/height can differ.
   */
  int xpos, ypos, width, height;
  /**
   * Preferable size calculated by the widget.
   */
  int wish_width, wish_height;
  /**
   * Automatic size set by the parent.
   */
  int auto_width, auto_height;
  /**
   * Flag indicating if the parent currently positions the widget and
   * updateArea() should not be called yet.
   */
  bool in_positioning_process;
  /**
   * Flag indicating if updateArea() should be called when the positioning
   * process is ended.
   */
  bool update_area_pending;
  /**
   * Flag indicating if the widget can grab the focus.
   */
  bool can_focus;
  /**
   * Flag indicating if the widget has the focus. Only one widget can have the
   * focus in the application.
   */
  bool has_focus;
  /**
   * Visibility flag.
   */
  bool visible;
  /**
   * This is the implementation dependent area of the widget.
   */
  Curses::Window *area;

  /**
   * Parent widget.
   */
  Container *parent;
  /**
   * Current color scheme.
   */
  char *color_scheme;

  /**
   * Tells @ref CoreManager that the widget has been updated and the screen
   * should be redrawn.
   */
  virtual void redraw();

  virtual void setWishSize(int neww, int newh);
  virtual void setWishWidth(int neww) { setWishSize(neww, wish_height); }
  virtual void setWishHeight(int newh) { setWishSize(wish_width, newh); }

  /**
   * Convenient method that calls COLORSCHEME->getColorPair(getColorScheme(),
   * widget, property).
   */
  virtual int getColorPair(const char *widget, const char *property) const;

  /**
   * @todo
   */
  virtual Container *getTopContainer();

private:
  CONSUI_DISABLE_COPY(Widget);
};

} // namespace CppConsUI

#endif // __WIDGET_H__

/* vim: set tabstop=2 shiftwidth=2 textwidth=78 expandtab : */
