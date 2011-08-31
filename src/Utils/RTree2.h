/* 
 * Copyright (c) 2011, Laminar Research.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a 
 * copy of this software and associated documentation files (the "Software"), 
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense, 
 * and/or sell copies of the Software, and to permit persons to whom the 
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN 
 * THE SOFTWARE.
 *
 */

#ifndef RTree2_H
#define RTree2_H

#include "CompGeomDefs2.h"

// Quick aside:
// A tagged ptr is a ptr + a boolean flag - because ptrs are always 4-byte aligned for dynamic memory.
// So we can use the low bit as a flag.  This typically saves 4 bytes, since the bool var in a struct
// would need to be at least 1 byte and would then be padded by 3-bytes for alignment.
// We use this to note leaf vs intermediate nodes without wasting storage.

const uintptr_t	lsb_bit = 1;
const uintptr_t  lsb_mask = ~1;

template <typename T>	T * _set_lsb(T * v)		{	return reinterpret_cast<T*>(reinterpret_cast<uintptr_t>(v) | lsb_bit);}
template <typename T>	T * _clear_lsb(T * v)	{	return reinterpret_cast<T*>(reinterpret_cast<uintptr_t>(v) & lsb_mask);}
template <typename T>	T * _clean_ptr(T * v)	{	return reinterpret_cast<T*>(reinterpret_cast<uintptr_t>(v) & lsb_mask);}
template <typename T>	bool _get_lsb(T * v)	{	return reinterpret_cast<T*>(reinterpret_cast<uintptr_t>(v) & lsb_bit);}

// RTree2 - a 2-d range tree templated on a value, using Bbox2 as a key.  N is the number of items per leaf node.
// Note that this is NOT an incremental range tree - it must be cleared and re-filled.  Insert works by dividing
// the range alternately among X and Y and forming equal splits.
//
// Since the value type stored may not contain a cache of its AABB and the AABB may be expensive, this class 
// caches AABB/value pairs ("items").  Insert chews a pile of storage since it needs a working buffer to sort.

template <typename T, int N>
class RTree2 {
public:

	typedef	T							value_type;
	typedef Bbox2						key_type;
	typedef	pair<key_type,value_type>	item_type;

	RTree2() : root(NULL) { }
	~RTree2() { if(root) delete root; }

	template <typename I>
	void	insert(I begin, I end);

	void	clear() { if(root) delete root; root = NULL; }
	
	template <typename O>
	void	query(const Bbox2& where, O out);						// This results item_type ptrs.
	template <typename O>
	void	query_value(const Bbox2& where, O out);					// This returns only the value, not the key.
	
private:
	struct node;
	struct leaf;

	template <typename O>
	void	query_recursive(node * node, const Bbox2& where, O out);
	template <typename O>
	void	query_value_recursive(node * node, const Bbox2& where, O out);
	
	node *	insert_range(int level, typename vector<item_type>::iterator begin, typename  vector<item_type>::iterator end);

	struct	item_compare_x {
		bool operator()(const item_type& lhs, const item_type& rhs) const {
			lesser_x_then_y cmp;
			return cmp(lhs.first.centroid(),rhs.first.centroid());
		};
	};
	struct	item_compare_y {
		bool operator()(const item_type& lhs, const item_type& rhs) const {
			lesser_y_then_x cmp;
			return cmp(lhs.first.centroid(),rhs.first.centroid());
		};
	};
	
	// Implementation: nodes and leafs both contain their AABB first; ptrs to leafs are stored by flagging the LSB.
	// All nodes contain both children.  The root must not be a leaf.  Therefore it is always safe to assume
	// root is a node and that it has a left and right child of SOME type.
	
	struct leaf {
		Bbox2		bounds;
		int			count;
		item_type	items[N];
	};
	

	struct node {
		Bbox2		bounds;
	
		union {
			node *		left_n;			// tagged ptr  - flag means leaf!
			leaf *		left_l;
		};
		union {
			node *		right_n;
			leaf *		right_l;
		};
		
		// Accessors simplify access to overloaded ptrs.
		bool		left_is_leaf() const { return _get_lsb(left_n); }
		bool		right_is_leaf() const { return _get_lsb(right_n); }
		node *		left_as_node() const { return _clean_ptr(left_n); }
		node *		right_as_node() const { return _clean_ptr(right_n); } 
		leaf *		left_as_leaf() const { return _clean_ptr(left_l); }
		leaf *		right_as_leaf() const { return _clean_ptr(right_l); }
		
