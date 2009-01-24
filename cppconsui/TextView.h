/* GTK - The GIMP Toolkit
 * gtktextview.h Copyright (C) 2000 Red Hat, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

/*
 * Modified by the GTK+ Team and others 1997-2000.  See the AUTHORS
 * file for a list of people on the GTK+ Team.  See the ChangeLog
 * files for a list of changes.  These files are distributed with
 * GTK+ at ftp://ftp.gtk.org/pub/gtk/.
 */

//#if defined(GTK_DISABLE_SINGLE_INCLUDES) && !defined (__GTK_H_INSIDE__) && !defined (GTK_COMPILATION)
//#error "Only <gtk/gtk.h> can be included directly."
//#endif

#ifndef __TEXTVIEW_H__
#define __TEXTVIEW_H__

//#include <gtk/gtkcontainer.h>
//#include <gtk/gtkimcontext.h>
#include "Widget.h"
#include "TextBuffer.h"
#include "TextTag.h"
//#include <gtk/gtkmenu.h>

#include <glib.h>

typedef enum
{
  JUSTIFY_LEFT,
  JUSTIFY_RIGHT,
  JUSTIFY_CENTER,
  JUSTIFY_FILL
} Justification;


#define GTK_TYPE_TEXT_VIEW             (gtk_text_view_get_type ())
#define GTK_TEXT_VIEW(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), GTK_TYPE_TEXT_VIEW, TextView))
#define GTK_TEXT_VIEW_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), GTK_TYPE_TEXT_VIEW, TextViewClass))
#define GTK_IS_TEXT_VIEW(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GTK_TYPE_TEXT_VIEW))
#define GTK_IS_TEXT_VIEW_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), GTK_TYPE_TEXT_VIEW))
#define GTK_TEXT_VIEW_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), GTK_TYPE_TEXT_VIEW, TextViewClass))

typedef enum
{
  GTK_TEXT_WINDOW_PRIVATE,
  GTK_TEXT_WINDOW_WIDGET,
  GTK_TEXT_WINDOW_TEXT,
  GTK_TEXT_WINDOW_LEFT,
  GTK_TEXT_WINDOW_RIGHT,
  GTK_TEXT_WINDOW_TOP,
  GTK_TEXT_WINDOW_BOTTOM
} GtkTextWindowType;

#define GTK_TEXT_VIEW_PRIORITY_VALIDATE (GDK_PRIORITY_REDRAW + 5)


//TODO port more of gtkadjustment.c/h
class Adjustment {
	public:
		Adjustment(gdouble lower, gdouble upper, gdouble value, gdouble step_increment, gdouble page_increment, gdouble page_size)
			:lower(lower), upper(upper), value(value), step_increment(step_increment)
			 ,page_increment(page_increment), page_size(page_size)
				{ ;}

		void set_value(gdouble value) { this->value = value;	}
		void adjustment_changed(void) { ; }
		void value_changed(void) { ; }
		void set_upper(gdouble upper) { ; }
		void set_lower(gdouble upper) { ; }

		gdouble lower;
		gdouble upper;
		gdouble value;
		gdouble step_increment;
		gdouble page_increment;
		gdouble page_size;

	protected:
	private:
};

typedef struct {
  TextMark   *mark;
  gdouble        within_margin;
  bool       use_align;
  gdouble        xalign;
  gdouble        yalign;
} TextPendingScroll;
 
class TextView
: public Widget
{
	public:
		TextView(Widget& parent, int x, int y, int w, int h, TextBuffer *buffer);
		TextView(Widget& parent, int x, int y, int w, int h);

	protected:

	private:
		void init(void);

		void ActionMoveCursor(CursorMovement step, int count, bool extend_selection);
		void ActionSelectAll(bool select_all);
		void ActionDelete(DeleteType type, gint count);
		void ActionBackspace(void);
		void ActionToggleOverwrite(void);

		void DeclareBindables(void);
		void BindActions(void);

		virtual void OnActivate(void);

  //guint blink_time;  /* time in msec the cursor has blinked since last user event */
  guint im_spot_idle;

  struct TextLayout *layout;
  TextBuffer *buffer;

  guint selection_drag_handler;
  guint scroll_timeout;

  /* Default style settings */
  gint pixels_above_lines;
  gint pixels_below_lines;
  gint pixels_inside_wrap;
  WrapMode wrap_mode;
  Justification justify;
  gint left_margin;
  gint right_margin;
  gint indent;
  //PangoTabArray *(tabs);
  bool editable;

  bool overwrite_mode ;//: 1;
  bool cursor_visible ;//: 1;

  /* if we have reset the IM since the last character entered */  
  bool need_im_reset ;//: 1;

  bool accepts_tab ;//: 1;

  bool width_changed ;//: 1;

  /* debug flag - means that we've validated onscreen since the
   * last "invalidate" signal from the layout
   */
  bool onscreen_validated;// : 1;

  bool mouse_cursor_obscured;// : 1;

  /*
  GtkTextWindow *GSEAL (text_window);
  GtkTextWindow *GSEAL (left_window);
  GtkTextWindow *GSEAL (right_window);
  GtkTextWindow *GSEAL (top_window);
  GtkTextWindow *GSEAL (bottom_window);*/

 
  Adjustment *hadjustment;
  Adjustment *vadjustment;

  gint xoffset;         /* Offsets between widget coordinates and buffer coordinates */
  gint yoffset;
  gint width;           /* Width and height of the buffer */ //TODO width of what? chars/pixels/columns?
  gint height;

  /* The virtual cursor position is normally the same as the
   * actual (strong) cursor position, except in two circumstances:
   *
   * a) When the cursor is moved vertically with the keyboard
   * b) When the text view is scrolled with the keyboard
   *
   * In case a), virtual_cursor_x is preserved, but not virtual_cursor_y
   * In case b), both virtual_cursor_x and virtual_cursor_y are preserved.
   */
  gint virtual_cursor_x;   /* -1 means use actual cursor position */
  gint virtual_cursor_y;   /* -1 means use actual cursor position */

  TextMark *first_para_mark; /* Mark at the beginning of the first onscreen paragraph */
  gint first_para_pixels;       /* Offset of top of screen in the first onscreen paragraph */

  TextMark *dnd_mark;
  guint blink_timeout;

  guint first_validate_idle;        /* Idle to revalidate onscreen portion, runs before resize */
  guint incremental_validate_idle;  /* Idle to revalidate offscreen portions, runs after redraw */

  //GtkIMContext *GSEAL (im_context);
  //TODO? GtkWidget *GSEAL (popup_menu);

  //gint GSEAL (drag_start_x);
  //gint GSEAL (drag_start_y);

  GSList *children;

  TextPendingScroll *pending_scroll;

  gint pending_place_cursor_button;

  //ContainerClass parent_class;

  void  set_scroll_adjustments   (TextView    *text_view,
                                     Adjustment  *hadjustment,
                                     Adjustment  *vadjustment);

  //void (* populate_popup)           (
  //                                   GtkMenu        *menu);

  /* These are all RUN_ACTION signals for keybindings */

  /* move insertion point */
  void move_cursor (
                        CursorMovement step,
                        gint            count,
                        bool        extend_selection);

  /* FIXME should be deprecated in favor of adding GTK_MOVEMENT_HORIZONTAL_PAGES
   * or something in GTK 2.2, was put in to avoid adding enum values during
   * the freeze.
   */
  void  page_horizontally (
                              gint         count,
                              bool     extend_selection);

  /* move the "anchor" (what Emacs calls the mark) to the cursor position */
  void  set_anchor  (void);

  /* Edits */
  void insert_at_cursor      (
                                  const gchar *str);
  void delete_from_cursor    (
                                  DeleteType type,
                                  gint          count);
  void backspace             (void);

  /* cut copy paste */
  /*
  void (* cut_clipboard)   (TextView *text_view);
  void (* copy_clipboard)  (TextView *text_view);
  void (* paste_clipboard) (TextView *text_view);
  */
  /* overwrite */
  void toggle_overwrite (void);

  /* as of GTK+ 2.12 the "move-focus" signal has been moved to GtkWidget,
   * so this is merley a virtual function now. Overriding it in subclasses
   * continues to work though.
   */
  //void (* move_focus)       (TextView     *text_view,
  //                           GtkDirectionType direction);

  /* Padding for future expansion *
  void (*_gtk_reserved1) (void);
  void (*_gtk_reserved2) (void);
  void (*_gtk_reserved3) (void);
  void (*_gtk_reserved4) (void);
  void (*_gtk_reserved5) (void);
  void (*_gtk_reserved6) (void);
  void (*_gtk_reserved7) (void);*/

  TextView (void);
  TextView (TextBuffer *buffer);
  ~TextView(void);

void set_buffer            ( TextBuffer *buffer);
TextBuffer *get_buffer            (void);
bool       scroll_to_iter        (
                                                    TextIter   *iter,
                                                    gdouble        within_margin,
                                                    bool       use_align,
                                                    gdouble        xalign,
                                                    gdouble        yalign);
void          scroll_to_mark        (
                                                    TextMark   *mark,
                                                    gdouble        within_margin,
                                                    bool       use_align,
                                                    gdouble        xalign,
                                                    gdouble        yalign);
void           scroll_mark_onscreen  (
                                                    TextMark   *mark);
bool       move_mark_onscreen    (
                                                    TextMark   *mark);
bool       place_cursor_onscreen (void);

void           get_visible_rect      ( Rect  *visible_rect);
void           set_cursor_visible    ( bool       setting);
bool       get_cursor_visible    (void);

void           get_iter_location     ( TextIter *iter,
                                                    Rect  *location);
void           get_iter_at_location  ( TextIter   *iter,
                                                    gint           x,
                                                    gint           y);
void           get_iter_at_position  (
                                                    TextIter   *iter,
						    gint          *trailing,
                                                    gint           x,
                                                    gint           y);
void           get_line_yrange       (
                                                    TextIter *iter,
                                                    gint              *y,
                                                    gint              *height);

void           get_line_at_y         (
                                                    TextIter       *target_iter,
                                                    gint               y,
                                                    gint              *line_top);

/*
void buffer_to_window_coords (
                                            GtkTextWindowType  win,
                                            gint               buffer_x,
                                            gint               buffer_y,
                                            gint              *window_x,
                                            gint              *window_y);
void window_to_buffer_coords (
                                            GtkTextWindowType  win,
                                            gint               window_x,
                                            gint               window_y,
                                            gint              *buffer_x,
                                            gint              *buffer_y);
*/

//GdkWindow*        get_window      (TextView       *text_view,
//                                                 GtkTextWindowType  win);
//GtkTextWindowType get_window_type (TextView       *text_view,
//                                                 GdkWindow         *window);

//void set_border_window_size (TextView       *text_view,
//                                           GtkTextWindowType  type,
//                                           gint               size);
//gint get_border_window_size (TextView       *text_view,
//					   GtkTextWindowType  type);

bool forward_display_line           ( TextIter       *iter);
bool backward_display_line          ( TextIter       *iter);
bool forward_display_line_end       ( TextIter       *iter);
bool backward_display_line_start    ( TextIter       *iter);
bool starts_display_line            ( TextIter *iter);
bool move_visually                  ( TextIter       *iter, gint               count);

/* Adding child widgets */
/*
void add_child_at_anchor (TextView          *text_view,
                                        GtkWidget            *child,
                                        GtkTextChildAnchor   *anchor);

void add_child_in_window (TextView          *text_view,
                                        GtkWidget            *child,
                                        GtkTextWindowType     which_window,
                                        * window coordinates *
                                        gint                  xpos,
                                        gint                  ypos);

void move_child          (TextView          *text_view,
                                        GtkWidget            *child,
                                        * window coordinates *
                                        gint                  xpos,
                                        gint                  ypos);
*/

/* Default style settings (fallbacks if no tag affects the property) */

void             set_wrap_mode          (
                                                       WrapMode       wrap_mode);
WrapMode      get_wrap_mode          (void);
void             set_editable           (
                                                       bool          setting);
bool         get_editable           (void);
void             set_overwrite          (
						       bool          overwrite);
bool         get_overwrite          (void);
void		 set_accepts_tab        ( bool		 accepts_tab);
bool	 get_accepts_tab        (void);
void             set_pixels_above_lines ( gint              pixels_above_lines);
gint             get_pixels_above_lines (void);
void             set_pixels_below_lines ( gint              pixels_below_lines);
gint             get_pixels_below_lines (void);
void             set_pixels_inside_wrap ( gint              pixels_inside_wrap);
gint             get_pixels_inside_wrap (void);
//void             set_justification      (TextView      *text_view,
//                                                       GtkJustification  justification);
//GtkJustification get_justification      (TextView      *text_view);
void             set_left_margin        (gint left_margin);
gint             get_left_margin        (void);
void             set_right_margin       (gint right_margin);
gint             get_right_margin       (void);
void             set_indent             (gint indent);
gint             get_indent             (void);
//void             set_tabs               ( PangoTabArray    *tabs);
//PangoTabArray*   get_tabs               (void);

/* note that the return value of this changes with the theme */
TextAttributes* get_default_attributes (void);

  private:

bool set_adjustment_clamped (Adjustment *adj, gdouble val);
void set_adjustment_upper (Adjustment *adj, gdouble upper);
/*static void gtk_text_view_set_property         (GObject         *object,
						guint            prop_id,
						const GValue    *value,
						GParamSpec      *pspec);
static void gtk_text_view_get_property         (GObject         *object,
						guint            prop_id,
						GValue          *value,
						GParamSpec      *pspec);*/

static void size_request         (
                                                Rect   *requisition);
//static void size_allocate        (
//                                                GtkAllocation    *allocation);

static void realize              (void);
static void unrealize            (void);
//static void style_set            (
//                                                GtkStyle         *previous_style);
//static void direction_changed    (
//                                                GtkTextDirection  previous_direction);
static void grab_notify          (
					        bool         was_grabbed);
//static void state_changed        (
//					        GtkStateType      previous_state);

/*
static gint event                ( GdkEvent         *event);
static gint key_press_event      ( GdkEventKey      *event);
static gint key_release_event    ( GdkEventKey      *event);
static gint button_press_event   ( GdkEventButton   *event);
static gint button_release_event ( GdkEventButton   *event);
static gint focus_in_event       ( GdkEventFocus    *event);
static gint focus_out_event      ( GdkEventFocus    *event);
static gint motion_event         ( GdkEventMotion   *event);
static gint expose_event         ( GdkEventExpose   *expose);
static void draw_focus           ( static bool focus            ( GtkDirectionType  direction);
static void move_focus           ( GtkDirectionType  direction_type);
static void select_all           ( bool          select);
*/

/* Source side drag signals */
/*
static void drag_begin       (GtkWidget        *widget,
                                            GdkDragContext   *context);
static void drag_end         (GtkWidget        *widget,
                                            GdkDragContext   *context);
static void drag_data_get    (GtkWidget        *widget,
                                            GdkDragContext   *context,
                                            GtkSelectionData *selection_data,
                                            guint             info,
                                            guint             time);
static void drag_data_delete (GtkWidget        *widget,
                                            GdkDragContext   *context);
					 */

/* Target side drag signals */
/*
static void     drag_leave         (GtkWidget        *widget,
                                                  GdkDragContext   *context,
                                                  guint             time);
static bool drag_motion        (GtkWidget        *widget,
                                                  GdkDragContext   *context,
                                                  gint              x,
                                                  gint              y,
                                                  guint             time);
static bool drag_drop          (GtkWidget        *widget,
                                                  GdkDragContext   *context,
                                                  gint              x,
                                                  gint              y,
                                                  guint             time);
static void     drag_data_received (GtkWidget        *widget,
                                                  GdkDragContext   *context,
                                                  gint              x,
                                                  gint              y,
                                                  GtkSelectionData *selection_data,
                                                  guint             info,
                                                  guint             time);
*/

void set_scroll_adjustments (
                                                  Adjustment *hadj,
                                                  Adjustment *vadj);
//static bool popup_menu         (GtkWidget     *widget);

bool move_viewport (
                                             ScrollStep          step,
                                             gint                   count);
bool scroll_pages (
                                            gint                   count,
                                            bool               extend_selection);
bool scroll_hpages(
                                            gint                   count,
                                            bool               extend_selection);
/*
static void cut_clipboard    (TextView           *text_view);
static void copy_clipboard   (TextView           *text_view);
static void paste_clipboard  (TextView           *text_view);*/
void toggle_cursor_visible (void);
void compat_move_focus(
                                            DirectionType       direction_type);
void unselect         (void);

void     validate_onscreen     (void);
void     get_first_para_iter   ( TextIter        *iter);
void     update_layout_width       (void);
//static void     set_attributes_from_style (
//                                                         TextAttributes *values,
//                                                         GtkStyle           *style);
void     ensure_layout          (void);
void     destroy_layout         (void);
void     check_keymap_direction (void);
void     reset_im_context       (void);
/*static void     start_selection_drag   (TextView        *text_view,
                                                      const TextIter  *iter,
                                                      GdkEventButton     *event);*/
bool end_selection_drag     (TextView        *text_view);
/*static void     start_selection_dnd    (TextView        *text_view,
                                                      const TextIter  *iter,
                                                      GdkEventMotion     *event);*/
void     check_cursor_blink     (void);
void     pend_cursor_blink      (void);
void     stop_cursor_blink      (void);
void     reset_blink_time       (void);

void     value_changed                (Adjustment *adj);
/*static void     commit_handler               (GtkIMContext  *context,
							    const gchar   *str,
							    TextView   *text_view);*/
void     commit_text                  (
                                                            const gchar   *text);
/*static void     preedit_changed_handler      (GtkIMContext  *context,
							    TextView   *text_view);*/
/*static bool retrieve_surrounding_handler (GtkIMContext  *context,
							    TextView   *text_view);*/
/*static bool delete_surrounding_handler   (GtkIMContext  *context,
							    gint           offset,
							    gint           n_chars,
							    TextView   *text_view);*/

void mark_set_handler       (TextBuffer     *buffer,
                                                  const TextIter *location,
                                                  TextMark       *mark,
                                                  gpointer           data);
void target_list_notify     ( void );


void get_cursor_location    ( Rect      *pos);
void get_virtual_cursor_pos ( gint              *x, gint              *y);
void set_virtual_cursor_pos ( gint               x, gint               y);

Adjustment* get_hadjustment            (void);
Adjustment* get_vadjustment            (void);

//static void do_popup               (
//						  GdkEventButton    *event);

void queue_scroll           (
                                                  TextMark   *mark,
                                                  gdouble        within_margin,
                                                  bool       use_align,
                                                  gdouble        xalign,
                                                  gdouble        yalign);

bool flush_scroll         (void);
void     update_adjustments   (void);
void     invalidate           (void);
void     flush_first_validate (void);

void update_im_spot_location (void);

/* Container methods */
/*
static void add    (GtkContainer *container,
                                  GtkWidget    *child);
static void remove (GtkContainer *container,
                                  GtkWidget    *child);
static void forall (GtkContainer *container,
                                  bool      include_internals,
                                  GtkCallback   callback,
                                  gpointer      callback_data);*/

/* FIXME probably need the focus methods. */

//typedef struct _TextViewChild TextViewChild;
/*
struct _TextViewChild
{
  GtkWidget *widget;

  GtkTextChildAnchor *anchor;

  gint from_top_of_line;
  gint from_left_of_buffer;
  
  * These are ignored if anchor != NULL *
  GtkTextWindowType type;
  gint x;
  gint y;
};*/
/*
static TextViewChild* text_view_child_new_anchored      (GtkWidget          *child,
							    GtkTextChildAnchor *anchor,
							    GtkTextLayout      *layout);
static TextViewChild* text_view_child_new_window        (GtkWidget          *child,
							    GtkTextWindowType   type,
							    gint                x,
							    gint                y);
static void              text_view_child_free              (TextViewChild   *child);
static void              text_view_child_set_parent_window (TextView        *text_view,
							    TextViewChild   *child);*/
/*
struct _GtkTextWindow
{
  GtkTextWindowType type;
  GtkWidget *widget;
  GdkWindow *window;
  GdkWindow *bin_window;
  Rect requisition;
  GdkRect allocation;
};*/
/*
static GtkTextWindow *text_window_new             (GtkTextWindowType  type,
                                                   GtkWidget         *widget,
                                                   gint               width_request,
                                                   gint               height_request);
static void           text_window_free            (GtkTextWindow     *win);
static void           text_window_realize         (GtkTextWindow     *win,
                                                   GtkWidget         *widget);
static void           text_window_unrealize       (GtkTextWindow     *win);
static void           text_window_size_allocate   (GtkTextWindow     *win,
                                                   GdkRect      *rect);
static void           text_window_scroll          (GtkTextWindow     *win,
                                                   gint               dx,
                                                   gint               dy);
static void           text_window_invalidate_rect (GtkTextWindow     *win,
                                                   GdkRect      *rect);
static void           text_window_invalidate_cursors (GtkTextWindow  *win);

static gint           text_window_get_width       (GtkTextWindow     *win);
static gint           text_window_get_height      (GtkTextWindow     *win);
*/

	void free_pending_scroll (TextPendingScroll *scroll);
	void cancel_pending_scroll (void);
	void flush_update_im_spot_location (void);
	bool clamp_iter_onscreen (TextIter *iter);
	void remove_validate_idles (void);
	bool first_validate_callback (gpointer data);
	void changed_handler (TextLayout     *layout,
                 gint               start_y,
                 gint               old_height,
                 gint               new_height,
                 gpointer           data);
	bool incremental_validate_callback (gpointer data);

	bool move_iter_by_lines ( TextIter *newplace, gint count);

	void some_move_cursor ( TextIter *new_location, bool extend_selection);
	void move_cursor_internal (
                                    CursorMovement  step,
                                    gint             count,
                                    bool         extend_selection);

	static bool whitespace (gunichar ch, gpointer user_data);
	static bool not_whitespace (gunichar ch, gpointer user_data);
	bool find_whitepace_region (const TextIter *center, TextIter *start, TextIter *end);

	//void move_mark_to_pointer_and_scroll ( const gchar *mark_name);
	bool selection_scan_timeout (gpointer data);
	static bool check_scroll (gdouble offset, Adjustment *adj);

typedef enum 
{
  SELECT_CHARACTERS,
  SELECT_WORDS,
  SELECT_LINES
} SelectionGranularity;

typedef struct
{
  SelectionGranularity granularity;
  TextMark *orig_start;
  TextMark *orig_end;
} SelectionData;



	void extend_selection (
		  SelectionGranularity granularity, 
		  TextIter *start, 
		  TextIter *end);

	void selection_data_free (SelectionData *data);

	bool range_contains_editable_text (const TextIter *start,
                              TextIter *end,
                              bool default_editability);

};

#endif /* __TEXTVIEW_H__ */
