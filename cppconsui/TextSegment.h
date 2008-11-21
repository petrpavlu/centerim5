/* GTK - The GIMP Toolkit
 * gtktextsegment.h Copyright (C) 2000 Red Hat, Inc.
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

#ifndef __GTK_TEXT_SEGMENT_H__
#define __GTK_TEXT_SEGMENT_H__

#include "TextMark.h"
#include "TextTag.h"
#include "TextBTree.h"
#include "TextIter.h"
//#include <gtk/gtktextchild.h>
//#include <gtk/gtktextchildprivate.h>

//G_BEGIN_DECLS

class TextMark;
class TextBTree;
class TextLine;

struct TextMarkBody {
  TextMark *obj;
  gchar *name;
  TextBTree *tree;
  TextLine *line;
  guint visible : 1;
  guint not_deleteable : 1;
};

typedef enum {
	text_segment_none,
	text_segment_char,
	text_segment_toggle_on,
	text_segment_toggle_off,
	text_segment_left_mark,
	text_segment_right_mark
} TextLineSegmentType;


/*
 * Segments: each line is divided into one or more segments, where each
 * segment is one of several things, such as a group of characters, a
 * tag toggle, a mark, or an embedded widget.  Each segment starts with
 * a standard header followed by a body that varies from type to type.
 */

/* This header has the segment type, and two specific segments
   (character and toggle segments) */

class TextBTreeNode;
class TextMark;

/* Information a BTree stores about a tag. */
//typedef _TextTagInfo TextTagInfo
class TextTagInfo {
	public:
  TextTag *tag;
  TextBTreeNode *tag_root; /* highest-level node containing the tag */
  gint toggle_count;      /* total toggles of this tag below tag_root */
	protected:
	private:
};

/* Body of a segment that toggles a tag on or off */
struct TextToggleBody {
  TextTagInfo *info;             /* Tag that starts or ends here. */
  bool inNodeCounts;             /* TRUE means this toggle has been
                                      * accounted for in node toggle
                                      * counts; FALSE means it hasn't, yet. */
};

class TextLineSegment;

/* Class struct for segments */
//TODO use sigc for this?


//TODO move inside textsegment class
/*
typedef struct {
  char *name;                           * Name of this kind of segment. *
  bool leftGravity;                 * If a segment has zero size (e.g. a
                                         * mark or tag toggle), does it
                                         * attach to character to its left
                                         * or right?  1 means left, 0 means
                                         * right. *
  TextSegSplitFunc splitFunc;        * Procedure to split large segment
                                         * into two smaller ones. *
  TextSegDeleteFunc deleteFunc;      * Procedure to call to delete
                                         * segment. *
  TextSegCleanupFunc cleanupFunc;   * After any change to a line, this
                                        * procedure is invoked for all
                                        * segments left in the line to
                                        * perform any cleanup they wish
                                        * (e.g. joining neighboring
                                        * segments). *
  TextSegLineChangeFunc lineChangeFunc;
  * Invoked when a segment is about
   * to be moved from its current line
   * to an earlier line because of
   * a deletion.  The line is that
   * for the segment's old line.
   * CleanupFunc will be invoked after
   * the deletion is finished. *

  TextSegCheckFunc checkFunc;       * Called during consistency checks
                                        * to check internal consistency of
                                        * segment. *
} TextLineSegmentType;*/

/*
 * The data structure below defines line segments.
 */

class TextIter;

class TextLineSegment
{ 
	public:

		TextLineSegment(const gchar *text, guint len);
		TextLineSegment(const gchar *text1, guint len1, guint chars1,
					const gchar *text2, guint len2, guint chars2);
		TextLineSegment(TextTagInfo *info, bool on);
		TextLineSegment(TextMark *mark_obj);

		static TextLineSegment* split_segment(TextIter *iter);

		TextLineSegmentType type;  /* Pointer to record describing segment's type. */

  TextLineSegment *next;             /* Next in list of segments for this
                                         * line, or NULL for end of list. */

  int char_count;                       /* # of chars of index space occupied */

  int byte_count;                       /* Size of this segment (# of bytes
                                         * of index space it occupies). */
  //TODO also add int column_count; /* Size of this segment in terminal columns. */

  //TODO no union please
  union {
    gchar *chars;                      /* Characters that make up character
                                         * info.  Actual length varies to
                                         * hold as many characters as needed.*/
    TextToggleBody toggle;              /* Information about tag toggle. */
    TextMarkBody mark;              /* Information about mark. */
    //GtkTextPixbuf pixbuf;              /* Child pixbuf */
    //GtkTextChildBody child;            /* Child widget */
  } body;

  	//TODO this was moved here from textmark.h
		void set_tree ( TextBTree       *tree);

		//from the old struct textlinesegmenttype
		char *name;
		bool leftGravity;

		/* Split seg at index, returning list of two new segments, and freeing seg */
		virtual TextLineSegment* split   (int                index);

		/* Delete seg which is contained in line; if tree_gone, the tree is being
		 * freed in its entirety, which may matter for some reason (?)
		 * Return TRUE if the segment is not deleteable, e.g. a mark.
		 */
		virtual bool deleteFunc (TextLine *line, bool tree_gone);

		/* Called after segment structure of line changes, so segments can
		 * cleanup (e.g. merge with adjacent segments). Returns a segment list
		 * to replace the original segment list with. The line argument is
		 * the current line.
		 */
		virtual TextLineSegment* cleanupFunc (TextLineSegment *segPtr, TextLine *line);

		/* Called when a segment moves from one line to another. CleanupFunc is also
		 * called in that case, so many segments just use CleanupFunc, I'm not sure
		 * what's up with that (this function may not be needed...)
		 */
		virtual void lineChangeFunc (TextLine *line);

		/* Called to do debug checks on the segment. */
		virtual void checkFunc (TextLine *line);


	protected:
  		TextLineSegment();

	private:
		TextLineSegment(const TextLineSegment&);

		TextLineSegment& operator=(const TextLineSegment&);
		void self_check (void);
};

/*
TextLineSegment *_gtk_char_segment_new                  (const gchar    *text,
                                                            guint           len);
TextLineSegment *_gtk_char_segment_new_from_two_strings (const gchar    *text1,
                                                            guint           len1,
							    guint           chars1,
                                                            const gchar    *text2,
                                                            guint           len2,
							    guint           chars2);
TextLineSegment *_gtk_toggle_segment_new                (TextTagInfo *info,
                                                            bool        on);
*/


//TODO rename to textlinesegmentmark, or also add a leftmark
//when renaming: leftgravity is no longer needed, as it is always true
class TextLineSegmentRightMark
: public TextLineSegment
{ 
	public:
		TextLineSegmentRightMark(const gchar *text, guint len);
		TextLineSegmentRightMark(const gchar *text1, guint len1, guint chars1,
					const gchar *text2, guint len2, guint chars2);
		TextLineSegmentRightMark(TextTagInfo *info, bool on);
		TextLineSegmentRightMark (TextMark *mark_obj);

		static TextLineSegment* split(const TextIter *iter);

		//TextLineSegment* split (gint index);
		bool deleteFunc (TextLineSegmentRightMark *segPtr, TextLine *line, bool tree_gone);
		TextLineSegment* cleanupFunc (TextLineSegmentRightMark *segPtr, TextLine *line);
		//void lineChangeFunc (TextLine *line);
		void checkFunc (TextLine *line);

	protected:

	private:
  		TextLineSegmentRightMark();
		TextLineSegmentRightMark(const TextLineSegment&);

		TextLineSegment& operator=(const TextLineSegment&);

		void self_check (void);
};

class TextLineSegmentChar
: public TextLineSegment
{ 
	public:
		TextLineSegmentChar(const gchar *text, guint len);
		TextLineSegmentChar(const gchar *text1, guint len1, guint chars1,
					const gchar *text2, guint len2, guint chars2);
		TextLineSegmentChar(TextTagInfo *info, bool on);
		TextLineSegmentChar (TextMark *mark_obj);

		static TextLineSegment* split(const TextIter *iter);

		TextLineSegment* split (gint index);
		bool deleteFunc (TextLineSegmentChar *segPtr, TextLine *line, bool tree_gone);
		TextLineSegment* cleanupFunc (TextLineSegmentChar *segPtr, TextLine *line);
		void checkFunc (TextLine *line);

	protected:

	private:
  		TextLineSegmentChar();
		TextLineSegmentChar(const TextLineSegment&);

		TextLineSegment& operator=(const TextLineSegment&);

		void self_check (void);
};

class TextLineSegmentToggle
: public TextLineSegment
{ 
	public:
		TextLineSegmentToggle(const gchar *text, guint len);
		TextLineSegmentToggle(const gchar *text1, guint len1, guint chars1,
					const gchar *text2, guint len2, guint chars2);
		TextLineSegmentToggle(TextTagInfo *info, bool on);
		TextLineSegmentToggle (TextMark *mark_obj);

		static TextLineSegment* split(const TextIter *iter);

		//TextLineSegment* split (gint index);
		bool deleteFunc (TextLineSegmentToggle *segPtr, TextLine *line, bool tree_gone);
		TextLineSegment* cleanupFunc (TextLineSegmentToggle *segPtr, TextLine *line);
		void lineChangeFunc (TextLine *line);
		void checkFunc (TextLine *line);

	protected:

	private:
  		TextLineSegmentToggle();
		TextLineSegmentToggle(const TextLineSegment&);

		TextLineSegment& operator=(const TextLineSegment&);

		void self_check (void);
};

/*
const GtkTextLineSegmentClass gtk_text_char_type = {
  "character",                          * name *
  0,                                            * leftGravity *
  char_segment_split_func,                              * splitFunc *
  char_segment_delete_func,                             * deleteFunc *
  char_segment_cleanup_func,                            * cleanupFunc *
  NULL,         					* lineChangeFunc *
  char_segment_check_func                               * checkFunc *
};

*
 * Type record for segments marking the beginning of a tagged
 * range:
 *

const GtkTextLineSegmentClass gtk_text_toggle_on_type = {
  "toggleOn",                                   * name *
  0,                                            * leftGravity *
  NULL,                 * splitFunc *
  toggle_segment_delete_func,                           * deleteFunc *
  toggle_segment_cleanup_func,                          * cleanupFunc *
  toggle_segment_line_change_func,                      * lineChangeFunc *
  _gtk_toggle_segment_check_func                        * checkFunc *
};

*
 * Type record for segments marking the end of a tagged
 * range:
 *

const GtkTextLineSegmentClass gtk_text_toggle_off_type = {
  "toggleOff",                          * name *
  1,                                            * leftGravity *
  NULL,                 			* splitFunc *
  toggle_segment_delete_func,                           * deleteFunc *
  toggle_segment_cleanup_func,                          * cleanupFunc *
  toggle_segment_line_change_func,                      * lineChangeFunc *
  _gtk_toggle_segment_check_func                        * checkFunc *
};

const GtkTextLineSegmentClass gtk_text_right_mark_type = {
  "mark",                                               * name *
  FALSE,                                                * leftGravity *
  NULL,                                         * splitFunc *
  mark_segment_delete_func,                             * deleteFunc *
  mark_segment_cleanup_func,                            * cleanupFunc *
  NULL,                                         * lineChangeFunc *
  mark_segment_check_func                               * checkFunc *
};

const GtkTextLineSegmentClass gtk_text_left_mark_type = {
  "mark",                                               * name *
  TRUE,                                         * leftGravity *
  NULL,                                         * splitFunc *
  mark_segment_delete_func,                             * deleteFunc *
  mark_segment_cleanup_func,                            * cleanupFunc *
  NULL,                                         * lineChangeFunc *
  mark_segment_check_func                               * checkFunc *
};
*/

#endif
