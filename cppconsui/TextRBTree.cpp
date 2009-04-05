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

/* The red-black tree implementation used here is based on the one
 * from the book `Introduction to Algorithms, second edition' by
 * Thomas H. Cormen et. al., ISBN 0-262-03296-7.
 *
 * There are some small differences due to implementation details.
 * */

//TODO overflow safe operations 

#include "config.h"

#include "TextRBTree.h"

#include <glib.h>

#include <cassert>

#include <stdio.h>

TextRBTree::TextRBTree()
{
	nil = new Node(this, NULL);
	root = nil;

	/* Set all pointers of the nil node to nil. The nil node is fake. */
	nil->parent = nil;
	nil->left = nil;
	nil->right = nil;
	nil->pred = nil;
	nil->succ = nil;

	/* Set all order statistic data of nil to 0. */
	nil->lines = 0;
	nil->lines_wrap = 0;
}

TextRBTree::~TextRBTree()
{
	if (root != nil)
		delete root; /* Deletes nodes recursively. */

	delete nil;
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
	/* Fix the order statistics data.
	 * remember that x is now a child of y.
	 */

	//TODO fix lines_wrap when textlin.e know how to calc it
	x->lines = 1 + x->left->lines + x->right->lines;
	x->lines_wrap = 1 + x->left->lines_wrap + x->right->lines_wrap;

	y->lines = 1 + y->left->lines + y->right->lines;
	y->lines_wrap = 1 + y->left->lines_wrap + y->right->lines_wrap;
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

	return TextRBTree::char_iterator(*z);
}

TextRBTree::char_iterator TextRBTree::insert(const TextRBTree::char_iterator& iter, const TextLine& line)
{
	return insert(new Node(this, NULL, line),
			iter.node->line_nr());
}

