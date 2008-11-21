/* GTK - The GIMP Toolkit
 * gtktextbtree.h Copyright (C) 2000 Red Hat, Inc.
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

#ifndef __TEXTBTREE_H__
#define __TEXTBTREE_H__

#if 0
#define DEBUG_VALIDATION_AND_SCROLLING
#endif

#ifdef DEBUG_VALIDATION_AND_SCROLLING
#define DV(x) (x)
#else
#define DV(x)
#endif

#include "TextChild.h"
#include "TextBuffer.h"
#include "TextTag.h"
#include "TextSegment.h"
#include "TextMark.h"
#include "TextIter.h"
#include "TextLayout.h"

#include <map>

/*
 * Types
 */

class TextBTree;
class TextChildAnchor;
class TextMark;
class TextLayout;
class TextLine;
class TextLineSegment;
class TextBuffer;
class TextIter;

//TODO move struct to the end of the file

/*
 * The structure below is used to pass information between
 * _gtk_text_btree_get_tags and inc_count:
 */

typedef struct TagInfo {
  int numTags;                  /* Number of tags for which there
                                 * is currently information in
                                 * tags and counts. */
  int arraySize;                        /* Number of entries allocated for
                                         * tags and counts. */
  TextTag **tags;           /* Array of tags seen so far.
                                * Malloc-ed. */
  int *counts;                  /* Toggle count (so far) for each
                                 * entry in tags.  Malloc-ed. */
} TagInfo;



typedef struct 
{
  gint remaining_pixels;
  bool in_validation;
  gint y;
  gint old_height;
  gint new_height;
} ValidateState;



/*
 * This is used to store per-view width/height info at the tree nodes.
 */

typedef struct _NodeData NodeData;

struct _NodeData {
  gpointer view_id;
  NodeData *next;

  /* Height and width of this node */
  gint height;
  signed int width : 24;

  /* boolean indicating whether the lines below this node are in need of validation.
   * However, width/height should always represent the current total width and
   * max height for lines below this node; the valid flag indicates whether the
   * width/height on the lines needs recomputing, not whether the totals
   * need recomputing.
   */
  guint valid : 8;		/* Actually a boolean */
};


/*
 * The data structure below keeps summary information about one tag as part
 * of the tag information in a node.
 */

class TextTagInfo;

typedef struct Summary {
  TextTagInfo *info;                     /* Handle for tag. */
  int toggle_count;                     /* Number of transitions into or
                                         * out of this tag that occur in
                                         * the subtree rooted at this node. */
  struct Summary *next;         /* Next in list of all tags for same
                                 * node, or NULL if at end of list. */
} Summary;



/*
 * Used to store the list of views in our btree
 */

class TextLayout;

class BTreeView {
	public:
		gpointer view_id;
		TextLayout *layout;
		BTreeView *next;
		BTreeView *prev;
	protected:
	private:
};

/*
 * And the tree itself
 */

	/*
	 * The data structure below defines a node in the B-tree.
	 */

	class TextBTreeNode {
		public:
			TextBTreeNode();
			~TextBTreeNode();

			void invalidate_downward  (void);
			void invalidate_upward    (
											       gpointer          view_id);
			NodeData* check_valid          (
											       gpointer          view_id);
			NodeData* check_valid_downward (
											       gpointer          view_id);
			static void check_valid_upward (TextBTreeNode *node, gpointer view_id);

			void remove_view (BTreeView *view, gpointer view_id);
			static void node_destroy (TextBTree *tree, TextBTreeNode *node);
			static void free_empty ( TextBTreeNode *node);
			NodeData* ensure_data         ( gpointer          view_id);
			void get_size ( gpointer          view_id,
											      gint             *width,
											      gint             *height);
			//TODO move to treenode class?
			static TextBTreeNode *    common_parent       (TextBTreeNode *node1,
							      TextBTreeNode *node2);

			TextBTreeNode *parent;         /* Pointer to parent node, or NULL if
							* this is the root. */
			TextBTreeNode *next;           /* Next in list of siblings with the
							* same parent node, or NULL for end
							* of list. */
			Summary *summary;	       /* First in malloc-ed list of info
							* about tags in this subtree (NULL if
							* no tag info in the subtree). */
			int level;                     /* Level of this node in the B-tree.
							* 0 refers to the bottom of the tree
							* (children are lines, not nodes). */
			union {                        /* First in linked list of children. */
				struct TextBTreeNode *node;	/* Used if level > 0. */
				TextLine *line;			/* Used if level == 0. */
			} children;

			int num_children;              /* Number of children of this node. */
			int num_lines;                 /* Total number of lines (leaves) in
							* the subtree rooted here. */
			int num_chars;                 /* Number of chars below here */

			NodeData *node_data;

void             remove_data ( gpointer          view_id);

			//TODO moved here from textbtree
bool          has_tag             ( TextTag       *tag);

		TextLine* prev_line_under_node ( TextLine      *line);

		void validate (BTreeView         *view,
		      gpointer           view_id,
		      ValidateState     *state);

		void compute_view_aggregates (
				     gpointer          view_id,
				     gint             *width_out,
				     gint             *height_out,
				     bool         *valid_out);


		 void recompute_level_zero_counts (void);
		 void recompute_level_nonzero_counts (void);

	 void check_consistency (TextBTree *tree);

	void adjust_toggle_count ( TextTagInfo *info, gint adjust);

		void change_node_toggle_count  (
                                     TextTagInfo     *info,
                                     gint                delta);

		protected:

		private:
			TextBTreeNode(const TextBTreeNode&);

			TextBTreeNode& operator=(const TextBTreeNode&);
	};



