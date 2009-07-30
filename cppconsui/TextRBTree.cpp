/*
 * Copyright (C) 2009 by Mark Pustjens <pustjens@dds.nl>
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

/* The red-black tree implementation used here is based on the one
 * from the book `Introduction to Algorithms, second edition' by
 * Thomas H. Cormen et. al., ISBN 0-262-03296-7.
 *
 * There are some small differences due to implementation details.
 * */

#include "config.h"

#include "TextRBTree.h"

#include <cassert>

TextRBTree::TextRBTree()
{
	nil = MakeNil();
	root = nil;
}

TextRBTree::~TextRBTree()
{
	if (root != nil)
		delete root; /* Deletes nodes recursively. */

	delete nil;
}

TextRBTree::Node* TextRBTree::MakeNil(void)
{
	Node* nil = new Node(this, NULL);

	/* Set all pointers of the nil node to nil. The nil node is fake. */
	nil->parent = nil;
	nil->left = nil;
	nil->right = nil;
	nil->pred = nil;
	nil->succ = nil;

	/* Line is empty by default. */

	/* Set all order statistic data of nil to 0 (or the unit element.) */
	nil->bytes = 0;
	nil->chars = 0;
	nil->cols = 0;

	return nil;
}

TextRBTree::Node::Node(TextRBTree *tree, Node *parent)
: tree(tree)
, color(BLACK)
, parent(parent)
, left(NULL)
, right(NULL)
, pred(NULL)
, succ(NULL)
, bytes(0)
, chars(0)
, cols(0)
, lines(0)
, line()
{
}

/*TODO remove this constructor?*/
TextRBTree::Node::Node(TextRBTree *tree, Node *parent, const TextLine &line)
: tree(tree)
, color(BLACK)
, parent(parent)
, left(NULL)
, right(NULL)
, pred(NULL)
, succ(NULL)
, bytes(0)
, chars(0)
, cols(0)
, lines(0)
, line(line)
{
}

TextRBTree::Node::~Node()
{
}

TextRBTree::char_iterator TextRBTree::insert(const char_iterator& _iter, const char* text, int len)
{
	/* Algorithm taken from gtktextrbtree */

	int chunk_len; /* number of characters in current chunk. */
	int sol; /* start of line */
	int eol; /* index of character just after last one in current chunk. */
	int delim; /* index of paragraph delimiter */
	char_iterator line_iter(*this);
	char_iterator iter(_iter);
	line_iterator tmp;

	eol = 0;
	sol = 0;
	line_iter = iter;

	while (eol < len)
	{
		sol = eol;

		find_paragraph_boundary (text + sol, len - sol, &delim, &eol);

		/* make these relative to the start of the text */
		delim += sol;
		eol += sol;

		assert (eol >= sol);
		assert (delim >= sol);
		assert (eol >= delim);
		assert (sol >= 0);
		assert (eol <= len);

		chunk_len = eol - sol;

		//TODO assert (g_utf8_validate ((const gchar*)text[sol], chunk_len, NULL));

		/* insert the paragraph in the current line just after the cursor position. */
		line_iter->insert(line_iter.byte_offset, &text[sol], chunk_len);

		/* We modify an existing node, so we must fix the augmented data after insertion. */
		post_insert_augmentation_fixup(line_iter.node);

		/* advance the char_iterator by chunk_len bytes */
		iter.forward_bytes(chunk_len);

		if (delim == eol)
		{
			/* chunk didn't end with a paragraph separator */
			assert (eol == len);
			break;
		}

		/*
		 * The chunk ended with a newline, so create a new TextLine
		 * and move the remainder of the old line to it.
		 */
		//tmp = iter;
		//line = new TextLine(
		//		*tmp,
		//		iter.char_offset - 1,
		//		iter->chars());
		//line_iter->erase(iter.char_offset, iter->chars());
		split_node(iter.node, iter.char_offset - 1);

		/* insert the new line after the current line. The returned
		 * char_iterator should point to the beginning of the new line.
		 */
		line_iter.forward_bytes(chunk_len); /* line_iter is still at the origional insert point, so we must move it. */

		//if (line->chars() > 0)
		//	iter = insert(line_iter, *line);
		//else
		//	iter = line_iter;
	}

	return iter;

}

TextRBTree::char_iterator TextRBTree::insert(const char_iterator& iter, const TextLine &line)
{
	return insert(new Node(this, NULL, line), iter);
}

void TextRBTree::append(const TextLine& line)
{
	Node *z, *y;

	/* Append line to the back of the buffer.
	 * This means it must be a right child of the tree maximum. */

	/* add z as child of y. */
	z = new Node(this, NULL, line);

	y = tree_maximum(root);

	if (y == nil) /* Tree is emtpy. */
	{
		root = z;
		z->parent = nil;
	}
	else
	{
		y->right = z;
		z->parent = y;
	}

	z->left = nil;
	z->right = nil;
	z->color = RED;

	/* Also fix predecessor and successor pointers. */
	z->pred = y;
	z->succ = nil;
	if (z->pred != nil) z->pred->succ = z;
	if (z->succ != nil) z->succ->pred = z;

	post_insert_augmentation_fixup(z);

	insert_fixup(z);
}

void TextRBTree::prefix(const TextLine& line)
{
	Node *z, *y;

	/* Prefix line to the start of the buffer.
	 * This means it must be a left child of the tree minimum. */

	/* add z as child of y. */
	z = new Node(this, NULL, line);

	y = tree_minimum(root);

	if (y == nil) /* Tree is emtpy. */
	{
		root = z;
		z->parent = nil;
	}
	else
	{
		y->left = z;
		z->parent = y;
	}

	z->left = nil;
	z->right = nil;
	z->color = RED;

	/* Also fix predecessor and successor pointers. */
	z->pred = nil;
	z->succ = y;
	if (z->pred != nil) z->pred->succ = z;
	if (z->succ != nil) z->succ->pred = z;

	post_insert_augmentation_fixup(z);

	insert_fixup(z);
}

TextRBTree::char_iterator TextRBTree::begin(void) const
{
	return char_iterator(*tree_minimum(root));
}

TextRBTree::char_iterator TextRBTree::back(void) const
{
	TextRBTree::char_iterator iter(*tree_maximum(root));
	iter.byte_offset = iter.node->line.bytes() - iter.node->left->bytes - iter.node->right->bytes;
	iter.char_offset = iter.node->line.chars() - iter.node->left->chars - iter.node->right->chars;
	iter.col_offset = iter.node->line.columns() - iter.node->left->cols - iter.node->right->cols;
	iter.line_offset = iter.node->line.lines() - iter.node->left->lines - iter.node->right->lines;
	return iter;
}

TextRBTree::char_iterator TextRBTree::end(void) const
{
	return char_iterator(*nil);
}

TextRBTree::char_iterator TextRBTree::reverse_begin(void) const
{
	return back();
}

TextRBTree::char_iterator TextRBTree::reverse_back(void) const
{
	return begin();
}

TextRBTree::char_iterator TextRBTree::reverse_end(void) const
{
	return char_iterator(*nil);
}

/* Split node z after the given charater offset. */
void TextRBTree::split_node(Node *node, unsigned int offset)
{
	TextLine* line;

	/* Make sure we do not split a line at the end or beginning. */
	if ( (offset == 0) || (offset >= node->line.chars()-1) )
		return;

	assert( *node->line.get_pointer_at_char_offset(offset) == '\n');

	/* Make sure we split at a valid location. */
	if ( *node->line.get_pointer_at_char_offset(offset) != '\n')
		return;

	/* Create a new line containing everything after the newline character. */
	line = new TextLine(
			node->line,
			offset,
			node->line.chars());

	/* Remove everything after the newline character. */
	node->line.erase(offset + 1, node->line.chars());

	/* Insert the line. */
	insert(new Node(this, NULL, *line), node->succ);
}

void TextRBTree::rotate_left(Node *x)
{
	Node *y = NULL;

	assert(x->tree == this);

	/* NOTE: rotations do not destroy the order, so predecessor
	 * and successor pointers remain the same.
	 */

	y = x->right; /* Set y */

	x->right = y->left; /* Turn y's left subtree into x's right subtree */

	if (y->left != nil)
		y->left->parent = x;

	y->parent = x->parent; /* Link x's parent to y */

	if (x->parent == nil)
		root = y;
	else if (x == x->parent->left)
		x->parent->left = y;
	else
		x->parent->right = y;

	y->left = x; /* Put x on y's left */
	x->parent = y;

	post_rotate_augmentation_fixup(x, y); /* Update augmented data */
}

void TextRBTree::rotate_right(Node *x)
{
	Node *y = NULL;

	assert(x->tree == this);

	y = x->left; /* Set y */

	x->left = y->right; /* Turn y's right subtree into x's left subtree */

	if (y->right != nil)
		y->right->parent = x;

	y->parent = x->parent; /* Link x's parent to y */

	if (x->parent == nil)
		root = y;
	else if (x == x->parent->right)
		x->parent->right = y;
	else
		x->parent->left = y;

	y->right = x; /* Put x on y's right */
	x->parent = y;

	post_rotate_augmentation_fixup(x, y); /* Update augmented data */
}

void TextRBTree::post_rotate_augmentation_fixup(Node *x, Node *y)
{
	/* Fix the order statistics data. Remember that x is now a
	 * child of y so x needs to be recalculated first. */

	x->bytes = x->line.bytes() + x->left->bytes + x->right->bytes;
	x->chars = x->line.chars() + x->left->chars + x->right->chars;
	x->cols = x->line.columns() + x->left->cols + x->right->cols;
	x->lines = x->line.lines() + x->left->lines + x->right->lines;

	y->bytes = y->line.bytes() + y->left->bytes + y->right->bytes;
	y->chars = y->line.chars() + y->left->chars + y->right->chars;
	y->cols = y->line.columns() + y->left->cols + y->right->cols;
	y->lines = y->line.lines() + y->left->lines + y->right->lines;
}

void TextRBTree::post_insert_augmentation_fixup(Node *z)
{
	Node *x = NULL;

	/* Fix the order statistics data. Remember that z was
	 * added as a child, so the data of all ancestors
	 * must chage.
	 */

	z->bytes = z->line.bytes();
	z->chars = z->line.chars();
	z->cols = z->line.columns();
	z->lines = z->line.lines();

	/* Increment the order statistic data for all ancestors. */
	for (x = z->parent; x != nil; x = x->parent)
	{
		x->bytes += z->bytes;
		x->chars += z->chars;
		x->cols += z->cols;
		x->lines += z->lines;
	}
}

void TextRBTree::insert_fixup(Node *z)
{
	Node *y = NULL;

	while (z->parent->color == RED)
	{
		if (z->parent == z->parent->parent->left)
		{
			y = z->parent->parent->right;
			if (y->color == RED)
			{
				z->parent->color = BLACK;
				y->color = BLACK;
				z->parent->parent->color = RED;
				z = z->parent->parent;
			}
			else
			{
				if (z == z->parent->right)
				{
					z = z->parent;
					rotate_left(z);
				}

				z->parent->color = BLACK;
				z->parent->parent->color = RED;
				rotate_right(z->parent->parent);
			}
		}
		else
		{
			y = z->parent->parent->left;
			if (y->color == RED)
			{
				z->parent->color = BLACK;
				y->color = BLACK;
				z->parent->parent->color = RED;
				z = z->parent->parent;
			}
			else
			{
				if (z == z->parent->left)
				{
					z = z->parent;
					rotate_right(z);
				}

				z->parent->color = BLACK;
				z->parent->parent->color = RED;
				rotate_left(z->parent->parent);
			}
		}
	}
	
	root->color = BLACK;
}

void TextRBTree::post_erase_augmentation_fixup(Node *z)
{
	Node *x = NULL;

	/* Fix the order statistics data. Remember that z is the parent
	 * of the node which was erased. */

	/* Update the order statistic data for all ancestors. */
	for (x = z; x != nil; x = x->parent)
	{
		x->bytes = x->line.bytes() + x->left->bytes + x->right->bytes;
		x->chars = x->line.chars() + x->left->chars + x->right->chars;
		x->cols = x->line.columns() + x->left->cols + x->right->cols;
		x->lines = x->line.lines() + x->left->lines + x->right->lines;
	}
}

void TextRBTree::erase_node_fixup(Node *x)
{
	Node *w = NULL;

	while (x != root && x->color == BLACK)
	{
		if (x == x->parent->left)
		{
			w = x->parent->right;

			if (w->color == RED)
			{
				w->color = BLACK;
				x->parent->color = RED;
				rotate_left(x->parent);
				w = x->parent->right;
			}

			if (w->left->color == BLACK && w->right->color == BLACK)
			{
				w->color = RED;
				x = x->parent;
			}

			else
			{
				if (w->right->color == BLACK)
				{
					w->left->color = BLACK;
					w->color = RED;
					rotate_right(w);
					w = x->parent->right;
				}

				w->color = x->parent->color;
				x->parent->color = BLACK;
				w->right->color = BLACK;
				rotate_left(x->parent);
				x = root;
			}
		}
		else
		{
			w = x->parent->left;
			if (w->color == RED)
			{
				w->color = BLACK;
				x->parent->color = RED;
				rotate_right(x->parent);
				w = x->parent->left;
			}
			if (w->right->color == BLACK && w->left->color == BLACK)
			{
				w->color = RED;
				x = x->parent;
			}
			else
			{
				if (w->left->color == BLACK)
				{
					w->right->color = BLACK;
					w->color = RED;
					rotate_left(w);
					w = x->parent->left;
				}

				w->color = x->parent->color;
				x->parent->color = BLACK;
				w->left->color = BLACK;
				rotate_right(x->parent);
				x = root;
			}
		}
	}

	x->color = BLACK;
}

TextRBTree::Node* TextRBTree::tree_minimum(Node *node) const
{
	Node *min = node;

	assert(node->tree == this);

	while (min->left != nil)
		min = min->left;

	return min;
}

TextRBTree::Node* TextRBTree::tree_maximum(Node *node) const
{
	Node *max = node;

	assert(node->tree == this);

	while (max->right != nil)
		max = max->right;

	return max;
}

TextRBTree::Node* TextRBTree::successor(Node *node) const
{
	Node *x = node;
	Node *y = NULL;
	
	assert(node->tree == this);

	if (x->right != nil)
		return tree_minimum(x);

	y = x->parent;

	while (y != nil && x == y->right)
	{
		x = y;
		y = y->parent;
	}

	return y;
}

TextRBTree::Node* TextRBTree::predecessor(Node *node) const
{
	Node *x = node;
	Node *y = NULL;
	
	assert(node->tree == this);

	if (x->left != nil)
		return tree_maximum(x);

	y = x->parent;

	while (y != nil && x == y->left)
	{
		x = y;
		y = y->parent;
	}

	return y;
}

TextRBTree::char_iterator TextRBTree::insert(Node *z, const iterator_base iter)
{
	return insert(z, iter.node);
}

TextRBTree::char_iterator TextRBTree::insert(Node *z, Node *y)
{
	Node *x;
	/* Insert z into the tree before y.
	 * Returns an iterator to the inserted node.
	 *
	 * If the tree is emtpy, z becomes the root.
	 * If iter is end() (and thus points to the nil node), append the line.
	 * If y has a left subtree, insert as y->pred->right.
	 * If y has no left subtree, insert as y->left.
	 * */

	/* Find the insert point for a node inserted before node y */

	if (y == nil)
	{
		if (root == nil) /* the tree is empty. */
		{
			root = z;
			z->parent = nil;
		}
		else /* iter is the end() iter, so append. */
		{
			x = tree_maximum(root);
			x->right = z;
			z->parent = x;
		}
	}
	else if (y->left != nil) /* y has a left subtree. */
	{
		assert(tree_maximum(y->left) == y->pred);
		y->pred->right = z;
		z->parent = y->pred;
	}
	else  /* y has no left subtree. */
	{
		y->left = z;
		z->parent = y;
	}

	z->left = nil;
	z->right = nil;
	z->color = RED;

	/* Also fix predecessor and successor pointers. */
	z->pred = predecessor(z);
	z->succ = successor(z);
	if (z->pred != nil) z->pred->succ = z;
	if (z->succ != nil) z->succ->pred = z;

	post_insert_augmentation_fixup(z);

	insert_fixup(z);

	/* Return a iterator to the inserted node. */
	return TextRBTree::char_iterator(*z);
}


TextRBTree::char_iterator TextRBTree::insert(Node *z, int line_nr)
{
	Node *y = NULL;
	Node *x = root;
	int line = 0;

	assert(z->tree == this);

	/* Find the insertion point. That is: find the parent
	 * for the new node.
	 *
	 * Here we find the node with line number line_nr
	 * and insert the new node before this node.
	 *
	 * If there are less than line_nr lines, the node is added
	 * as the last node.
	 */
	while (x != nil)
	{
		y = x;

		line = x->left->lines + 1;

		if (line_nr < line)
		{
			x = x->left;
		}
		else
		{
			x = x->right;
			line_nr -= line;
		}
	}

	/* add z as child of y. */
	z->parent = y;

	if (y == nil)
		root = z;
	else if (line_nr < line)
		y->left = z;
	else
		y->right = z;

	z->left = nil;
	z->right = nil;
	z->color = RED;

	/* Also fix predecessor and successor pointers. */
	z->pred = predecessor(z);
	z->succ = successor(z);
	if (z->pred != nil) z->pred->succ = z;
	if (z->succ != nil) z->succ->pred = z;

	post_insert_augmentation_fixup(z);

	insert_fixup(z);

	return TextRBTree::char_iterator(*z);
}

TextRBTree::iterator_base::iterator_base(void)
: tree(NULL)
, node(NULL)
, byte_offset(0)
, char_offset(0)
, col_offset(0)
, line_offset(0)
{
}

TextRBTree::iterator_base::iterator_base(TextRBTree &_tree)
: tree(&_tree)
, node(NULL)
, byte_offset(0)
, char_offset(0)
, col_offset(0)
, line_offset(0)
{
	node = _tree.tree_minimum(_tree.root);
}

TextRBTree::iterator_base::iterator_base(Node &node_)
: tree(node_.tree)
, node(&node_)
, byte_offset(0)
, char_offset(0)
, col_offset(0)
, line_offset(0)
{
}

TextRBTree::iterator_base::iterator_base(const iterator_base &iter)
{
	*this = iter;
}

bool TextRBTree::iterator_base::valid(void) const
{
	return tree != NULL;
}

bool TextRBTree::iterator_base::valid_char(void) const
{
	const char *chr = node->line.get_pointer_at_char_offset(char_offset);

	if (chr == NULL)
		return false;

	return  g_unichar_validate( g_utf8_get_char_validated(chr, -1) );
}

unsigned int TextRBTree::iterator_base::line_nr(void) const
{
	Node *y;
	unsigned int line;
       
	line = node->left->lines;
	y = node;

	while (y != tree->root) {
		if (y == y->parent->right) {
			line = line + y->parent->left->lines + 1;
		}
		
		y = y->parent;
	}

	return line;
}

/*
unsigned int TextRBTree::iterator_base::display_line_nr(void) const
{
	Node *y;
	unsigned int line;
       
	line = node->left->lines + line_offset;
	y = node;

	while (y != tree->root) {
		if (y == y->parent->right) {
			line = line + y->parent->left->lines + y->parent->line.display_lines;
		}
		
		y = y->parent;
	}

	return line;
}*/

unsigned int TextRBTree::iterator_base::char_bytes(void)
{
	const char *chr = node->line.get_pointer_at_char_offset(char_offset);
	return g_utf8_next_char(chr) - chr;
}

unsigned int TextRBTree::iterator_base::char_cols(void)
{
	const char *chr = node->line.get_pointer_at_char_offset(char_offset);
	gunichar c;

	c = g_utf8_get_char(chr);

	return (g_unichar_iswide(c) ? 2 : 1);
}

unsigned int TextRBTree::iterator_base::chars(void) const
{
	return node->chars;
}

unsigned int TextRBTree::iterator_base::bytes(void) const
{
	return node->bytes;
}

unsigned int TextRBTree::iterator_base::cols(void) const
{
	return node->cols;
}

TextLine& TextRBTree::iterator_base::operator*() const
{
	return node->line;
}

TextLine* TextRBTree::iterator_base::operator->() const
{
	return &(node->line);
}

TextRBTree::iterator_base& TextRBTree::iterator_base::operator=(const TextRBTree::iterator_base& iter)
{
	if (this == &iter) return *this;

	node = iter.node;
	tree = iter.tree;
	byte_offset = iter.byte_offset;
	char_offset = iter.char_offset;
	col_offset = iter.col_offset;
	line_offset = iter.line_offset;

	return *this;
}

bool TextRBTree::iterator_base::operator<(const iterator_base& iter)
{
	return (this->line_nr() < iter.line_nr()) || 
		((this->node == iter.node) && (this->byte_offset < iter.byte_offset));
}

TextRBTree::iterator_base& TextRBTree::iterator_base::forward_bytes(unsigned int n)
{
	assert(tree != NULL);

	if (node == tree->nil)
		return *this;

	while ( (node != tree->nil) && ((byte_offset + n) > (node->line.bytes() - 1)) ) {
		n -= node->line.bytes() - byte_offset;
		node = node->succ;
		byte_offset = 0;
	}

	if (node != tree->nil) {
		byte_offset += n;
		char_offset = node->line.byte_to_char_offset(byte_offset);
		col_offset = node->line.byte_to_col_offset(byte_offset);
	} else {
		byte_offset = 0;
		char_offset = 0;
		col_offset = 0;
		line_offset = 0;
	}

	return *this;
}

TextRBTree::iterator_base& TextRBTree::iterator_base::backward_bytes(unsigned int n)
{
	assert(tree != NULL);

	if (node == tree->nil)
		return *this;

	while ( (node != tree->nil) && (byte_offset < n) ) {
		n -= byte_offset;
		node = node->pred;
		byte_offset = node->line.bytes() - 1;
	}

	if (node != tree->nil) {
		byte_offset -= n;
		char_offset = node->line.byte_to_char_offset(byte_offset);
		col_offset = node->line.byte_to_col_offset(byte_offset);
	} else {
		byte_offset = 0;
		char_offset = 0;
		col_offset = 0;
		line_offset = 0;
	}

	return *this;
}

TextRBTree::iterator_base& TextRBTree::iterator_base::forward_chars(unsigned int n)
{
	assert(tree != NULL);

	if (node == tree->nil)
		return *this;

	while ( (node != tree->nil) && ((char_offset + n) > (node->line.chars() - 1)) ) {
		n -= node->line.chars() - char_offset;
		node = node->succ;
		char_offset = 0;
	}

	if (node != tree->nil) {
		char_offset += n;
		byte_offset = node->line.char_to_byte_offset(char_offset);
		col_offset = node->line.byte_to_col_offset(byte_offset);
	} else {
		byte_offset = 0;
		char_offset = 0;
		col_offset = 0;
		line_offset = 0;
	}

	return *this;
}

TextRBTree::iterator_base& TextRBTree::iterator_base::backward_chars(unsigned int n)
{
	assert(tree != NULL);

	if (node == tree->nil)
		return *this;

	while ( (node != tree->nil) && (char_offset < n) ) {
		n -= char_offset;
		node = node->pred;
		char_offset = node->line.chars() - 1;
	}

	if (node != tree->nil) {
		char_offset -= n;
		byte_offset = node->line.char_to_byte_offset(char_offset);
		col_offset = node->line.byte_to_col_offset(byte_offset);
	} else {
		byte_offset = 0;
		char_offset = 0;
		col_offset = 0;
		line_offset = 0;
	}

	return *this;
}

TextRBTree::iterator_base& TextRBTree::iterator_base::forward_cols(unsigned int n)
{
	assert(tree != NULL);

	if (node == tree->nil)
		return *this;

	while ( (node != tree->nil) && ((col_offset + n) > (node->line.columns() - 1)) ) {
		n -= node->line.columns() - col_offset;
		node = node->succ;
		col_offset = 0;
	}

	if (node != tree->nil) {
		col_offset += n;
		byte_offset = node->line.col_to_byte_offset(col_offset);
		char_offset = node->line.byte_to_char_offset(byte_offset);
	} else {
		byte_offset = 0;
		char_offset = 0;
		col_offset = 0;
		line_offset = 0;
	}

	return *this;
}

TextRBTree::iterator_base& TextRBTree::iterator_base::backward_cols(unsigned int n)
{
	assert(tree != NULL);

	if (node == tree->nil)
		return *this;

	while ( (node != tree->nil) && (col_offset < n) ) {
		n -= col_offset;
		node = node->pred;
		col_offset = node->line.columns() - 1;
	}

	if (node != tree->nil) {
		col_offset -= n;
		byte_offset = node->line.col_to_byte_offset(col_offset);
		char_offset = node->line.byte_to_char_offset(byte_offset);
	} else {
		byte_offset = 0;
		char_offset = 0;
		col_offset = 0;
		line_offset = 0;
	}

	return *this;
}

TextRBTree::iterator_base& TextRBTree::iterator_base::forward_lines(unsigned int n)
{
	assert(tree != NULL);

	if (node == tree->nil)
		return *this;

	while ( (node != tree->nil) && (n > 0) ) {
		n--;
		node = node->succ;
	}

	/* Always move to the start of the line. */
	byte_offset = 0;
	char_offset = 0;
	col_offset = 0;
	line_offset = 0;

	return *this;
}

TextRBTree::iterator_base& TextRBTree::iterator_base::backward_lines(unsigned int n)
{
	assert(tree != NULL);

	if (node == tree->nil)
		return *this;

	while ( (node != tree->nil) && (n > 0) ) {
		n--;
		node = node->pred;
	}

	if (node == tree->nil) {
		byte_offset = 0;
		char_offset = 0;
		col_offset = 0;
		line_offset = 0;
	}

	return *this;
}

TextRBTree::byte_iterator::byte_iterator(void)
: iterator_base()
{
}

TextRBTree::byte_iterator::byte_iterator(TextRBTree &tree)
: iterator_base(tree)
{
}

TextRBTree::byte_iterator::byte_iterator(Node &node)
: iterator_base(node)
{
}

TextRBTree::byte_iterator::byte_iterator(const iterator_base &iter)
: iterator_base(iter)
{
}

TextRBTree::byte_iterator& TextRBTree::byte_iterator::operator=(const TextRBTree::byte_iterator& iter)
{
	if (this == &iter) return *this;

	TextRBTree::iterator_base::operator=(iter);

	return *this;
}

TextRBTree::byte_iterator& TextRBTree::byte_iterator::operator=(const TextRBTree::iterator_base& iter)
{
	if (this == &iter) return *this;

	TextRBTree::iterator_base::operator=(iter);

	return *this;
}

bool TextRBTree::byte_iterator::operator==(const byte_iterator &other) const
{
	assert (tree != NULL);
	return (other.node == this->node) && (other.byte_offset == this->byte_offset);
}

bool TextRBTree::byte_iterator::operator!=(const byte_iterator &other) const
{
	assert (tree != NULL);
	return (other.node != this->node) || (other.byte_offset != this->byte_offset);
}

TextRBTree::byte_iterator& TextRBTree::byte_iterator::operator++()
{
	forward_bytes(1);
	return *this;
}

TextRBTree::byte_iterator& TextRBTree::byte_iterator::operator--()
{
	backward_bytes(1);
	return *this;
}

TextRBTree::byte_iterator TextRBTree::byte_iterator::operator++(int)
{
	byte_iterator copy = *this;
	++(*this);
	return copy;
}

TextRBTree::byte_iterator TextRBTree::byte_iterator::operator--(int)
{
	byte_iterator copy = *this;
	--(*this);
	return copy;
}

TextRBTree::byte_iterator& TextRBTree::byte_iterator::operator+=(unsigned int n)
{
	forward_bytes(n);
	return *this;
}

TextRBTree::byte_iterator& TextRBTree::byte_iterator::operator-=(unsigned int n)
{
	backward_bytes(n);
	return *this;
}

TextRBTree::char_iterator::char_iterator(void)
: iterator_base()
{
}

TextRBTree::char_iterator::char_iterator(TextRBTree &tree)
: iterator_base(tree)
{
}

TextRBTree::char_iterator::char_iterator(Node &node)
: iterator_base(node)
{
}

TextRBTree::char_iterator::char_iterator(const iterator_base &iter)
: iterator_base(iter)
{
}

char* TextRBTree::char_iterator::operator*() const
{
	return node->line.get_pointer_at_char_offset(char_offset);
}


TextRBTree::char_iterator& TextRBTree::char_iterator::operator=(const TextRBTree::char_iterator& iter)
{
	if (this == &iter) return *this;

	TextRBTree::iterator_base::operator=(iter);

	return *this;
}

TextRBTree::char_iterator& TextRBTree::char_iterator::operator=(const TextRBTree::iterator_base& iter)
{
	if (this == &iter) return *this;

	TextRBTree::iterator_base::operator=(iter);

	return *this;
}

bool TextRBTree::char_iterator::operator==(const char_iterator &other) const
{
	assert (tree != NULL);
	return (other.node == this->node) && (other.char_offset == this->char_offset);
}

bool TextRBTree::char_iterator::operator!=(const char_iterator &other) const
{
	assert (tree != NULL);
	return (other.node != this->node) || (other.char_offset != this->char_offset);
}


TextRBTree::char_iterator& TextRBTree::char_iterator::operator++()
{
	forward_chars(1);
	return *this;
}

TextRBTree::char_iterator& TextRBTree::char_iterator::operator--()
{
	backward_chars(1);
	return *this;
}

TextRBTree::char_iterator TextRBTree::char_iterator::operator++(int)
{
	char_iterator copy = *this;
	++(*this);
	return copy;
}

TextRBTree::char_iterator TextRBTree::char_iterator::operator--(int)
{
	char_iterator copy = *this;
	--(*this);
	return copy;
}

TextRBTree::char_iterator& TextRBTree::char_iterator::operator+=(unsigned int n)
{
	forward_chars(n);
	return *this;
}

TextRBTree::char_iterator& TextRBTree::char_iterator::operator-=(unsigned int n)
{
	backward_chars(n);
	return *this;
}

TextRBTree::col_iterator::col_iterator(void)
: iterator_base()
{
}

TextRBTree::col_iterator::col_iterator(TextRBTree &tree)
: iterator_base(tree)
{
}

TextRBTree::col_iterator::col_iterator(Node &node)
: iterator_base(node)
{
}

TextRBTree::col_iterator::col_iterator(const iterator_base &iter)
: iterator_base(iter)
{
}

TextRBTree::col_iterator& TextRBTree::col_iterator::operator=(const TextRBTree::col_iterator& iter)
{
	if (this == &iter) return *this;

	TextRBTree::iterator_base::operator=(iter);

	return *this;
}

TextRBTree::col_iterator& TextRBTree::col_iterator::operator=(const TextRBTree::iterator_base& iter)
{
	if (this == &iter) return *this;

	TextRBTree::iterator_base::operator=(iter);

	return *this;
}

bool TextRBTree::col_iterator::operator==(const col_iterator &other) const
{
	assert (tree != NULL);
	return (other.node == this->node) && (other.col_offset == this->col_offset);
}

bool TextRBTree::col_iterator::operator!=(const col_iterator &other) const
{
	assert (tree != NULL);
	return (other.node != this->node) || (other.col_offset != this->col_offset);
}

TextRBTree::col_iterator& TextRBTree::col_iterator::operator++()
{
	forward_cols(1);
	return *this;
}

TextRBTree::col_iterator& TextRBTree::col_iterator::operator--()
{
	backward_cols(1);
	return *this;
}

TextRBTree::col_iterator TextRBTree::col_iterator::operator++(int)
{
	col_iterator copy = *this;
	++(*this);
	return copy;
}

TextRBTree::col_iterator TextRBTree::col_iterator::operator--(int)
{
	col_iterator copy = *this;
	--(*this);
	return copy;
}

TextRBTree::col_iterator& TextRBTree::col_iterator::operator+=(unsigned int n)
{
	forward_cols(n);
	return *this;
}

TextRBTree::col_iterator& TextRBTree::col_iterator::operator-=(unsigned int n)
{
	backward_cols(n);
	return *this;
}

TextRBTree::line_iterator::line_iterator(void)
: iterator_base()
{
}

TextRBTree::line_iterator::line_iterator(TextRBTree &tree)
: iterator_base(tree)
{
}

TextRBTree::line_iterator::line_iterator(Node &node)
: iterator_base(node)
{
}

TextRBTree::line_iterator::line_iterator(const iterator_base &iter)
: iterator_base(iter)
{
}

TextRBTree::line_iterator& TextRBTree::line_iterator::operator=(const TextRBTree::line_iterator& iter)
{
	if (this == &iter) return *this;

	TextRBTree::iterator_base::operator=(iter);

	return *this;
}

TextRBTree::line_iterator& TextRBTree::line_iterator::operator=(const TextRBTree::iterator_base& iter)
{
	if (this == &iter) return *this;

	TextRBTree::iterator_base::operator=(iter);

	return *this;
}

bool TextRBTree::line_iterator::operator==(const line_iterator &other) const
{
	assert (tree != NULL);
	return (other.node == this->node) && (other.line_offset == this->line_offset);
}

bool TextRBTree::line_iterator::operator!=(const line_iterator &other) const
{
	assert (tree != NULL);
	return (other.node != this->node) || (other.line_offset != this->line_offset);
}

TextRBTree::line_iterator& TextRBTree::line_iterator::operator++()
{
	forward_lines(1);
	return *this;
}

TextRBTree::line_iterator& TextRBTree::line_iterator::operator--()
{
	backward_lines(1);
	return *this;
}

TextRBTree::line_iterator TextRBTree::line_iterator::operator++(int)
{
	line_iterator copy = *this;
	++(*this);
	return copy;
}

TextRBTree::line_iterator TextRBTree::line_iterator::operator--(int)
{
	line_iterator copy = *this;
	--(*this);
	return copy;
}

TextRBTree::line_iterator& TextRBTree::line_iterator::operator+=(unsigned int n)
{
	forward_lines(n);
	return *this;
}

TextRBTree::line_iterator& TextRBTree::line_iterator::operator-=(unsigned int n)
{
	backward_lines(n);
	return *this;
}
