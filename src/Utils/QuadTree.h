/*
 * Copyright (c) 2007, Laminar Research.
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

#ifndef QUADTREE_H
#define QUADTREE_H

/*
		SUBDIVISION

		Each node contains N subnodes defined by a template.  The tree works in any dimensional space as
		long as the space is fully partitioned by the N subnodes.  The goal of making N subnodes flexible
		is actually to allow us to put extra buckets across the "cracks" in the quad-split.  This keeps
		us from picking up objects at the half-points of the area on every cull.

		INVARIANCE ON KEYS AND CULLING

		A key represents a subdividable area of the quad-tree in N dimensions.
		The cull represents a union-able area that is fully contained within the key.

		The actual cull values (and their unions) are used for iteration, but keys are used for filng.  The
		idea here is that we use a "first-fit" drop-in alg for keys, so we start with the whole-tree extent
		and subdivide when filing an item.  But when we cull, we have access to the REAL minimum spans of the
		items, which might be a lot smaller than the max legal size of a node.

		So the important relationship is that "make_key" always returns a key that fully contains the culling
		area, and expand_by yields a cull area that fully contains the passed in part.

		REMOVAL

		Removal is done by entity - we simply do a first-fit depth-first search for our bucket.  When we remove
		an item, we need to find its bucket to recompute parent culling volumes.  So the cost of going down
		the tree (log-N anyway) is worth it.

		MEMORY MANAGEMENT AND INTRINSIC LISTS

		Items in a given node are linked using an intrinsic list - that is, the value type must have a public member "next" that points to the same type as it is.
		This is done to save on small allocations - however it also means that a given entity can only be in
		one spatial index!
		Please note that when a visitor gets an item,
		it gets the first of a list that IT is expected to crawl!

		Nodes are dynamically allocated and are NOT freed - the traits must release them.  However, the class
		attempts to heavily reuse nodes when possible.

		(The goal of this is to let traits allocate nodes out of
		a big pool and then blow the entire pool away without individually deleting the blocks.  When the whole
		structure is destroyed at once and the max me use tends to be near the sustained mem use, this lets us
		write an allocator that is (1) really fast and (2) doesn't touch all that mem on deallocate, which is
		nice to the VM system.)

*/

#if BENTODO
bulk remove?
put in some assertions, etc.
#endif

/*
	struct Traits {

		typedef	Bbox2		KeyType;										// Key type - used for subdivision
		typedef Bbox2		CullType;										// Culling type - for iteration
		typedef Whatever	ValueType;

		void		get_cull(ValueType * v, CullType& c);					// get cull radius of V
		ValueType *	get_next(ValueType * v);								// get next item in chain after V
		void		set_next(ValueType * v, ValueType * n);					// Set N to be after V

		void		expand_by(CullType& io_cull, const CullType& part);		// expand io_cull by part
		void		set_empty(CullType& c);									// make zero-area cull

		void		subkey(const KeyType& e, KeyType& k, int n);			// change K to be the Nth subsection of E
		bool		contains(const KeyType& outer, const KeyType& inner);	// return true if outer fully contains inner
		void		make_key(const CullType& cull, KeyType&		key);		// Derive a key from a culling volume.

		void *		alloc(size_t bytes);									// Alloc N bytes.  Please note that the TRAITS must dispose of them!!
	};

*/

template <typename __T, int __N>
class QuadTree {
public:

	typedef			 __T					Traits;
	typedef typename __T::KeyType			KeyType;
	typedef typename __T::CullType			CullType;
	typedef typename __T::ValueType			ValueType;

	struct	Node {
		Node *		children[__N];
		CullType	cull;
		ValueType *	items;
	};

					 QuadTree(const KeyType& extent, const Traits& traits);
					~QuadTree();

	void			set_extent(const KeyType& extent);

	Node *			find(ValueType * v);
	Node *			insert(ValueType * v, int max_depth);
	void			remove(ValueType * v);
	void			remove_all(void);

	template <typename __Culler, typename __Visitor>
	void			iterate_cull(
							const __Culler&		c,
							const __Visitor&	v);

private:

	Node *			find_node(
							Node *				node,
							const KeyType&		bounds,
							ValueType *			v,
							const KeyType&		k);

	Node *			insert_node(
							Node *				node,
							const KeyType&		bounds,
							ValueType *			v,
							const CullType&		c,
							const KeyType&		k,
							int					max_depth);

	bool			remove_node(
							Node *				node,
							const KeyType&		bounds,
							ValueType *			v,
							const KeyType&		k);

	void			remove_all_node(
							Node *				node);

	template <typename __Culler, typename __Visitor>
	void			iterate_cull_node(
							Node *				n,
							const __Culler&		c,
							const __Visitor&	v);

	void			recull_node(Node * n);
	bool			empty_node(Node * n);

	Node *		internal_alloc(void);
	void		internal_free(Node *);

	KeyType		extent;
	Node *		root;
	Traits		traits;
	Node *		free_list;
};

template <typename __T, int __N>
QuadTree<__T, __N>::QuadTree(const KeyType& iextent, const Traits& itraits) :
	traits(itraits),
	extent(iextent),
	root(NULL),
	free_list(NULL)
{
}

template <typename __T, int __N>
QuadTree<__T, __N>::~QuadTree()
{
}

template <typename __T, int __N>
void	QuadTree<__T, __N>::set_extent(const KeyType& e)
{
	extent = e;
}


//------------------------------------------------------------ INSERT --------------------------------------------------------------------------------

template <typename __T, int __N>
typename QuadTree<__T, __N>::Node * QuadTree<__T, __N>::insert(ValueType * v, int max_depth)
{
	CullType	c;
	KeyType		k;
	traits.get_cull(v,c);
	traits.make_key(c,k);

	if(root == NULL)
		root = internal_alloc();
	return insert_node(root, extent, v, c, k, max_depth);
}

