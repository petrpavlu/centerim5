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

#ifndef __TEXTLINERBTREE_H__
#define __TEXTLINERBTREE_H__

#include "config.h"

#include "CppConsUI.h"

class TextLineRBTree
{
	private:
		class Node;

	public:
		class iterator_base;
		class char_iterator;
		class column_iterator;

		TextLineRBTree();
		~TextLineRBTree();

		TextLineRBTree& operator=(const TextLineRBTree&);

		//TODO standard string insert methods, same for erase and such
		char_iterator insert(const char_iterator& iter, const char* str, unsigned int len);

		/* We can only erase characters, not columns. */
		void erase(char_iterator pos);
		void erase(char_iterator start, char_iterator end);

		/* Obtaining iterators for the string. */
		char_iterator begin(void) const;
		char_iterator back(void) const;
		char_iterator end(void) const;

		char_iterator reverse_begin(void) const;
		char_iterator reverse_back(void) const;
		char_iterator reverse_end(void) const;

		char_iterator get_iter_at_char_offset(unsigned int index);

		/* Iterator for the tree. */
		class iterator_base
		{
			public:
				iterator_base(void);
				iterator_base(TextLineRBTree &tree);
				iterator_base(Node &node);
				iterator_base(const iterator_base &iter);

				bool valid(void) const;
				bool valid_char(void) const;

				unsigned int byte_index(void) const;

				/* Determine the number of bytes/columns used
				 * by the character under the iterator. */
				unsigned int char_bytes(void);
				unsigned int char_cols(void);

				/* Determine the total number of chars/bytes/columns
				 * stored in the node pointed at by the iterator
				 * including children. lines() returns the number
				 * of lines on screen when formatted. */
				unsigned int chars(void) const;
				unsigned int bytes(void) const;
				unsigned int cols(void) const;
				unsigned int lines(void) const;

				char* operator*() const;
				/* This operator omitted to prevent confusion. */
				//char** operator->() const;

				iterator_base& operator=(const iterator_base&);
				bool operator<(const iterator_base&);

				TextLineRBTree *tree;
				Node *node;

				/* These are the iterator location measured in bytes/chars/cols
				 * from the node.str. */
				unsigned int byte_offset;
				unsigned int char_offset;
				unsigned int col_offset;

				iterator_base& forward_bytes(unsigned int);
				iterator_base& backward_bytes(unsigned int);

				iterator_base& forward_chars(unsigned int);
				iterator_base& backward_chars(unsigned int);
				
				iterator_base& forward_cols(unsigned int);
				iterator_base& backward_cols(unsigned int);

			protected:

			private:
		};

		class byte_iterator
		: public iterator_base
		{
			public:
				byte_iterator(void);
				byte_iterator(TextLineRBTree &tree);
				byte_iterator(Node &node);
				byte_iterator(const iterator_base &iter);

				//TODO one of the following two is superfluos?
				byte_iterator& operator=(const byte_iterator&);
				byte_iterator& operator=(const iterator_base&);
				bool operator==(const byte_iterator&) const;
				bool operator!=(const byte_iterator&) const;
				byte_iterator& operator++();
				byte_iterator& operator--();
				byte_iterator operator++(int);
				byte_iterator operator--(int);
				byte_iterator& operator+=(unsigned int);
				byte_iterator& operator-=(unsigned int);
			
			protected:

			private:
		};


		class char_iterator
		: public iterator_base
		{
			public:
				char_iterator(void);
				char_iterator(TextLineRBTree &tree);
				char_iterator(Node &node);
				char_iterator(const iterator_base &iter);

				//TODO one of the following two is superfluos?
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

		class col_iterator
		: public iterator_base
		{
			public:
				col_iterator(void);
				col_iterator(TextLineRBTree &tree);
				col_iterator(Node &node);
				col_iterator(const iterator_base &iter);

				//TODO one of the following two is superfluos?
				col_iterator& operator=(const col_iterator&);
				col_iterator& operator=(const iterator_base&);
				bool operator==(const col_iterator&) const;
				bool operator!=(const col_iterator&) const;
				col_iterator& operator++();
				col_iterator& operator--();
				col_iterator operator++(int);
				col_iterator operator--(int);
				col_iterator& operator+=(unsigned int);
				col_iterator& operator-=(unsigned int);
			
			protected:

			private:
		};

	protected:

	private:
		class Node;

		TextLineRBTree(const TextLineRBTree &);

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
		void post_erase_augmentation_fixup(Node *z);
		void post_update_augmentation_fixup(Node *z);

		/* This RBTree implementation also maintains
		 * predecessor/successor pointers. This allows
		 * fast iterator operations. */
		Node* successor(Node *node) const;
		Node* predecessor(Node *node) const;

		/**/
		char_iterator insert(Node *z, int line_nr);
		void erase(Node *z);

		Node* MakeNil(void);

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

				unsigned int bytes; /* Number of bytes stored in the substree rooted at this node. */
				unsigned int chars; /* Number of characters... */
				unsigned int cols; /* Number of terminal columns needed to display the text... */

				char *str; /* The data we store in a node. */
				unsigned int str_bytes;
				unsigned int str_chars;
				unsigned int str_cols;
				
				//TODO text attributes

			protected:

			private:
				Node();

				Node(const Node &);

				Node& operator=(const Node&);
		};
};

#endif /* __TEXTLINERBTREE_H__ */
