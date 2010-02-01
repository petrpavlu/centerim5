/*
 * Copyright (C) 2007 by Mark Pustjens <pustjens@dds.nl>
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
/** @file Widget.h Widget class
 * @ingroup cppconsui
 */

#ifndef __WIDGET_H__
#define __WIDGET_H__

#include "CppConsUI.h"
#include "InputProcessor.h"
#include "ColorScheme.h"

#include <sigc++/sigc++.h>
#include <sigc++/signal.h>

class Widget
: public sigc::trackable
, public InputProcessor
{
	public:
		typedef enum {
			FocusNext,
			FocusPrevious,
			FocusDown,
			FocusUp,
			FocusRight,
			FocusLeft
		} FocusDirection;

		/** All widgets must have a parent. 
		 * The test for the root Widget is (parent == this), so the 
		 * root object should initialize the Widget with *this as the parent.
		 * @todo use the Point and Rect classes, perhaps implement a Size class 
		 *   hold width and height variables, to enable easier usage
		 */
		Widget(Widget& parent, int x_, int y_, int w_, int h_);
		virtual ~Widget();

		/** This method, as well as Resize(), MoveResize() 
		 * are the methods to be called when the coordinates of the Widget should change.
		 * they emit the appropriate signals after calling UpdateArea()
		 */
		virtual void Move(const int newx, const int newy);
		virtual void Resize(const int neww, const int newh);
		virtual void MoveResize(const int newx, const int newy, const int neww, const int newh);
		/** The @ref area is recreated, the new area should be redrawn.
		 * It is called whenever the coordinates of the widget change. The caller must ensure the 
		 * proper signals are emitted.
		 */
		virtual void UpdateArea();

		/* The difference between the Draw() and Redraw() functions should be
		 * clarified.
		 */
	
		/**  The Draw() function does the actual drawing on some (virtual) area
		 * of the screen. The WindowMananger object calls Draw() on all on-screen
		 * Windows. This causes all Draw() implementations needed to draw the
		 * screen to be called.
		 */
		virtual void Draw(void);
		/** The Redraw() function can be called by a Widget to tell its 
		 * @ref Container object that the Widget has been updated and that it should
		 * be redrawn. This implies that the @ref Container of a Widget should
		 * connect to the @ref signal_redraw.
		 */
		void Redraw(void);
		/** Resets the focus child by @ref StealFocus "stealing" the focus 
		 * from the current chain and also ensures the focus 
		 * goes also UP the chain to the root widget. (normally a Window)
		 * 
		 * @todo It must be always true that if the focus is stolen from the child, then the call
		 * parent->SetFocusChild(this) always succeeds. (Otherwise, the focus is not set, but stolen from the current widget)
		 * The child Widget should not be null (perhaps reference ?)
		 */
		virtual bool SetFocusChild(Widget* child);
		/** It deletes the focus (and input) chain starting from this widget.
		 * @return true if the focus belonged to this widget's chain and was 
		 * successfuly removed
		 */
		virtual bool StealFocus(void);
		/** Sets the focus to the FocusWidget (@ref GetFocusWidget)
		 * @todo if no focus widget exists in the chain, it silently 
		 * returns with no indication of this
		 */
		void RestoreFocus(void);
		/** finds the widget that could be the focus widget from the 
		 * focus chain starting with this widget
		 */
		Widget* GetFocusWidget(void);
		/** Makes this widget the widget that has focus,
		 * only if the parent window has focus.
		 * @todo Difference between this and StealFocus() ?
		 */
		virtual bool GrabFocus(void);
		/** Takes focus from the widget.
		 * @todo perhaps find a better name for this one
		 */
		void UngrabFocus(void);
		/** Explain a bit the algoritm
		 */
		virtual void MoveFocus(FocusDirection direction);
		
		bool CanFocus(void) const { return can_focus; }
		/** @todo check if this CanFocus() should be public
		 * instead of protected
		 */
		void CanFocus(bool val) { can_focus = val; }
		bool HasFocus(void) const { return has_focus; }
		const Widget* Parent(void) const { return parent; }

		int Left() const { return x; }
		int Top() const { return y; }
		int X() const { return x; }
		int Y() const { return y; }
		int Width() { return w; }
		int Height() { return h; }

		/** Returns a subpad of current widget with given coordinates.
		 */
		virtual Curses::Window *GetSubPad(const Widget &child, int begin_x, int begin_y, int ncols, int nlines);

		/// @todo encapsulate with a function, make sure derived class call Move()/Resize()/Redraw() to emit signal.
		/// Also check if this is possible at all.
		sigc::signal<void, Widget*, Point&, Point&> signal_move;
		sigc::signal<void, Widget*, Rect&, Rect&> signal_resize;
		sigc::signal<void, Widget*> signal_redraw;
		sigc::signal<void, Widget*, bool> signal_focus;

	protected:
		/** Screen area relative to parent area. Note that this is a requested
		 * area if parent window is big enough, real width/height can differ.
		 */
		int x, y, w, h;

		bool can_focus;
		/** Flag if the widget has the focus. Only one widget can have the focus in the application.
		 *  (check if this is a condition)
		 */
		bool has_focus;
		/** This defines a chain of focus
		 * @todo explain the difference between this chain and @ref InputProcessor::inputchild
		 * Isn't this a duplication of functionality from inputchild ?
  		 */
		Widget *focus_child;
		/** This is the implementation dependent area of the widget
		 */
		Curses::Window *area;
		/** Used to define a tree of Widgets
		 * @todo make parent a reference
		 */
		Widget *parent;

		ColorScheme *colorscheme;

	private:
		Widget();
		Widget(const Widget &);

		Widget& operator=(const Widget&);
};

#endif /* __WIDGET_H__ */
