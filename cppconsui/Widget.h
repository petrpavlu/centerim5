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
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

/// @file
/// Widget class.
///
/// @ingroup cppconsui

#ifndef WIDGET_H
#define WIDGET_H

#include "ConsUICurses.h"
#include "CppConsUI.h"
#include "InputProcessor.h"

#include <vector>

#define DRAW(call)                                                             \
  do {                                                                         \
    if (call != 0)                                                             \
      return error.getCode();                                                  \
  } while (0)

namespace CppConsUI {

class Container;
class Widget;
class Window;

typedef std::vector<Widget *> Widgets;

class Widget : public sigc::trackable, public InputProcessor {
public:
  enum {
    AUTOSIZE = -1024,
    UNSETPOS = -2048,
  };

  Widget(int w, int h);
  virtual ~Widget() override;

  /// Moves and resizes the widget.
  virtual void moveResize(int newx, int newy, int neww, int newh);
  virtual void moveResizeRect(const Rect &rect)
  {
    moveResize(rect.x, rect.y, rect.width, rect.height);
  }

  /// The draw() method does the actual drawing on a (virtual) area of the
  /// screen. The @ref CoreManager singleton calls draw() on all on-screen
  /// Windows. This causes all draw() implementations needed to draw the screen
  /// to be called.
  virtual int draw(Curses::ViewPort area, Error &error) = 0;

  /// Finds the widget that could be the focus widget from the focus chain
  /// starting with this widget.
  virtual Widget *getFocusWidget();

  /// Deletes the focus (and input) chain starting from this widget.
  virtual void cleanFocus();

  /// @todo
  virtual bool restoreFocus();

  /// Takes focus from the widget. Used when this window is no longer a top
  /// window.
  virtual void ungrabFocus();

  /// Makes this widget the widget that has the focus. This operation is
  /// successful if the widget is visible and all predecessors are visible too.
  virtual bool grabFocus();

  virtual bool canFocus() const { return can_focus_; }
  virtual bool hasFocus() const { return has_focus_; }

  virtual void setVisibility(bool new_visible);
  virtual bool isVisible() const { return visible_; }
  virtual bool isVisibleRecursive() const;

  virtual void setParent(Container &new_parent);
  virtual Container *getParent() const { return parent_; }

  virtual int getLeft() const { return xpos_; }
  virtual int getTop() const { return ypos_; }
  virtual int getWidth() const { return width_; }
  virtual int getHeight() const { return height_; }

  /// Convenient moveResize() wrapper.
  virtual void move(int newx, int newy);

  /// Convenient moveResize() wrapper.
  virtual void resize(int neww, int newh);

  /// Convenient moveResize() wrapper.
  virtual void setLeft(int newx);

  /// Convenient moveResize() wrapper.
  virtual void setTop(int newy);

  /// Convenient moveResize() wrapper.
  virtual void setWidth(int neww);

  /// Convenient moveResize() wrapper.
  virtual void setHeight(int newh);

  /// Returns an absolute position of the widget.
  virtual Point getAbsolutePosition() const;

  /// Returns a relative position of the widget to a given predecessor.
  virtual Point getRelativePosition(const Container &ref) const;

  /// Returns an area width that is requested by the widget. This method can be
  /// used by a parent widget if width is set to AUTOSIZE. The AUTOSIZE returned
  /// value means "as much as possible".
  virtual int getWishWidth() const { return wish_width_; }

  /// Returns an area height that is requested by the widget. This method can be
  /// used by a parent widget in if height is set to AUTOSIZE. The AUTOSIZE
  /// returned value means "as much as possible".
  virtual int getWishHeight() const { return wish_height_; }

  virtual void setRealPosition(int newx, int newy);
  virtual int getRealLeft() const { return real_xpos_; }
  virtual int getRealTop() const { return real_ypos_; }
  virtual void setRealSize(int neww, int newh);
  virtual int getRealWidth() const { return real_width_; }
  virtual int getRealHeight() const { return real_height_; }

  virtual void setColorScheme(int new_color_scheme);
  virtual int getColorScheme() const;

  virtual void registerAbsolutePositionListener(Widget &widget);
  virtual void unregisterAbsolutePositionListener(Widget &widget);
  virtual void onAbsolutePositionChange(Widget &widget);

  /// Signal emitted when the widget grabs or looses the focus.
  sigc::signal<void, Widget &, bool> signal_focus;

  /// Signal emitted when the visibility property of the widget is changed.
  sigc::signal<void, Widget &, bool> signal_visible;

protected:
  /// Screen area relative to parent area. Note that this is a requested area if
  /// parent window is big enough, real width/height can differ.
  int xpos_, ypos_, width_, height_;

  /// Preferable size calculated by the widget.
  int wish_width_, wish_height_;

  /// Real position and size set by the parent container.
  int real_xpos_, real_ypos_, real_width_, real_height_;

  /// Flag indicating if the widget can grab the focus.
  bool can_focus_;

  // Flag indicating if the widget has the focus. Only one widget can have the
  // focus in the application.
  bool has_focus_;

  /// Visibility flag.
  bool visible_;

  /// Parent widget.
  Container *parent_;

  /// Current color scheme ID.
  int color_scheme_;

  /// Vector of widgets that need to be informed when the absolute on-screen
  /// position of this widget has changed. This is used by widgets that need to
  /// position itself relative to another widget, for example, MenuWindow
  /// relative to the originating ComboBox.
  Widgets absolute_position_listeners_;

  virtual void signalMoveResize(const Rect &oldsize, const Rect &newsize);
  virtual void signalWishSizeChange(const Size &oldsize, const Size &newsize);
  virtual void signalVisible(bool visible);
  virtual void signalAbsolutePositionChange();

  /// Recalculates the widget area. It is called whenever coordinates of the
  /// widget change.
  virtual void updateArea();

  /// Informs @ref CoreManager that the widget has been updated and the screen
  /// should be redrawn.
  virtual void redraw();

  virtual void setWishSize(int neww, int newh);
  virtual void setWishWidth(int neww) { setWishSize(neww, wish_height_); }
  virtual void setWishHeight(int newh) { setWishSize(wish_width_, newh); }

  /// Convenient method that calls getAttributes(property, 0, attrs, error).
  virtual int getAttributes(int property, int *attrs, Error &error) const;
  /// Convenient method that calls COLORSCHEME->getAttributes(getColorScheme(),
  /// property, subproperty, attrs, error).
  virtual int getAttributes(
    int property, int subproperty, int *attrs, Error &error) const;

  /// Returns the top container.
  virtual Container *getTopContainer();

private:
  CONSUI_DISABLE_COPY(Widget);
};

} // namespace CppConsUI

#endif // WIDGET_H

// vim: set tabstop=2 shiftwidth=2 textwidth=80 expandtab:
