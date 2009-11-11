/*
 * Copyright (C) 2008 by Mark Pustjens <pustjens@dds.nl>
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

#include "TextLineRBTree.h"

#include <cassert>

TextLineRBTree::TextLineRBTree()
{
	nil = MakeNil();

	root = MakeNil();
	root->parent = nil;
	root->left = nil;
	root->right = nil;
	root->pred = nil;
	root->succ = nil;
}

TextLineRBTree::~TextLineRBTree()
{
	if (root != nil)
		delete root; /* Deletes nodes recursively. */

	delete nil;
}

TextLineRBTree& TextLineRBTree::operator=(const TextLineRBTree& tree)
{
	Node *node;
	TextLineRBTree::char_iterator iter;

	if (this == &tree) return *this;

	nil = MakeNil();

	root = MakeNil();
	root->parent = nil;
	root->left = nil;
	root->right = nil;
	root->pred = nil;
	root->succ = nil;

	iter = begin();
	for (node = tree.tree_minimum(tree.root); node != tree.nil; node = node->succ) {
		iter = insert(iter, node->str, node->str_bytes);
	}	

	return *this;
}

TextLineRBTree::Node* TextLineRBTree::MakeNil(void)
{
	Node* nil = new Node(this, NULL);

	/* Set all pointers of the nil node to nil. The nil node is fake. */
	nil->parent = nil;
	nil->left = nil;
	nil->right = nil;
	nil->pred = nil;
	nil->succ = nil;

	/* Set the data to null. */
	nil->str = NULL;
	nil->str_bytes = 0;
	nil->str_chars = 0;
	nil->str_cols = 0;

	/* Set all order statistic data of nil to 0 (or the unit element.) */
	nil->bytes = 0;
	nil->chars = 0;
	nil->cols = 0;

	return nil;
}

TextLineRBTree::Node::Node(TextLineRBTree *tree, Node *parent)
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
, str(NULL)
, str_bytes(0)
, str_chars(0)
, str_cols(0)
{
}

TextLineRBTree::Node::~Node()
{
	g_free(str);
}

TextLineRBTree::char_iterator TextLineRBTree::insert(const TextLineRBTree::char_iterator& iter, const char* str, unsigned int len)
{
	Node *node = iter.node;
	char_iterator retval = iter;

	if (node->str == NULL) {
		node->str = g_strndup(str, len);
		node->str_bytes = len;
	} else {
		char* s;

		if (iter.byte_offset == 0) {
			s = g_strconcat(str, node->str, NULL);
		} else if (iter.byte_offset > node->str_bytes) {
			s = g_strconcat(node->str, str, NULL);
		} else {
			s = g_new0(char, node->str_bytes + len + 1);
			g_strlcpy(s, node->str, iter.byte_offset + 1);
			g_strlcpy(s + iter.byte_offset, str, len + 1);
			g_strlcpy(s + iter.byte_offset + len,
					node->str + iter.byte_offset, node->str_bytes - iter.byte_offset + 2);
		}

		g_free(node->str);
		node->str = s;
		node->str_bytes = node->str_bytes + len;
	}

	if (node->str == NULL) {
		node->str_chars = 0;
		node->str_cols = 0;
	} else {
		node->str_chars = g_utf8_strlen(node->str, -1);
		node->str_cols = width(node->str, node->str + node->str_bytes);
	}

	/* node has changed, so update augmented data. */
	post_update_augmentation_fixup(node);

	retval.forward_bytes(len);

	return retval;
}

void TextLineRBTree::post_update_augmentation_fixup(Node *z)
{
	Node *x = NULL;

	/* Fix the order statistics data. Remember that z is the parent
	 * of the node which was erased. */

	/* Decrement the order statistic data for all ancestors. */
	for (x = z; x != nil; x = x->parent)
	{
		x->bytes = x->str_bytes + x->left->bytes + x->right->bytes;
		x->chars = x->str_chars + x->left->chars + x->right->chars;
		x->cols = x->str_cols + x->left->cols + x->right->cols;
	}

}

void TextLineRBTree::erase(const TextLineRBTree::char_iterator pos)
{
	char_iterator iter = pos;
	erase(iter, ++iter);
}

void TextLineRBTree::erase(TextLineRBTree::char_iterator start, TextLineRBTree::char_iterator end)
{
	char* from;
	char* to;

	Node *node = start.node;

	assert(start.tree == this && end.tree == this);
	assert(start.node == end.node);

	if (start.byte_offset == end.byte_offset)
		return;

	if (node->str == NULL)
		return;

	if (end.byte_offset < end.node->str_bytes)
		end++;

	from = node->str + end.byte_offset;
	to = node->str + start.byte_offset;

	while (*from != '\0')
	{
		*to = *from;
		from++;
		to++;
	}

	*to = *from;

	node->str = g_renew(char, node->str, start.byte_offset + node->str_bytes - end.byte_offset + 1);

	node->str_bytes = start.byte_offset + node->str_bytes - end.byte_offset;
	node->str_chars = g_utf8_pointer_to_offset(node->str, node->str + node->str_bytes);
	node->str_cols = width(node->str, node->str + node->str_bytes);

	post_update_augmentation_fixup(node);
}

unsigned int TextLineRBTree::chars(void) const
{
	return root->chars;
}

unsigned int TextLineRBTree::bytes(void) const
{
	return root->bytes;
}

unsigned int TextLineRBTree::cols(void) const
{
	return root->cols;
}

unsigned int TextLineRBTree::lines(void) const
{
	return root->lines;
}

TextLineRBTree::char_iterator TextLineRBTree::begin(void) const
{
	return char_iterator(*tree_minimum(root));
}

TextLineRBTree::char_iterator TextLineRBTree::back(void) const
{
	TextLineRBTree::char_iterator iter(*tree_maximum(root));
	iter.forward_bytes(iter.node->bytes);
	return iter;
}

TextLineRBTree::char_iterator TextLineRBTree::end(void) const
{
	return char_iterator(*nil);
}

TextLineRBTree::char_iterator TextLineRBTree::reverse_begin(void) const
{
	TextLineRBTree::char_iterator iter;
	iter = char_iterator(*tree_maximum(root));
	iter.byte_offset = iter.node->str_bytes;
	iter.char_offset = iter.node->str_chars;
	iter.col_offset = iter.node->str_cols;
	return iter;
}

TextLineRBTree::char_iterator TextLineRBTree::reverse_back(void) const
{
	TextLineRBTree::char_iterator iter(*tree_minimum(root));
	iter.backward_bytes(1);
	return iter;
}

TextLineRBTree::char_iterator TextLineRBTree::reverse_end(void) const
{
	return char_iterator(*nil);
}

TextLineRBTree::char_iterator TextLineRBTree::get_iter_at_char_offset(unsigned int index)
{
	Node* node;		/* Current node we are looking at. */
	unsigned int r;		/* Number of chars in de left subtree of node and the node itself. */
	unsigned int bytes;	/* byte_offset to the current node. */
	unsigned int i;		/* The i'th character being looked for in the current subtree. */
	
	/* If the index i falls outside the range of the tree, return
	 * the appropriate iterator. */
	if (index == 0) {
		return begin();
	} else if (index > root->chars) {
		return end();
	}


	/* Initialisation for the while loop. */
	node = root;
	i = index;
	bytes = 0;

	/* Find the node where the i'th character is stored. */
	while (true) {
		r = node->left->chars + node->str_chars;

		if (node->left->chars < i && i <= r) {
			break;
		} else if ( i <= node->left->chars ) {
			node = node->left;
		} else { /* r < i */
			i -= r;
			bytes += node->left->bytes + node->str_bytes;
			node = node->right;
		}
	}

	/* Now we have found the node, create the iterator. */
	char_iterator iter;

	iter.node = node;
	iter.tree = this;
	iter.byte_offset = g_utf8_offset_to_pointer(node->str, i) - node->str;
	iter.char_offset = g_utf8_pointer_to_offset(node->str, node->str +iter. byte_offset);
	iter.col_offset = width(node->str, node->str + iter.byte_offset);

	return iter;
}

void TextLineRBTree::rotate_left(Node *x)
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

void TextLineRBTree::rotate_right(Node *x)
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

void TextLineRBTree::post_rotate_augmentation_fixup(Node *x, Node *y)
{
	/* Fix the order statistics data. Remember that x is now a
	 * child of y so x needs to be recalculated first. */

	x->bytes = x->str_bytes + x->left->bytes + x->right->bytes;
	x->chars = x->str_chars + x->left->chars + x->right->chars;
	x->cols = x->str_cols + x->left->cols + x->right->cols;

	y->bytes = y->str_bytes + y->left->bytes + y->right->bytes;
	y->chars = y->str_chars + y->left->chars + y->right->chars;
	y->cols = y->str_cols + y->left->cols + y->right->cols;
}

/*TextLineRBTree::char_iterator TextLineRBTree::insert(Node *z, int line_nr)
{
	Node *y = NULL;
	Node *x = root;
	int line = 0;

	assert(z->tree == this);

	* Find the insertion point. That is: find the parent
	 * for the new node.
	 *
	 * Here we find the node with line number line_nr
	 * and insert the new node before this node.
	 *
	 * If there are less than line_nr lines, the node is added
	 * as the last node.
	 *
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

	post_insert_augmentation_fixup(z);

	insert_fixup(z);

	return TextLineRBTree::char_iterator(*z);
}*/

/*
TextLineRBTree::char_iterator TextLineRBTree::insert(const TextLineRBTree::char_iterator& iter, const TextLine& line)
{
	return insert(new Node(this, NULL, line),
			iter.node->line_nr());
}*/

void TextLineRBTree::post_insert_augmentation_fixup(Node *z)
{
	Node *x = NULL;

	/* Fix the order statistics data. Remember that z was
	 * added, so the values of all ancestors should increase. */

	z->bytes = z->str_bytes;
	z->chars = z->str_chars;
	z->cols = z->str_cols;

	/* Increment the order statistic data for all ancestors. */
	for (x = z->parent; x != nil; x = x->parent)
	{
		x->bytes += z->bytes;
		x->chars += z->chars;
		x->cols += z->cols;
	}
}

void TextLineRBTree::insert_fixup(Node *z)
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
}

/* There are three cases when removing a node z from a Red-Black tree.
 * 1) z has no children.
 *    In this case we simply remove z and fix the order statistic data by following
 *    the path from z->parent to the root.
 * 2) z has one child.
 *    In this case we replace z by its only child. Then we fix the order
 *    statistic data as in case 1.
 * 3) z has two children.
 *    Now we need to swap z with its successor. After swapping z has at most
 *    one child as the successor of z has at most one child when z has two
 *    children. Now we can recursively call erase() to erase z from its new
 *    position. */
void TextLineRBTree::erase(Node *z)
{
	Node *y;

	assert (z != NULL); 
	assert (z->tree == this);

	if ( (z->left == nil) || (z->right == nil) ) /* z has at most one child. */
	{
		if (z == z->parent->left) {
			if (z->left != nil) z->parent->left = z->left;
			else z->parent->left = z->right;
		} else { /* z == z->parent->right */
			if (z->left != nil) z->parent->right = z->left;
			else z->parent->right = z->right;
		}

		/* Fix the predesessor/successor pointers. */
		if (z->pred)
			z->pred->succ = z->succ;

		if (z->succ)
			z->succ->pred = z->pred;

		/* Fix the augmented data before fixing the RBTree properties. */
		post_erase_augmentation_fixup(z->parent);

		/* See if we need to fix the RBTree properties. */
		if (z->color == BLACK)
			erase_node_fixup(z->parent); //TODO should this be the parent or a child?

		/* Free the memory. */
		z->left = nil;
		z->right = nil;
		delete z;

	} else { /* z has two children*/

		/* The book algorithm moves the key and satellite data to
		 * y. We need to swap the actual nodes because iterators
		 * point to nodes. If we would move the key and satellite
		 * data, then deleting a node might invalidate any iterator.
		 * By swapping the actual nodes, an iterator remains valid
		 * until its node is deleted.
		 *
		 * The books code is as follows:
		 * z->key = y->key;
		 * z->data = y->data;
		 */

		/* We need to swap z with its successor, which we call y.
		 * Note that we don't need to call successor() since this
		 * implementation also stores prodecessor and successor
		 * pointers.
		 *
		 * For each node 6 pointers need to be adjusted.
		 * Each of the three forward pointers from z has its dual
		 * in the node it points to.
		 *
		 * O     z->parent
		 *  \
		 *   O   z
		 *  / \
		 * O   O z->left / z->right
		 *
		 */

		y = z->succ;

		/* Make a copy of z. */
		Node zcopy(this, z->parent);
		zcopy.left = z->left;
		zcopy.right = z->right;

		/* First put z in it's new place. */
		if (y == y->parent->left) y->parent->left = z;
		else y->parent->right = z;
		z->parent = y->parent;
		z->left = y->left;
		z->right = y->right;
		z->left->parent = z;
		z->right->parent = z;

		/* Then put y in it's new place. */
		if (&zcopy == zcopy.parent->left) zcopy.parent->left = y;
		else zcopy.parent->right = y;
		y->parent = zcopy.parent;
		y->left = zcopy.left;
		y->right = zcopy.right;
		y->left->parent = y;
		y->right->parent = y;

		/* Now z has at most one child, so this can be handled by the simple
		 * case of erasing a node.
		 * Note that when this recursive call fixes the order statistic data
		 * it will also visit y, hence all is well */
		erase(z);
	}
}		