class TextBTree
{
	public:


		TextLine* find_line_by_y (
				BTreeView *view,
				TextBTreeNode *node,
				gint y, gint *line_top,
				TextLine *last_line);

		TextLine * find_line_by_y ( gpointer view_id, gint ypixel, gint *line_top_out);

		TextBTree(TextTagTable *table, TextBuffer *buffer);

		/* Indexable segment mutation */

		static void delete_text(TextIter *start, TextIter *end);
		static void insert_text(TextIter *iter, const gchar *text, gint len);
		/*void _gtk_text_btree_insert_pixbuf (TextIter *iter,
                                    GdkPixbuf   *pixbuf);*/

		//TODO cim doesnt support child widgets for now
		//static void insert_child_anchor (TextIter        *iter,
                //                          TextChildAnchor *anchor);

		static void unregister_child_anchor (TextChildAnchor *anchor);

		/* View stuff */

		gint         find_line_top     (
								TextLine       *line,
								gpointer           view_id);
		void         add_view          (
								TextLayout     *layout);
		void         remove_view       (
								gpointer           view_id);
		void         invalidate_region ( const TextIter *start, const TextIter *end, bool cursors_only);
		void         get_view_size     (
								gpointer           view_id,
								gint              *width,
								gint              *height);
		bool     is_valid          (
								gpointer           view_id);
		bool     validate          (
								gpointer           view_id,
								gint               max_pixels,
								gint              *y,
								gint              *old_height,
								gint              *new_height);
		void         validate_line     (
								TextLine       *line,
								gpointer           view_id);

		/* Tag */

		static void tag (TextIter *start,
					  TextIter *end,
					  TextTag        *tag,
					  bool           apply);

		/* "Getters" */

		TextLine * get_line          (
								 gint               line_number,
								 gint              *real_line_number);
		TextLine * line_no_last  (
								 gint               line_number,
								 gint              *real_line_number);
		TextLine * get_end_iter_line (void);
		TextLine * get_line_at_char  (
								 gint               char_index,
								 gint              *line_start_index,
								 gint              *real_char_index);
		static TextTag**  get_tags          (TextIter *iter, gint              *num_tags);
		static gchar        *get_text          (TextIter *start,
								 TextIter *end,
								 bool           include_hidden,
								 bool           include_nonchars);
		gint          get_line_count        (void);
		gint          get_char_count        (void);
		//TODO move to textiter?
		static bool      char_is_invisible (TextIter *iter);


		//TODO move to the Summary class?
		static void summary_destroy       (Summary          *summary);