void TextRBTree::post_insert_augmentation_fixup(Node *z)
{
	Node *x = NULL;

	z->lines = 1;
	z->lines_wrap = 1; //TODO get actual nr of lines when wrapped

	/* Increment the order statistic data for all ancestors. */
	for (x = z->parent; x != nil; x = x->parent)
	{
		x->lines += z->lines;
		x->lines_wrap += z->lines_wrap;
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
}

TextRBTree::line_iterator TextRBTree::erase(TextRBTree::line_iterator pos)
{
	assert(pos.tree == this);

	return erase(pos.node);
}

TextRBTree::line_iterator TextRBTree::erase(TextRBTree::line_iterator start, TextRBTree::line_iterator _end)
{
	Node *n;

	assert(start.tree == this);
	assert(_end.tree == this);

	/* Node deletion operations invalidate the iterator, so we need
	 * to advance the iterator before we can erase the node. */

	while (start != _end)
	{
		/* Store the node. */
		n = start.node;

		/* Advance iterator. */
		start++;

		/* Delete node. */
		erase(n);
	}

	return _end;
}

TextRBTree::line_iterator TextRBTree::erase(Node *z)
{
	Node *y = NULL; /* The node taking z's place */
	Node *x = NULL; /* The node taking y's place */
	TextRBTree::line_iterator retval(*this);

	assert(z->tree == this);

	/* Get an iterator to the next node. */
	retval = line_iterator(*z->succ);

	/* Find the node which is going to replace z. */
	if ( (z->left == nil) || (z->right == nil) )
		y = z;
	else
		y = successor(z);

	if ( y->left != nil)
		x = y->left;
	else
		x = y->right;

	x->parent = y->parent;

	if (y->parent == nil)
		root = x;
	else if (y == y->parent->left)
		y->parent->left = x;
	else
		y->parent->right = x;

	if (y != z)
	{
		Color tmp;
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

		/* In the code below, y replaces z, and z is deleted. */

		/* Fix the predesessor/successor pointers */
		if (z->pred)
			z->pred->succ = z->succ;

		if (z->succ)
			z->succ->pred = z->pred;

		/* Swap the actual nodes. */
		y->parent = z->parent;

		tmp = y->color;
		y->color = z->color;
		z->color = tmp;

		y->left = z->left;
		y->right = z->right;

		if (z == z->parent->left)
			z->parent->left = y;
		else
			z->parent->right = y;
	}

	if (z->color == BLACK)
		erase_node_fixup(x);

	/* Free the memory. */
	z->left = nil;
	z->right = nil;
	delete z;

	return retval;
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

void TextRBTree::print(void)
{
	printf("BEGIN Redblack tree: %p", this);
	print_node(root);
	printf("END Redblack tree: %p", this);
}

void TextRBTree::print_node(Node *node)
{
	printf("BEGIN node: %p", node);
	printf("left");
	if (node->left != nil) print_node(node->left);
	printf("right");
	if (node->right!= nil) print_node(node->right);
	printf("END node: %p", this);
}

TextRBTree::Node* TextRBTree::successor(Node *node) const
{
	Node *x = node;
	Node *y = NULL;
	
	assert(node->tree == this);

	if (x->right != nil)
		return tree_minimum(x);

	y = x->parent;

	while (y != nil && x == x->right)
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

	while (y != nil && x == x->left)
	{
		x = y;
		y = y->parent;
	}

	return y;
}

TextRBTree::char_iterator TextRBTree::begin(void) const
{
	return char_iterator(*tree_minimum(root));
}

TextRBTree::char_iterator TextRBTree::back(void) const
{
	TextRBTree::char_iterator it(*tree_maximum(root));
	it.forward_bytes(it.node->line.bytes());
	return it;
}

TextRBTree::char_iterator TextRBTree::end(void) const
{
	return char_iterator(*nil);
}

TextRBTree::char_iterator TextRBTree::reverse_begin(void) const
{
	return char_iterator(*tree_maximum(root));
}

TextRBTree::char_iterator TextRBTree::reverse_back(void) const
{
	TextRBTree::char_iterator it(*tree_minimum(root));
	it.backward_bytes(it.node->line.bytes());
	return it;
}

TextRBTree::char_iterator TextRBTree::reverse_end(void) const
{
	return char_iterator(*nil);
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
				unsigned int line_nr(void);
				unsigned int line_nr(void);

unsigned int TextRBTree::lines_before(const TextRBTree::Node *x) const
{
	const TextRBTree::Node *y;
	unsigned int r;

	assert(x->tree == this);

	r = x->left->lines + 1;
	y = x;

	while (y != root) {
		if (y == y->parent->right)
			r += y->parent->left->lines + 1;

		y = y->parent;
	}

	return r;
}

TextRBTree::Node::Node(TextRBTree *tree, Node *parent)
: tree(tree)
, color(BLACK)
, parent(parent)
, left(NULL)
, right(NULL)
, pred(NULL)
, succ(NULL)
{
}

TextRBTree::Node::Node(TextRBTree *tree, Node *parent, const TextLine &line)
: tree(tree)
, color(BLACK)
, parent(parent)
, left(NULL)
, right(NULL)
, pred(NULL)
, succ(NULL)
{
}

TextRBTree::Node::~Node()
{
	/* Recursively delete */
	if (left != tree->nil)
		delete left;
	if (right != tree->nil)
		delete right;
}

unsigned int TextRBTree::Node::line_nr(void) const
{
	assert(tree != NULL);

	return tree->lines_before(this);
}

TextRBTree::iterator_base::iterator_base(void)
: tree(NULL)
, node(NULL)
, byte_offset(0)
, char_offset(0)
{
}

TextRBTree::iterator_base::iterator_base(TextRBTree &_tree)
: tree(&_tree)
, node(NULL)
, byte_offset(0)
, char_offset(0)
{
	node = _tree.tree_minimum(_tree.root);
}

TextRBTree::iterator_base::iterator_base(Node &node)
: tree(node.tree)
, node(NULL)
, byte_offset(0)
, char_offset(0)
{
	this->node = tree->tree_minimum(node.tree->root);
	byte_offset = ;
	char_offset = ;
	col_offset = ;
}

TextRBTree::iterator_base::iterator_base(const iterator_base &iter)
{
	*this = iter;
}

bool TextRBTree::iterator_base::valid(void)
{
	return tree != NULL;
}

bool TextRBTree::iterator_base::char_valid(void)
{
	return line_iter.char_valid();
}

/* Determine the number of bytes/columns used
* by the character under the iterator. */
unsigned int TextRBTree::iterator_base::char_bytes(void)
{
	return line_iter.char_bytes();
}

unsigned int TextRBTree::iterator_base::char_columns(void)
{
	return line_iter.char_columns();
}

TextLine& TextRBTree::iterator_base::operator*()
{
	return node->line;
}

TextLine* TextRBTree::iterator_base::operator->()
{
	return &node->line;
}

TextRBTree::iterator_base& TextRBTree::iterator_base::operator=(const TextRBTree::iterator_base& iter)
{
	if (this == &iter) return *this;

	node = iter.node;
	tree = iter.tree;
	byte_offset = iter.byte_offset;
	char_offset = iter.char_offset;

	return *this;
}

bool TextRBTree::iterator_base::operator<(const TextRBTree::iterator_base& iter)
{
	return this->char_offset < iter.char_offset;
}

TextRBTree::iterator_base& TextRBTree::iterator_base::forward_lines(unsigned int n)
{
	while ( (node != tree->nil) && (n > 0) )
	{ 
		node = node->succ;
		--n;
	}

	return *this;
}

TextRBTree::iterator_base& TextRBTree::iterator_base::backward_lines(unsigned int n)
{
	while ( (node != tree->nil) && (n > 0) )
	{
		node = node->pred;
		--n;
	}

	return *this;
}

/*
TextRBTree::iterator_base& TextRBTree::iterator_base::forward_lines(WrapMode wrap_mode, unsigned int n)
{
	assert(node != tree->nil);

	switch (tree->wrap()) {
		case WRAP_NONE:
		case WRAP_CHAR:
		case WRAP_WORD:
		case WRAP_WORD_CHAR:
			break;
	}

	*TODO implement other forward line thingies *
	return forward_lines(n);
}

TextRBTree::iterator_base& TextRBTree::iterator_base::backward_lines(WrapMode wrap_mode, unsigned int n)
{ 
	assert(node != tree->nil);

	switch (wrap_mode) {
		case WRAP_NONE:
		case WRAP_CHAR:
		case WRAP_WORD:
		case WRAP_WORD_CHAR:
			break;
	}

	*TODO implement other forward line thingies *
	return backward_lines(n);
}*/

TextRBTree::iterator_base& TextRBTree::iterator_base::forward_bytes(unsigned int n)
{
	assert(tree != NULL);

	if (node == nil)
		return *this;

	while ( (node != tree->nil) && ((byte_offset + n) > node->line.bytes()) ) {
		n -= node->line.bytes() - byte_offset;
		node = node->succ;
		byte_offset = 0;
	}

	if (node != tree->nil) {
		byte_offset = n;
		char_offset = node->line.char_count_to_byte_offset(byte_offset);
	} else {
		byte_offset = 0;
		char_offset = 0;
	}

	return *this;
}

TextRBTree::iterator_base& TextRBTree::iterator_base::backward_bytes(unsigned int n)
{
	assert(tree != NULL);

	if (node == nil)
		return *this;

	while ( (node != tree->nil) && (byte_offset < n) ) {
		n -= byte_offset;
		node = node->pred;
		byte_offset = node->line.bytes();
	}

	if (node != tree->nil) {
		byte_offset = node->line.bytes() - n;
		char_offset = node->line.char_count_to_byte_offset(byte_offset);
	} else {
		byte_offset = 0;
		char_offset = 0;
	}

	return *this;
}

TextRBTree::iterator_base& TextRBTree::iterator_base::forward_chars(unsigned int n)
{
	assert(tree != NULL);

	if (node == nil)
		return *this;

	while ( (node != tree->nil) && ((char_offset + n) > node->line.chars()) ) {
		n -= (node->line.chars() - char_offset);
		node = node->succ;
		char_offset = 0;
	}

	if (node != tree->nil) {
		char_offset = n;
		byte_offset = node->line.byte_count_to_char_offset(char_offset);
		//byte_offset = g_utf8_offset_to_pointer(node->line.c_str(), n) - node->line.c_str();
	} else {
		byte_offset = 0;
		char_offset = 0;
	}

	return *this;
}

TextRBTree::iterator_base& TextRBTree::iterator_base::backward_chars(unsigned int n)
{
	assert(tree != NULL);

	if (node == nil)
		return *this;

	while ( (node != tree->nil) && (char_offset < n) ) {
		n -= char_offset;
		node = node->pred;
		char_offset = node->line.chars();
	}

	if (node != tree->nil) {
		char_offset = node->line.chars() - n;
		byte_offset = node->line.byte_count_to_char_offset(char_offset);
	} else {
		byte_offset = 0;
		char_offset = 0;
	}

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
	return (other.node == this->node);
}

bool TextRBTree::line_iterator::operator!=(const line_iterator &other) const
{
	return (other.node != this->node);
}

TextRBTree::line_iterator& TextRBTree::line_iterator::operator++(void)
{
	forward_lines(1);
	return *this;
}

TextRBTree::line_iterator& TextRBTree::line_iterator::operator--(void)
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
	return ( (other.node == this->node) && (other.char_offset == this->char_offset) );
}

bool TextRBTree::char_iterator::operator!=(const char_iterator &other) const
{
	return ( (other.node != this->node) || (other.char_offset != this->char_offset) );
}

TextRBTree::char_iterator& TextRBTree::char_iterator::operator++(void)
{
	forward_chars(1);
	return *this;
}

TextRBTree::char_iterator& TextRBTree::char_iterator::operator--(void)
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
