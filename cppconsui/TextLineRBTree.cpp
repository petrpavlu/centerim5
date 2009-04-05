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

TextLineRBTree::TextLineRBTree()
{
	nil = new Node(this, NULL);
	root = nil;

	/* Set all pointers of the nil node to nil. The nil node is fake. */
	nil->parent = nil;
	nil->left = nil;
	nil->right = nil;
	nil->pred = nil;
	nil->succ = nil;

	/* Set the data to null. */
	nil->str = NULL;
	nil->str_len = 0;
	nil->str_chars = 0;
	nil->str_cols = 0;

	/* Set all order statistic data of nil to 0 (or the unit element.) */
	nil->bytes = 0;
	nil->chars = 0;
	nil->cols = 0;
}

TextLineRBTree::~TextLineRBTree()
{
	if (root != nil)
		delete root; /* Deletes nodes recursively. */

	delete nil;
}

TextLineRBTree::char_iterator TextLineRBTree::insert(const TextLineRBTree::char_iterator& iter, const char* str, unsigned int len)
{

	if (str == NULL) {
		node.str = g_strndup(str, len)
	} else {
		char* s;

		if (byte_offset = 0) {
			s = g_strconcat(str, node.str);
		} else if (byte_offset > node.str_len) {
			s = g_strconcat(node.str, str);
		} else {
			s = g_new0(node.str_len + len + 1);
			g_strlcpy(s, node.str, byte_offset);
			g_strlcpy(s + iter.byte_offset + 1, str, len);
			g_strlcpy(s + iter.byte_offset + len + 1, node.str_len + 1 - byte_offset);
		}

		g_free(node.str);
		node.str = s;
	}
}

TextLineRBTree::char_iterator TextLineRBTree::erase(const TextLineRBTree::char_iterator pos)
{
	char_iterator iter = pos;
	erase(iter, ++iter);
}

TextLineRBTree::char_iterator erase(TextLineRBTree::char_iterator start, TextLineRBTree::char_iterator end)
{
	char* s;

	assert(start.node == end.node);

	if (start.byte_offset == end.byte_offset)
		return;

	if (node.str == NULL)
		return;

	s = g_new0(start.byte_offset + node.str_len - end.byte_offset);
	g_strlcpy(s, node.str, start.byte_offset);
	g_strlcpy(s + start.byte_offset + 1, node.str + end.byte_offset, node.str_len - start.byte_offset);

	g_free(node.str);
	node.str = s;
}

TextLineRBTree::char_iterator TextLineRBTree::begin(void) const
{
	return char_iterator(*tree_minimum(root));
}

TextLineRBTree::char_iterator TextLineRBTree::back(void) const
{
	TextLineRBTree::char_iterator iter(*tree_maximum(root));
	iter.forward_bytes(iter.node.str_len);
	return iter;
}

TextLineRBTree::char_iterator TextLineRBTree::end(void) const
{
	return char_iterator(*nil);
}