		/* Get iterators (these are implemented in gtktextiter.c) */
		void     get_iter_at_char         (
								   TextIter        *iter,
								   gint                char_index);
		void     get_iter_at_line_char    (
								   TextIter        *iter,
								   gint                line_number,
								   gint                char_index);
		void     get_iter_at_line_byte    (
								   TextIter        *iter,
								   gint                line_number,
								   gint                byte_index);
		bool get_iter_from_string     (
								   TextIter        *iter,
								   const gchar        *string);
		bool get_iter_at_mark_name ( TextIter        *iter, const gchar        *mark_name);
		void get_iter_at_mark         ( TextIter        *iter, TextMark        *mark);
		void get_end_iter             (
								   TextIter        *iter);
		void get_iter_at_line         (
								   TextIter        *iter,
								   TextLine        *line,
								   gint                byte_offset);
		bool get_iter_at_first_toggle (
								   TextIter        *iter,
								   TextTag         *tag);
		bool get_iter_at_last_toggle  (
								   TextIter        *iter,
								   TextTag         *tag);

		void get_iter_at_child_anchor  (
								    TextIter        *iter,
								    TextChildAnchor *anchor);



		/* Manipulate marks */
		TextMark* set_mark (
									     TextMark         *existing_mark,
									     const gchar        *name,
									     bool            left_gravity,
									     TextIter  *index,
									     bool           should_exist);
		void                remove_mark_by_name     (
									     const gchar        *name);
		void                remove_mark             (
									     TextMark        *segment);
		bool            get_selection_bounds    (
									     TextIter        *start,
									     TextIter        *end);
		void                place_cursor            (
									     TextIter  *where);
		void                select_range            (
									     TextIter  *ins,
									     TextIter  *bound);
		bool            mark_is_insert          (
									     TextMark        *segment);
		bool            mark_is_selection_bound (
									     TextMark        *segment);
		TextMark        *get_insert		    (void);
		TextMark        *get_selection_bound	    (void);
		TextMark        *get_mark_by_name        (
									     const gchar        *name);
		TextLine *       first_could_contain_tag (
									     TextTag         *tag);
		TextLine *       last_could_contain_tag  (
									     TextTag         *tag);

		/* for coordination with the tag table */
		void notify_will_remove_tag (
                                             TextTag   *tag);

		/* Debug */
		void check (void);
		//void spew (void);

	TextTagInfo *get_existing_tag_info ( TextTag     *tag);

	//TODO move to textline
	void             text_line_destroy           ( TextLine      *line);


		 void node_view_check_consistency ( TextBTreeNode *node, NodeData         *nd);

		guint get_segments_changed_stamp (void);
		guint get_chars_changed_stamp (void);

		TextBuffer* get_buffer (void);

		bool is_end (TextLine *line, TextLineSegment *seg,
                                 int byte_index,
                                 int char_offset);

		TextLine* get_line_no_last (
						  gint               line_number,
						  gint              *real_line_number);
		
		bool line_is_last (TextLine *line);

		/* for gtktextmark.c */
		void release_mark_segment ( TextLineSegment *segment);

		void             segments_changed                (void);

		void ensure_end_iter_line (void);

		//TODO add getter/setter? (textline class wants to know this sometimes)
	  TextLine *end_iter_line;

		//TODO move these to a new nodedata class?
		static NodeData         *node_data_new          (gpointer  view_id);
		static void              node_data_destroy      (NodeData *nd);
		static void              node_data_list_destroy (NodeData *nd);
		static NodeData         *node_data_find         (NodeData *nd,
								 gpointer  view_id);

	protected:
	private:
		void ref(void);
		void unref(void);

		TextBuffer *GetBuffer(TextBTree *tree);

		guint GetCharsChangedStamp(void);
		guint GetSegmentsChangedStamp(void);
		void  SegmentsChanged(void);


  TextBTreeNode *root_node;          /* Pointer to root of B-tree. */
  TextTagTable *table; //TODO does this need to be a pointer?
  GHashTable *mark_table;
  guint refcount;
  TextMark *insert_mark;
  TextMark *selection_bound_mark;
  TextBuffer *buffer;
  BTreeView *views;
  GSList *tag_infos;
  gulong tag_changed_handler;

  /* Incremented when a segment with a byte size > 0
   * is added to or removed from the tree (i.e. the
   * length of a line may have changed, and lines may
   * have been added or removed). This invalidates
   * all outstanding iterators.
   */
  guint chars_changed_stamp;
  /* Incremented when any segments are added or deleted;
   * this makes outstanding iterators recalculate their
   * pointed-to segment and segment offset.
   */
  guint segments_changed_stamp;

  /* Cache the last line in the buffer */
  TextLine *last_line;
  guint last_line_stamp;

  /* Cache the next-to-last line in the buffer,
   * containing the end iterator
   */
  TextLineSegment *end_iter_segment;
  int end_iter_segment_byte_index;
  int end_iter_segment_char_offset;
  guint end_iter_line_stamp;
  guint end_iter_segment_stamp;
  
  GHashTable *child_anchor_table;

/*
 * Upper and lower bounds on how many children a node may have:
 * rebalance when either of these limits is exceeded.  MAX_CHILDREN
 * should be twice MIN_CHILDREN and MIN_CHILDREN must be >= 2.
 */

/* Tk used MAX of 12 and MIN of 6. This makes the tree wide and
   shallow. It appears to be faster to locate a particular line number
   if the tree is narrow and deep, since it is more finely sorted.  I
   guess this may increase memory use though, and make it slower to
   walk the tree in order, or locate a particular byte index (which
   is done by walking the tree in order).

   There's basically a tradeoff here. However I'm thinking we want to
   add pixels, byte counts, and char counts to the tree nodes,
   at that point narrow and deep should speed up all operations,
   not just the line number searches.
*/

	BTreeView        *get_view                 (
									  gpointer          view_id);
	void              rebalance                (
									  TextBTreeNode *node);
	TextLine     * get_last_line                           (void);
	void              post_insert_fixup                       (
									  TextLine      *insert_line,
									  gint              char_count_delta,
									  gint              line_count_delta);

	void             chars_changed                   (void);
	void             summary_list_destroy            (Summary          *summary);


	void get_tree_bounds       (
					   TextIter      *start,
					   TextIter      *end);
	void tag_changed_cb        (TextTagTable  *table,
					   TextTag       *tag,
					   bool          size_changed,
					   TextBTree     *tree);
	void recompute_node_counts (
					   TextBTreeNode *node);

	//TODO move to texttag class?
	static void inc_count             (TextTag       *tag,
					   int               inc,
					   TagInfo          *tagInfoPtr);

	//TODO to textlinesegment class
	static void link_segment   (TextLineSegment *seg,
						   TextIter  *iter);
	void unlink_segment (
						   TextLineSegment *seg,
						   TextLine        *line);


	TextTagInfo *get_tag_info          (
								     TextTag     *tag);
	void            remove_tag_info       (
								     TextTag     *tag);

	void redisplay_region (
				      TextIter *start,
				      TextIter *end,
				      bool           cursors_only);


		void resolve_bidi (TextIter *start, TextIter *end);

		void queue_tag_redisplay (TextTag *tag, TextIter *start, TextIter *end);


		TextLine* get_line_internal (
			   gint          line_number,
			   gint         *real_line_number,
			   bool      include_last);


		gint find_line_top_in_line_list (
					    BTreeView *view,
					    TextLine *line,
					    TextLine *target_line,
					    gint y);


		//TODO move to textmark class
		void redisplay_mark (TextLineSegment *mark);
		void redisplay_mark_if_visible (TextLineSegment *mark);

		TextLineSegment* real_set_mark (
		       TextMark       *existing_mark,
		       const gchar       *name,
		       bool           left_gravity,
		       TextIter *where,
		       bool           should_exist,
		       bool           redraw_selections);


		void ensure_not_off_end (
                    TextLineSegment *mark,
                    TextIter *iter);

		//TODO for the textmark class
		void mark_set_visible (TextMark       *mark,
                           bool           setting);


		void ensure_end_iter_segment (void);
	//TODO was moved to textline class	bool text_line_contains_end_iter (TextLine  *line);

};

/* Lines */

/* Chunk of data associated with a line; views can use this to store
   info at the line. They should "subclass" the header struct here. */

class TextLineData
{
	public:
		TextLineData(TextLayout *layout, TextLine *line);
		~TextLineData();

		gpointer view_id;
		TextLineData *next;
		gint height;
		signed int width : 24;
		guint valid : 8;		/* Actually a boolean */

	protected:

	private:
  		TextLineData();
		TextLineData(const TextLineData&);

		TextLineData& operator=(const TextLineData&);
};

/*
 * The data structure below defines a single line of text (from newline
 * to newline, not necessarily what appears on one line of the screen).
 *
 * You can consider this line a "paragraph" also
 */

class TextLine
{
	public:
	TextLine();