void TextLineRBTree::post_erase_augmentation_fixup(Node *z)
{
	Node *x = NULL;

	/* Fix the order statistics data. Remember that z is the parent
	 * of the node which was erased. */

	/* Decrement the order statistic data for all ancestors. */
	for (x = z; x != nil; x = x->parent)
	{
		x->bytes = x->str_bytes + x->left->bytes + x->right->bytes;
		x->chars = x->str_chars + x->left->chars + x->right->chars;
		x->cols = x->str_cols + x->left->cols + x->right->cols;
	}
}

void TextLineRBTree::erase_node_fixup(Node *x)
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

TextLineRBTree::Node* TextLineRBTree::tree_minimum(Node *node) const
{
	Node *min = node;

	assert(node->tree == this);

	while (min->left != nil)
		min = min->left;

	return min;
}

TextLineRBTree::Node* TextLineRBTree::tree_maximum(Node *node) const
{
	Node *max = node;

	assert(node->tree == this);

	while (max->right != nil)
		max = max->right;

	return max;
}

TextLineRBTree::Node* TextLineRBTree::successor(Node *node) const
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

TextLineRBTree::Node* TextLineRBTree::predecessor(Node *node) const
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

TextLineRBTree::iterator_base::iterator_base(void)
: tree(NULL)
, node(NULL)
, byte_offset(0)
, char_offset(0)
, col_offset(0)
{
}

TextLineRBTree::iterator_base::iterator_base(TextLineRBTree &_tree)
: tree(&_tree)
, node(NULL)
, byte_offset(0)
, char_offset(0)
, col_offset(0)
{
	node = _tree.tree_minimum(_tree.root);
}

TextLineRBTree::iterator_base::iterator_base(Node &node_)
: tree(node_.tree)
, node(&node_)
, byte_offset(0)
, char_offset(0)
, col_offset(0)
{
}

TextLineRBTree::iterator_base::iterator_base(const iterator_base &iter)
{
	*this = iter;
}

bool TextLineRBTree::iterator_base::valid(void) const
{
	return tree != NULL;
}

bool TextLineRBTree::iterator_base::valid_char(void) const
{
	return g_unichar_validate( g_utf8_get_char(node->str + byte_offset) );
}

unsigned int TextLineRBTree::iterator_base::byte_index(void) const
{
	Node *y;
	unsigned int index;
       
	index = node->left->bytes + byte_offset;
	y = node;

	while (y != tree->root) {
		if (y == y->parent->right) {
			index += y->parent->left->bytes + y->parent->str_bytes;
		}
		
		y = y->parent;
	}

	return index;
}

unsigned int TextLineRBTree::iterator_base::char_bytes(void)
{
	return g_utf8_next_char(node->str + byte_offset) - (node->str + byte_offset);
}

unsigned int TextLineRBTree::iterator_base::char_cols(void)
{
	gunichar c;

	c = g_utf8_get_char(node->str + byte_offset);

	return (g_unichar_iswide(c) ? 2 : 1);
}

unsigned int TextLineRBTree::iterator_base::chars(void) const
{
	return node->chars;
}

unsigned int TextLineRBTree::iterator_base::bytes(void) const
{
	return node->bytes;
}

unsigned int TextLineRBTree::iterator_base::cols(void) const
{
	return node->cols;
}

unsigned int TextLineRBTree::iterator_base::lines(void) const
{
//TODO implement correctly.
	return 1;
}

char* TextLineRBTree::iterator_base::operator*() const
{
	assert(node->str != NULL);

	return node->str + byte_offset;
}

TextLineRBTree::iterator_base& TextLineRBTree::iterator_base::operator=(const TextLineRBTree::iterator_base& iter)
{
	if (this == &iter) return *this;

	node = iter.node;
	tree = iter.tree;
	byte_offset = iter.byte_offset;
	char_offset = iter.char_offset;
	col_offset = iter.col_offset;

	return *this;
}

bool TextLineRBTree::iterator_base::operator<(const iterator_base& iter)
{
	return (this->byte_index() < iter.byte_index()) ||
		((this->node == iter.node) && (this->byte_offset < iter.byte_offset));
}

TextLineRBTree::iterator_base& TextLineRBTree::iterator_base::forward_bytes(unsigned int n)
{
	assert(tree != NULL);

	if (node == tree->nil)
		return *this;

	while ( (node != tree->nil) && ((byte_offset + n) > (node->str_bytes - 1)) ) {
		n -= node->str_bytes - byte_offset;
		node = node->succ;
		byte_offset = 0;
	}

	if (node != tree->nil) {
		byte_offset += n;
		char_offset = g_utf8_pointer_to_offset(node->str, node->str + byte_offset);
		col_offset = width(node->str, node->str + byte_offset);
	} else {
		byte_offset = 0;
		char_offset = 0;
		col_offset = 0;
	}

	return *this;
}

TextLineRBTree::iterator_base& TextLineRBTree::iterator_base::backward_bytes(unsigned int n)
{
	assert(tree != NULL);

	if (node == tree->nil)
		return *this;

	while ( (node != tree->nil) && (byte_offset < n) ) {
		n -= byte_offset;
		node = node->pred;
		byte_offset = node->str_bytes - 1;
	}

	if (node != tree->nil) {
		byte_offset -= n;
		char_offset = g_utf8_pointer_to_offset(node->str, node->str + byte_offset);
		col_offset = width(node->str, node->str + byte_offset);;
	} else {
		byte_offset = 0;
		char_offset = 0;
		col_offset = 0;
	}

	return *this;
}

TextLineRBTree::iterator_base& TextLineRBTree::iterator_base::forward_chars(unsigned int n)
{
	assert(tree != NULL);

	if (node == tree->nil)
		return *this;

	while ( (node != tree->nil) && ((char_offset + n) > (node->str_chars - 1)) ) {
		n -= node->str_chars - char_offset;
		node = node->succ;
		char_offset = 0;
	}

	if (node != tree->nil) {
		char_offset += n;
		byte_offset = g_utf8_offset_to_pointer(node->str, char_offset) - node->str;
		col_offset = width(node->str, node->str + byte_offset);
	} else {
		byte_offset = 0;
		char_offset = 0;
		col_offset = 0;
	}

	return *this;
}

TextLineRBTree::iterator_base& TextLineRBTree::iterator_base::backward_chars(unsigned int n)
{
	assert(tree != NULL);

	if (node == tree->nil)
		return *this;

	while ( (node != tree->nil) && (char_offset < n) ) {
		n -= char_offset;
		node = node->pred;
		char_offset = node->str_chars - 1;
	}

	if (node != tree->nil) {
		char_offset -= n;
		byte_offset = g_utf8_offset_to_pointer(node->str, char_offset) - node->str;
		col_offset = width(node->str, node->str + byte_offset);
	} else {
		byte_offset = 0;
		char_offset = 0;
		col_offset = 0;
	}

	return *this;
}

TextLineRBTree::iterator_base& TextLineRBTree::iterator_base::forward_cols(unsigned int n)
{
	assert(tree != NULL);

	if (node == tree->nil)
		return *this;

	while ( (node != tree->nil) && ((col_offset + n) > (node->str_cols - 1)) ) {
		n -= node->str_cols - col_offset;
		node = node->succ;
		col_offset = 0;
	}

	if (node != tree->nil) {
		col_offset += n;
		byte_offset = col_offset_to_pointer(node->str, col_offset) - node->str;
		char_offset = g_utf8_pointer_to_offset(node->str, node->str + byte_offset);
	} else {
		byte_offset = 0;
		char_offset = 0;
		col_offset = 0;
	}

	return *this;
}

TextLineRBTree::iterator_base& TextLineRBTree::iterator_base::backward_cols(unsigned int n)
{
	assert(tree != NULL);

	if (node == tree->nil)
		return *this;

	while ( (node != tree->nil) && (col_offset < n) ) {
		n -= col_offset;
		node = node->pred;
		col_offset = node->str_cols - 1;
	}

	if (node != tree->nil) {
		col_offset -= n;
		byte_offset = col_offset_to_pointer(node->str, col_offset) - node->str;
		char_offset = g_utf8_pointer_to_offset(node->str, node->str + byte_offset);
	} else {
		byte_offset = 0;
		char_offset = 0;
		col_offset = 0;
	}

	return *this;
}

TextLineRBTree::byte_iterator::byte_iterator(void)
: iterator_base()
{
}

TextLineRBTree::byte_iterator::byte_iterator(TextLineRBTree &tree)
: iterator_base(tree)
{
}

TextLineRBTree::byte_iterator::byte_iterator(Node &node)
: iterator_base(node)
{
}

TextLineRBTree::byte_iterator::byte_iterator(const iterator_base &iter)
: iterator_base(iter)
{
}

TextLineRBTree::byte_iterator& TextLineRBTree::byte_iterator::operator=(const TextLineRBTree::byte_iterator& iter)
{
	if (this == &iter) return *this;

	TextLineRBTree::iterator_base::operator=(iter);

	return *this;
}

TextLineRBTree::byte_iterator& TextLineRBTree::byte_iterator::operator=(const TextLineRBTree::iterator_base& iter)
{
	if (this == &iter) return *this;

	TextLineRBTree::iterator_base::operator=(iter);

	return *this;
}

bool TextLineRBTree::byte_iterator::operator==(const byte_iterator &other) const
{
	assert (tree != NULL);
	return (other.node == this->node) && (other.byte_offset == this->byte_offset);
}

bool TextLineRBTree::byte_iterator::operator!=(const byte_iterator &other) const
{
	assert (tree != NULL);
	return (other.node != this->node) || (other.byte_offset != this->byte_offset);
}

TextLineRBTree::byte_iterator& TextLineRBTree::byte_iterator::operator++()
{
	forward_bytes(1);
	return *this;
}

TextLineRBTree::byte_iterator& TextLineRBTree::byte_iterator::operator--()
{
	backward_bytes(1);
	return *this;
}

TextLineRBTree::byte_iterator TextLineRBTree::byte_iterator::operator++(int)
{
	byte_iterator copy = *this;
	++(*this);
	return copy;
}

TextLineRBTree::byte_iterator TextLineRBTree::byte_iterator::operator--(int)
{
	byte_iterator copy = *this;
	--(*this);
	return copy;
}

TextLineRBTree::byte_iterator& TextLineRBTree::byte_iterator::operator+=(unsigned int n)
{
	forward_bytes(n);
	return *this;
}

TextLineRBTree::byte_iterator& TextLineRBTree::byte_iterator::operator-=(unsigned int n)
{
	backward_bytes(n);
	return *this;
}

TextLineRBTree::char_iterator::char_iterator(void)
: iterator_base()
{
}

TextLineRBTree::char_iterator::char_iterator(TextLineRBTree &tree)
: iterator_base(tree)
{
}

TextLineRBTree::char_iterator::char_iterator(Node &node)
: iterator_base(node)
{
}

TextLineRBTree::char_iterator::char_iterator(const iterator_base &iter)
: iterator_base(iter)
{
}

TextLineRBTree::char_iterator& TextLineRBTree::char_iterator::operator=(const TextLineRBTree::char_iterator& iter)
{
	if (this == &iter) return *this;

	TextLineRBTree::iterator_base::operator=(iter);

	return *this;
}