TextLineRBTree::char_iterator TextLineRBTree::reverse_begin(void) const
{
	return char_iterator(*tree_maximum(root));
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

TextLineRBTree::char_iterator TextLineRBTree::get_iter_at_char_offset(unsigned int index) const
{
	Node* node = root;	/* Current node we are looking at. */
	unsigned int r;		/* Number of chars in de left subtree of node and the node itself. */
	unsigned int bytes;	/* byte_offset to the current node. */
	
	/* If the index i falls outside the range of the tree, return
	 * the appropriate iterator. */
	if (index == 0) {
		return front();
	} else if (index > root.chars) {
		return end();
	}

	/* Initialisation for the while loop. */
	r = root.left.chars + root.str_chars;
	i = index;
	bytes = 0;

	/* Find the node where the i'th character is stored. */
	while ( !(i > root.left.chars && i < r) ) {
		if (i < r) { /* Turn left. */
			node = node.left;
		} else { /* Turn right. */
			i -= r;
			bytes += root.left.bytes + root.str_bytes;
			node = node.right;
		}
	}

	/* Now we have found the node, create the iterator. */
	char_iterator iter;

	/* Find the byte offset of the i'th character in node.str. */
	bytes += (g_utf8_offset_to_pointer(node.str, i) - node.str)

	iter.node = node;
	iter.tree = this;
	iter.byte_offset = ;
	iter.char_offset = index;
	iter.col_offset = ;

	return iter;
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

TextLineRBTree::iterator_base::iterator_base(const Node &node_)
: tree(node.tree)
, node(NULL)
, byte_offset(0)
, char_offset(0)
, col_offset(0)
{
	node = tree->tree_minimum(node_.tree->root);
	byte_offset = ;
	char_offset = ;
	col_offset = ;
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
	return g_unichar_validate( g_utf8_get_char(node.str + byte_offset) );
}

unsigned int TextLineRBTree::iterator_base::char_bytes(void)
{
}

unsigned int TextLineRBTree::iterator_base::char_cols(void)
{
	gunichar c;

	c = g_utf8_get_char(node.str + byte_offset);

	return (g_unichar_iswide(c) ? 2 : 1);
}

unsigned int TextLineRBTree::iterator_base::chars(void) const
{
	return node.chars;
}

unsigned int TextLineRBTree::iterator_base::bytes(void) const
{
	return node.bytes;
}

unsigned int TextLineRBTree::iterator_base::cols(void) const
{
	return node.cols;
}

char*& TextLineRBTree::iterator_base::operator*() const
{
	return node.str;
}

iterator_base& TextLineRBTree::iterator_base::operator=(const iterator_base& iter)
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
	return this->byte_offset < iter.byte_offset;
}

TextLineRBTree::iterator_base& TextLineRBTree::iterator_base::forward_bytes(unsigned int n)
{
	assert(tree != NULL);

	if (node == tree->nil)
		return *this;

	while ( (node != tree->nil) && (n > node->str_bytes) ) {
		n -= node->str_bytes - byte_offset;
		node = node->succ;
		byte_offset = 0;
	}

	if (node != tree->nil) {
		byte_offset = n;
		char_offset = g_utf8_pointer_to_offset(node->str, node->str + n);
		col_offset = ;
	} else {
		*this = nil;
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
		byte_offset = node->str_bytes();
	}

	if (node != tree->nil) {
		byte_offset = node->str_bytes - n;
		char_offset = g_utf8_pointer_to_offset(node->str, node->str + n);
		col_offset = ;
	} else {
		*this = nil;
	}

	return *this;
}

TextLineRBTree::iterator_base& TextLineRBTree::iterator_base::forward_chars(unsigned int n)
{
	assert(tree != NULL);

	if (node == tree->nil)
		return *this;

	while ( (node != tree->nil) && (n > node->str_chars) ) {
		n -= node->str_chars - char_offset;
		node = node->succ;
		char_offset = 0;
	}

	if (node != tree->nil) {
		byte_offset = ;
		char_offset = n;
		col_offset = ;
	} else {
		*this = nil;
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
		char_offset = node->str_chars();
	}

	if (node != tree->nil) {
		byte_offset = ;
		char_offset = node->str_chars - n;
		col_offset = ;
	} else {
		*this = nil;
	}

	return *this;
}

TextLineRBTree::iterator_base& TextLineRBTree::iterator_base::forward_cols(unsigned int n)
{
	assert(tree != NULL);

	if (node == tree->nil)
		return *this;

	while ( (node != tree->nil) && (n > node->str_cols) ) {
		n -= node->str_cols - col_offset;
		node = node->succ;
		col_offset = 0;
	}

	if (node != tree->nil) {
		byte_offset = ;
		char_offset = ;
		col_offset = n;
	} else {
		*this = nil;
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
		col_offset = node->str_colss();
	}

	if (node != tree->nil) {
		byte_offset = ;
		char_offset = ;
		col_offset = node->str_cols - n;
	} else {
		*this = nil;
	}

	return *this;
}

		class char_iterator
		: public iterator_base
		{
			public:
				char_iterator(void);
				char_iterator(TextLineRBTree &tree);
				char_iterator(Node &node);
				char_iterator(const iterator_base &iter);

				char_iterator& operator=(const char_iterator&);
				char_iterator& operator=(const iterator_base&);
				bool operator==(const char_iterator&) const;
				bool operator!=(const char_iterator&) const;
				char_iterator& operator++();
				char_iterator& operator--();
				char_iterator operator++(int);
				char_iterator operator--(int);
				char_iterator& operator+=(unsigned int);
				char_iterator& operator-=(unsigned int);

			protected:

			private:
		};

		class column_iterator
		: public iterator_base
		{
			public:
				column_iterator(void);
				column_iterator(TextLineRBTree &tree);
				column_iterator(Node &node);
				column_iterator(const iterator_base &iter);

				column_iterator& operator=(const column_iterator&);
				column_iterator& operator=(const iterator_base&);
				bool operator==(const column_iterator&) const;
				bool operator!=(const column_iterator&) const;
				column_iterator& operator++();
				column_iterator& operator--();
				column_iterator operator++(int);
				column_iterator operator--(int);
				column_iterator& operator+=(unsigned int);
				column_iterator& operator-=(unsigned int);
			
			protected:

			private:
		};

	protected:

	private:
		class Node;

		TextLineRBTree(const TextLineRBTree &);

		TextLineRBTree& operator=(const TextLineRBTree&);

		/* Function to support RedBlackTree operations. */
		void rotate_left(Node *x);
		void rotate_right(Node *x);
		void insert_fixup(Node *z);
		void erase_node_fixup(Node *x);

		Node* tree_minimum(Node *node) const;
		Node* tree_maximum(Node *node) const;

		/* Functions specific to this augmented RBTree. */
		void post_rotate_augmentation_fixup(Node *x, Node *y);
		void post_insert_augmentation_fixup(Node *z);

		/* This RBTree implementation also maintains
		 * predecessor/successor pointers. This allows
		 * fast iterator operations. */
		Node* successor(Node *node) const;
		Node* predecessor(Node *node) const;

		/**/
		char_iterator insert(Node *z, int line_nr);
		column_iterator erase(Node *z);

		//TODO only if debug build
		void print(void);
		void print_node(Node *node);

		/* Data we store for the entire RBTree. */
		Node *root; /* The root of the tree. */
		Node *nil; /* The dummy node. */

		enum Color {RED, BLACK};

		/* Node type. */
		class Node
		{
			public:
				Node(TextLineRBTree *tree, Node *parent);
				Node(TextLineRBTree *tree, Node *parent, const char *str);
				~Node();

				TextLineRBTree *tree; /* Pointer to the tree this node belongs to.
			       				 Used for sanity checks. */

				Color color; /* Color of the node, RED or BLACK. */

				Node *parent, *left, *right; /* Pointers to the parent and  left/right child */
				Node *pred, *succ; /* Pointers to the predecessor nodes and successor nodes */

				unsigned int bytes; /* Number of bytes stored in this node. */
				unsigned int chars; /* Number of characters. */
				unsigned int cols; /* Number of terminal columns needed to display the text */

				char *str; /* The data we store in a node. */
				
				//TODO text attributes

			protected:

			private:
				Node();

				Node(const Node &);

				Node& operator=(const Node&);
		};
};

#endif /* __TEXTLINERBTREE_H__ */