template <typename __T, int __N>
typename QuadTree<__T, __N>::Node * QuadTree<__T, __N>::insert_node(Node * node, const KeyType& bounds, ValueType * v, const CullType& c, const KeyType& k, int max_depth)
{
	traits.expand_by(node->cull,c);

	if (max_depth > 0)
	{
		for (int n = 0; n < __N; ++n)
		{
			KeyType sub;
			traits.subkey(bounds, sub, n);
			if (traits.contains(sub, k))
			{
				if (node->children[n] == NULL)
					node->children[n] = internal_alloc();
				return insert_node(node->children[n], sub, v, c, k, max_depth-1);
			}
		}
	}

	traits.set_next(v, node->items);
	node->items = v;
	return node;
}

//------------------------------------------------------------ FIND --------------------------------------------------------------------------------

template <typename __T, int __N>
typename QuadTree<__T, __N>::Node * QuadTree<__T, __N>::find(ValueType * v)
{
	if (root == NULL) return NULL;

	CullType	c;
	KeyType		k;
	traits.get_cull(v,c);
	traits.make_key(c,k);

	return find_node(root, extent, v, k);

}

template <typename __T, int __N>
typename QuadTree<__T, __N>::Node * QuadTree<__T, __N>::find_node(Node * node, const KeyType& bounds, ValueType * v, const KeyType& k)
{
	for (int n = 0; n < __N; ++n)
	if (node->children[n])
	{
		KeyType sub;
		traits.subkey(bounds, sub, n);
		if (traits.contains(sub, k))
		{
			return find_node(node->children[n], sub, v, k);
		}
	}

	for (ValueType * i = node->items; i; i = traits.get_next(i))
		if (i == v) return node;

	return NULL;
}

//------------------------------------------------------------ REMOVE --------------------------------------------------------------------------------



template <typename __T, int __N>
void QuadTree<__T, __N>::remove(ValueType * v)
{
	if (root == NULL) return NULL;

	CullType	c;
	KeyType		k;
	traits.get_cull(v,c);
	traits.make_key(c,k);

	if (remove_node(root, extent, v, k))
	{
		if (empty_node(root))
		{
			internal_free(root);
			root = NULL;
		}
	}
}

template <typename __T, int __N>
bool QuadTree<__T, __N>::remove_node(Node * node, const KeyType& bounds, ValueType * v, const KeyType& k)
{
	for (int n = 0; n < __N; ++n)
	if (node->children[n])
	{
		KeyType sub;
		traits.subkey(bounds, sub, n);
		if (traits.contains(sub, k))
		{
			bool dead = remove_node(node->children[n], sub, v, k);
			if (dead)
			{
				if (empty_node(node->children[n]))
				{
					internal_free(node->children[n]);
					node->children[n] = NULL;
				}

				recull_node(node);
				return true;
			}
		}
	}

	if (node->items == NULL) return false;
	else if (node->items == v) node->items = traits.get_next(v);
	else for (ValueType * i = node->items; i; i = traits.get_next(i))
	if (traits.get_next(i) == v)
		traits.set_next(i,traits.get_next(v));

	recull_node(node);
	return true;
}

//------------------------------------------------------------ REMOVE_ALL --------------------------------------------------------------------------------

template <typename __T, int __N>
void QuadTree<__T, __N>::remove_all(void)
{
	if (root)	remove_all_node(root);
	root = NULL;
}

template <typename __T, int __N>
void QuadTree<__T, __N>::remove_all_node(Node * n)
{
	for (int k = 0; k < __N; ++k)
	if(n->children[k])
		remove_all_node(n->children[k]);
	internal_free(n);
}

//------------------------------------------------------------ ITERATE/CULL --------------------------------------------------------------------------------


template <typename __T, int __N>
template <typename __Culler, typename __Visitor>
void			QuadTree<__T, __N>::iterate_cull(
						const __Culler&		c,
						const __Visitor&	v)
{
	if (root)
	iterate_cull_node(root,c,v);
}

template <typename __T, int __N>
template <typename __Culler, typename __Visitor>
void			QuadTree<__T, __N>::iterate_cull_node(
							Node *				n,
							const __Culler&		c,
							const __Visitor&	v)
{
	if (c(n->cull))
	{
		if (n->items)	v(n->items);

		for (int k = 0; k < __N; ++k)
		if(n->children[k])
			iterate_cull_node(n->children[k],c,v);
	}
}

template <typename __T, int __N>
typename QuadTree<__T, __N>::Node * QuadTree<__T, __N>::internal_alloc(void)
{
	Node * r = free_list;
	if (free_list)
		free_list = free_list->children[0];
	if (!r) r = (Node *) traits.alloc(sizeof(Node));
	for (int k = 0; k < __N; ++k)
		r->children[k] = NULL;
	r->items = NULL;
	traits.set_empty(r->cull);
	return r;
}

template <typename __T, int __N>
void QuadTree<__T, __N>::internal_free(Node * n)
{
	n->children[0] = free_list;
	free_list = n;
}

template <typename __T, int __N>
void QuadTree<__T, __N>::recull_node(Node * n)
{
	traits.set_empty(n->cull);
	for (int k = 0; k < __N; ++k)
		if (n->children[k]) traits.expand_by(n->cull,n->children[k]->cull);

	for (ValueType * i = n->items; i; i = traits.get_next(i))
	{
		CullType c;
		traits.get_cull(i,c);
		traits.expand_by(n->cull,c);
	}
}


template <typename __T, int __N>
bool QuadTree<__T, __N>::empty_node(Node * n)
{
	for (int k = 0; k < __N; ++k)
		if (n->children[k]) return false;
	return n->items == NULL;
}

#endif