  TextBTreeNode *parent;             /* Pointer to parent node containing
                                         * line. */
  TextLine *next;            /* Next in linked list of lines with
                                 * same parent node in B-tree.  NULL
                                 * means end of list. */
  TextLineSegment *segments; /* First in ordered list of segments
                                 * that make up the line. */
  TextLineData *views;      /* data stored here by views */
  guchar dir_strong;                /* BiDi algo dir of line */
  guchar dir_propagated_back;       /* BiDi algo dir of next line */
  guchar dir_propagated_forward;    /* BiDi algo dir of prev line */


gint            get_number                 (void);
bool            char_has_tag               (
                                                               TextBTree        *tree,
                                                               gint                 char_in_line,
                                                               TextTag          *tag);
bool            has_tag               (
                                                               TextBTree        *tree,
                                                               gint                 byte_in_line,
                                                               TextTag          *tag);
//bool            is_last                    (
//                                                               TextBTree        *tree);
bool            contains_end_iter          ( TextBTree        *tree);
TextLine *       next_line                       (void);
TextLine *       next_line_excluding_last        (void);
TextLine *       previous_line                   (void);
void                add_data                   (
                                                               TextLineData     *data);
TextLineData* remove_data                ( gpointer             view_id);
TextLineData* get_data                   ( gpointer             view_id);
void                invalidate_wrap            (
                                                               TextLineData     *ld);
gint                char_count                 (void);
gint                byte_count                 (void);
gint                char_index                 (void);
TextLineSegment *_gtk_text_line_byte_to_segment            (TextLine         *line,
                                                               gint                 byte_offset,
                                                               gint                *seg_offset);
TextLineSegment *_gtk_text_line_char_to_segment            (TextLine         *line,
                                                               gint                 char_offset,
                                                               gint                *seg_offset);
bool            byte_locate                (
                                                               gint                 byte_offset,
                                                               TextLineSegment **segment,
                                                               TextLineSegment **any_segment,
                                                               gint                *seg_byte_offset,
                                                               gint                *line_byte_offset);
bool            char_locate                (
                                                               gint                 char_offset,
                                                               TextLineSegment **segment,
                                                               TextLineSegment **any_segment,
                                                               gint                *seg_char_offset,
                                                               gint                *line_char_offset);
void                byte_to_char_offsets       (
                                                               gint                 byte_offset,
                                                               gint                *line_char_offset,
                                                               gint                *seg_char_offset);
void                char_to_byte_offsets       (
                                                               gint                 char_offset,
                                                               gint                *line_byte_offset,
                                                               gint                *seg_byte_offset);
TextLineSegment *_gtk_text_line_byte_to_any_segment        (TextLine         *line,
                                                               gint                 byte_offset,
                                                               gint                *seg_offset);
TextLineSegment *_gtk_text_line_char_to_any_segment        (TextLine         *line,
                                                               gint                 char_offset,
                                                               gint                *seg_offset);
gint                _gtk_text_line_byte_to_char               (TextLine         *line,
                                                               gint                 byte_offset);
gint                _gtk_text_line_char_to_byte               (TextLine         *line,
                                                               gint                 char_offset);
TextLine    *    next_could_contain_tag     (
                                                               TextBTree        *tree,
                                                               TextTag          *tag);
TextLine    *    previous_could_contain_tag (
                                                               TextBTree        *tree,
                                                               TextTag          *tag);

	void             set_parent        (
								 TextBTreeNode *node);
	void cleanup_line          ();

		bool find_toggle_outside_current_line (TextBTree *tree, TextTag *tag);
		TextLineSegment* find_toggle_segment_before_byte ( gint byte_in_line, TextTag *tag);
		TextLineSegment* find_toggle_segment_before_char ( gint char_in_line, TextTag *tag);
		TextLine* next_excluding_last (void);

		bool byte_has_tag ( TextBTree *tree, gint byte_in_line, TextTag *tag);


		TextLineSegment* byte_to_segment ( gint byte_offset, gint *seg_offset);
		TextLineSegment* char_to_segment (
                               gint char_offset,
                               gint *seg_offset);
		TextLineSegment* byte_to_any_segment (
                                   gint byte_offset,
                                   gint *seg_offset);

	protected:
	private:
};



//extern bool _gtk_text_view_debug_btree;

/* ignore, exported only for gtktextsegment.c */
void _gtk_toggle_segment_check_func (TextLineSegment *segPtr,
                                     TextLine        *line);


#endif /* __TEXTBTREE_H__ */
