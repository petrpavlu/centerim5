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

#ifndef __TEXTRBTREE_H__
#define __TEXTRBTREE_H__

#include "config.h"

#include "CppConsUI.h"

#include "TextLine.h"

class TextRBTree
{
	private:
		class Node;

	public:
		class iterator_base;
		class line_iterator;
		class char_iterator;

		TextRBTree();
		~TextRBTree();

		char_iterator insert(const char_iterator& iter, const TextLine& line);

		char_iterator erase(char_iterator pos);
		line_iterator erase(line_iterator pos);
		char_iterator erase(char_iterator start, char_iterator end);
		line_iterator erase(line_iterator start, line_iterator end);

		void print(void);

		char_iterator begin(void) const;
		char_iterator back(void) const;
		char_iterator end(void) const;

		char_iterator reverse_begin(void) const;
		char_iterator reverse_back(void) const;
		char_iterator reverse_end(void) const;

		/* Iterator for the tree. */
		class iterator_base
		{
			public:
				iterator_base(void);
				iterator_base(TextRBTree &tree);
				iterator_base(Node &node);
				iterator_base(const iterator_base &iter);

				bool valid(void);

				/* Is the byte pointed at by the cursor a valid character? */
				bool char_valid(void);

				/* Determine the number of bytes/columns used
				 * by the character under the iterator. */
				unsigned int char_bytes(void);
				unsigned int char_columns(void);

				TextLine& operator*() const;
				TextLine* operator->() const;

				iterator_base& operator=(const iterator_base&);
				bool operator<(const iterator_base&);

				TextRBTree *tree;
				Node *node;

				unsigned int byte_offset;
				unsigned int char_offset;

				iterator_base& forward_lines(unsigned int);
				iterator_base& backward_lines(unsigned int);

				iterator_base& forward_lines(WrapMode, unsigned int width, unsigned int n);
				iterator_base& backward_lines(WrapMode, unsigned int width, unsigned int n);

				iterator_base& forward_bytes(unsigned int);
				iterator_base& backward_bytes(unsigned int);

				iterator_base& forward_chars(unsigned int);
				iterator_base& backward_chars(unsigned int);

				iterator_base& forward_columns(unsigned int);
				iterator_base& backward_columns(unsigned int);

			protected:

			private:
		};

		class char_iterator
		: public iterator_base
		{
			public:
				char_iterator(void);
				char_iterator(TextRBTree &tree);
				char_iterator(Node &node);
				char_iterator(const iterator_base &iter);

				char*& operator*() const;

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

		class line_iterator
		: public iterator_base
		{
			public:
				line_iterator(void);
				line_iterator(TextRBTree &tree);
				line_iterator(Node &node);
				line_iterator(const iterator_base &iter);

				//TODO one of the following two is superfluos?
				line_iterator& operator=(const line_iterator&);
				line_iterator& operator=(const iterator_base&);
				bool operator==(const line_iterator&) const;
				bool operator!=(const line_iterator&) const;
				line_iterator& operator++();
				line_iterator& operator--();
				line_iterator operator++(int);
				line_iterator operator--(int);
				line_iterator& operator+=(unsigned int);
				line_iterator& operator-=(unsigned int);

			protected:

			private:
		};

	protected:

	private:
		class Node;

		TextRBTree(const TextRBTree &);

		TextRBTree& operator=(const TextRBTree&);

		void rotate_left(Node *x);
		void rotate_right(Node *x);
		void insert_fixup(Node *z);
		void erase_node_fixup(Node *x);

		void post_rotate_augmentation_fixup(Node *x, Node *y);
		void post_insert_augmentation_fixup(Node *z);

		Node* tree_minimum(Node *node) const;
		Node* tree_maximum(Node *node) const;

		unsigned int lines_before(const Node *node) const;

		Node* successor(Node *node) const;
		Node* predecessor(Node *node) const;

		char_iterator insert(Node *z, int line_nr);
		line_iterator erase(Node *z);

		void print_node(Node *node);

		Node *root; /* The root of the tree. */
		Node *nil; /* The dummy node. */

		enum Color {RED, BLACK};

		class Node
		{
			public:
				Node(TextRBTree *tree, Node *parent);
				Node(TextRBTree *tree, Node *parent, const TextLine& line);
				~Node();

				unsigned int line_nr(void) const;

				TextRBTree *tree; /* Pointer to the tree this node belongs to */

				Color color; /* Color of the node, RED or BLACK. */

				Node *parent, *left, *right; /* Pointers to the parent and  left/right child */
				Node *pred, *succ; /* Pointers to the predecessor nodes and successor nodes */

				int lines; /* Number of lines in this subtree including this one. */
				int lines_wrap; /* Number of lines in this subtree including this one
			       			 * taking account of the current wrapping mode and maximum
						 * display size. */

				TextLine line;

			protected:

			private:
				Node();

				Node(const Node &);

				Node& operator=(const Node&);
		};
};

#endif /* __TEXTRBTREE_H__ */
