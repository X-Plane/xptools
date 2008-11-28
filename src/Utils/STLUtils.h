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

#ifndef STLUtils_H
#define STLUtils_H

#include "AssertUtils.h"
#include <iterator>

template <class Container>
class set_insert_iterator
	: public iterator<output_iterator_tag,void,void,void,void>
{
protected:
	Container* container;

public:
	typedef Container container_type;

	explicit set_insert_iterator(Container& x) : container(&x) { }
	set_insert_iterator& operator=(typename Container::const_reference value) { container->insert(value); return *this; }


	set_insert_iterator& operator*() { return *this; }
	set_insert_iterator& operator++() { return *this; }
	set_insert_iterator  operator++(int) { return *this; }
};

template <class Container>
inline set_insert_iterator<Container> set_inserter(Container& x) { return set_insert_iterator<Container>(x); }



template <class Container>
class set_erase_iterator
	: public iterator<output_iterator_tag,void,void,void,void>
{
protected:
	Container* container;

public:
	typedef Container container_type;

	explicit set_erase_iterator(Container& x) : container(&x) { }
	set_erase_iterator& operator=(typename Container::const_reference value) { container->erase(value); return *this; }


	set_erase_iterator& operator*() { return *this; }
	set_erase_iterator& operator++() { return *this; }
	set_erase_iterator  operator++(int) { return *this; }
};

template <class Container>
inline set_erase_iterator<Container> set_eraser(Container& x) { return set_erase_iterator<Container>(x); }

template <typename Priority, typename Value>
class pqueue {
public:
	typedef Priority								priority_type;
	typedef Value									value_type;
	typedef	multimap<priority_type,value_type>		map_type;
	typedef typename map_type::iterator				map_iterator;
	typedef typename map_type::value_type			map_value_type;
	typedef map<value_type, map_iterator>			back_link_type;
	typedef typename back_link_type::iterator		back_link_iterator;
	typedef typename back_link_type::value_type		back_link_value_type;

	pqueue() { }
	pqueue(const pqueue& rhs) : items_(rhs.items_) { rebuild_links(); }
	pqueue& operator=(pqueue& rhs) { items_ = rhs.items_; back_links_.clear(); rebuild_links(); }
	
	map_iterator	insert(const priority_type& p, const value_type& v)
	{
		erase(v);
		map_iterator i = items_.insert(map_value_type(p,v));
		back_links_.insert(back_link_value_type(v,i));
		return i;
	}
	
	bool			erase(const value_type& v)
	{
		back_link_iterator bi = back_links_.find(v);
		if(bi != back_links_.end())
		{
			items_.erase(bi->second);
			back_links_.erase(bi);
			return true;
		}
		return false;
	}

	void			pop_front()
	{
		DebugAssert(!items_.empty());
		DebugAssert(back_links_.count(items_.begin()->second));
		back_links_.erase(items_.begin()->second);
		items_.erase(items_.begin());
	}
				
	priority_type		front_priority() const
	{
		DebugAssert(!items_.empty());
		return items_.begin()->first;
	}

	value_type			front_value() const
	{
		DebugAssert(!items_.empty());
		return items_.begin()->second;
	}
	
	int				count(const value_type& v) const
	{
		return back_links_.count(v);
	}
	
	bool			empty(void) const
	{
		return items_.empty();
	}

	size_t			size(void) const 
	{
		return items_.size();
	}

private:
	void rebuild_links()
	{
		DebugAssert(back_links_.empty());
		for(map_iterator i = items_.begin(); i != items_.end(); ++i)
			back_links_.insert(back_link_value_type(i->second,i));
	}

	map_type		items_;
	back_link_type	back_links_;


};



#endif /* STLUtils_H */