TextLineRBTree::char_iterator& TextLineRBTree::char_iterator::operator=(const TextLineRBTree::iterator_base& iter)
{
	if (this == &iter) return *this;

	TextLineRBTree::iterator_base::operator=(iter);

	return *this;
}

bool TextLineRBTree::char_iterator::operator==(const char_iterator &other) const
{
	assert (tree != NULL);
	return (other.node == this->node) && (other.char_offset == this->char_offset);
}

bool TextLineRBTree::char_iterator::operator!=(const char_iterator &other) const
{
	assert (tree != NULL);
	return (other.node != this->node) || (other.char_offset != this->char_offset);
}


TextLineRBTree::char_iterator& TextLineRBTree::char_iterator::operator++()
{
	forward_chars(1);
	return *this;
}

TextLineRBTree::char_iterator& TextLineRBTree::char_iterator::operator--()
{
	backward_chars(1);
	return *this;
}

TextLineRBTree::char_iterator TextLineRBTree::char_iterator::operator++(int)
{
	char_iterator copy = *this;
	++(*this);
	return copy;
}

TextLineRBTree::char_iterator TextLineRBTree::char_iterator::operator--(int)
{
	char_iterator copy = *this;
	--(*this);
	return copy;
}

TextLineRBTree::char_iterator& TextLineRBTree::char_iterator::operator+=(unsigned int n)
{
	forward_chars(n);
	return *this;
}

TextLineRBTree::char_iterator& TextLineRBTree::char_iterator::operator-=(unsigned int n)
{
	backward_chars(n);
	return *this;
}

TextLineRBTree::col_iterator::col_iterator(void)
: iterator_base()
{
}

TextLineRBTree::col_iterator::col_iterator(TextLineRBTree &tree)
: iterator_base(tree)
{
}

TextLineRBTree::col_iterator::col_iterator(Node &node)
: iterator_base(node)
{
}

TextLineRBTree::col_iterator::col_iterator(const iterator_base &iter)
: iterator_base(iter)
{
}

TextLineRBTree::col_iterator& TextLineRBTree::col_iterator::operator=(const TextLineRBTree::col_iterator& iter)
{
	if (this == &iter) return *this;

	TextLineRBTree::iterator_base::operator=(iter);

	return *this;
}

TextLineRBTree::col_iterator& TextLineRBTree::col_iterator::operator=(const TextLineRBTree::iterator_base& iter)
{
	if (this == &iter) return *this;

	TextLineRBTree::iterator_base::operator=(iter);

	return *this;
}

bool TextLineRBTree::col_iterator::operator==(const col_iterator &other) const
{
	assert (tree != NULL);
	return (other.node == this->node) && (other.col_offset == this->col_offset);
}

bool TextLineRBTree::col_iterator::operator!=(const col_iterator &other) const
{
	assert (tree != NULL);
	return (other.node != this->node) || (other.col_offset != this->col_offset);
}


TextLineRBTree::col_iterator& TextLineRBTree::col_iterator::operator++()
{
	forward_cols(1);
	return *this;
}

TextLineRBTree::col_iterator& TextLineRBTree::col_iterator::operator--()
{
	backward_cols(1);
	return *this;
}

TextLineRBTree::col_iterator TextLineRBTree::col_iterator::operator++(int)
{
	col_iterator copy = *this;
	++(*this);
	return copy;
}

TextLineRBTree::col_iterator TextLineRBTree::col_iterator::operator--(int)
{
	col_iterator copy = *this;
	--(*this);
	return copy;
}

TextLineRBTree::col_iterator& TextLineRBTree::col_iterator::operator+=(unsigned int n)
{
	forward_cols(n);
	return *this;
}

TextLineRBTree::col_iterator& TextLineRBTree::col_iterator::operator-=(unsigned int n)
{
	backward_cols(n);
	return *this;
}

void TextLineRBTree::print(void)
{
	printf("BEGIN Redblack tree: %p", (void*)this);
	print_node(root);
	printf("END Redblack tree: %p", (void*)this);
}

void TextLineRBTree::print_node(Node *node)
{
	printf("BEGIN node: %p", (void*)node);
	printf("left");
	if (node->left != nil) print_node(node->left);
	printf("right");
	if (node->right!= nil) print_node(node->right);
	printf("END node: %p", (void*)this);
}