		void		set_left_node(node * n) { left_n = n; }
		void		set_right_node(node * n) { right_n = n; }
		void		set_left_leaf(leaf * l) { left_l = _set_lsb(l); }
		void		set_right_leaf(leaf * r) { right_l = _set_lsb(r); }
		
		~node() {	if(left_is_leaf()) delete left_as_leaf(); else delete left_as_node();
					if(right_is_leaf()) delete right_as_leaf(); else delete right_as_node(); }
	};
	
	node *		root;

};

/************************************************************************************************************
 *
 ************************************************************************************************************/

template<typename T, int N>
template <typename O>
void	RTree2<T,N>::query(const Bbox2& where, O out)
{
	if(root && where.overlap(root->bounds))
		query_recursive(root, where,out);
}

template<typename T, int N>
template <typename O>
void	RTree2<T,N>::query_value(const Bbox2& where, O out)
{
	if(root && where.overlap(root->bounds))
		query_value_recursive(root, where,out);
}

template<typename T, int N>
template <typename O>
void	RTree2<T,N>::query_recursive(node * node, const Bbox2& where, O out)
{
	if(where.overlap(node->left_as_leaf()->bounds))
	{
		if(node->left_is_leaf())
		{
			leaf * l = node->left_as_leaf();
			for(int n = 0; n < l->count; ++n)
			if(where.overlap(l->items[n].first))
			{
				*out = l->items[n];
				++out;
			}
		}
		else
			query_recursive(node->left_as_node(),where, out);
	}
	if(where.overlap(node->right_as_leaf()->bounds))
	{
		if(node->right_is_leaf())
		{
			leaf * l = node->right_as_leaf();
			for(int n = 0; n < l->count; ++n)
			if(where.overlap(l->items[n].first))
			{
				*out = l->items[n];
				++out;
			}
		}
		else
			query_recursive(node->right_as_node(),where, out);
	}
}

template<typename T, int N>
template <typename O>
void	RTree2<T,N>::query_value_recursive(node * node, const Bbox2& where, O out)
{
	if(where.overlap(node->left_as_leaf()->bounds))
	{
		if(node->left_is_leaf())
		{
			leaf * l = node->left_as_leaf();
			for(int n = 0; n < l->count; ++n)
			if(where.overlap(l->items[n].first))
			{
				*out = l->items[n].second;
				++out;
			}
		}
		else
			query_value_recursive(node->left_as_node(),where, out);
	}
	if(where.overlap(node->right_as_leaf()->bounds))
	{
		if(node->right_is_leaf())
		{
			leaf * l = node->right_as_leaf();
			for(int n = 0; n < l->count; ++n)
			if(where.overlap(l->items[n].first))
			{
				*out = l->items[n].second;
				++out;
			}
		}
		else
			query_value_recursive(node->right_as_node(),where, out);
	}
}

template<typename T, int N>
template <typename I>
void	RTree2<T,N>::insert(I begin, I end)
{
	if(root) delete root;
	if(begin == end) 
		root = NULL;
	else {
		vector<item_type>	container(begin,end);
		root = insert_range(0, begin, end);
	}
}

template<typename T, int N>
typename RTree2<T,N>::node *	RTree2<T,N>::insert_range(int level, typename vector<item_type>::iterator begin, typename vector<item_type>::iterator end)
{	
	if(level > 0 && distance(begin,end) <= N)
	{
		leaf * l = new leaf;
		l->count = 0;
		for(typename vector<item_type>::iterator i = begin; i != end; ++i)
		{
			l->items[l->count++] = *i;
			l->bounds += i->first;
		}
		return _set_lsb(reinterpret_cast<node*>(l));
	}
	else
	{
		if(level % 2)
		{
			item_compare_x cmp;
			sort(begin,end,cmp);			
		} 
		else
		{
			item_compare_y cmp;
			sort(begin,end,cmp);
		}
		typename vector<item_type>::iterator k(begin);
		advance(k,distance(begin,end)/2);
		node * me = new node;
		me->set_left_node(insert_range(level+1,begin,k));
		me->set_right_node(insert_range(level+1,k,end));
		me->bounds = me->left_as_leaf()->bounds;
		me->bounds += me->right_as_leaf()->bounds;
		return me;
	}
}


#endif /* RTree2_H */
