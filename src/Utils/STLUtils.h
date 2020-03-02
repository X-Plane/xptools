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

template<typename T>				void trim(T& v);							// Remove extra space from a vector.
template <class T>					void nuke_container(T& v);					// Clear a container, free vector memory
template <typename I>				I nth_from(const I& i, int n);				// Return iterator that is "n" from input.  Like advance but inline with return value.
template <typename C>				int count_circulator(C circulator);			// Number of items in one whole circulation.
template<typename K, typename V>	K highest_key(const map<K,V>& histo);		// Return most common key vlaue in a histogram, linear time.

// Reverse a histogram (so go from key->number of items to item count -> which item), returning the total item count.
template <typename K, typename V>	V reverse_histo(const map<K,V>& in_histo, multimap<V,K>& out_histo);

// General purpose string tokenizer. Output iterator's value type must be constructed from PAIRs of input iterators.
// (So if the input is a pair of string iterator, the output should be a back vector inserter where the vector contains strings (for exampe).
template<class InputIterator, class Separator, class OutputIterator>
void tokenize_string(InputIterator begin, InputIterator end, OutputIterator oi, Separator sep);

// Same as above, but the separator is treated as a character functor instead of a delimiting character.
template<class InputIterator, class Separator, class OutputIterator>
void tokenize_string_func(InputIterator begin, InputIterator end, OutputIterator oi, Separator sep);

// Return true if any of the strings in the input range are case-insensitive sub-strings of the passed
// in string.  Returns true if txt is empty, but false if the filter range is empty.
template <class InputIterator, class String>
bool filter_match(const String& txt, InputIterator begin, InputIterator end);

// SET UTILS

// Simple inserter and eraser output iterator adapter for STL sets.
template <class Container>	class set_insert_iterator;
template <class Container>	inline set_insert_iterator<Container> set_inserter(Container& x) { return set_insert_iterator<Container>(x); }
template <class Container>	class set_erase_iterator;
template <class Container>	inline set_erase_iterator<Container> set_eraser(Container& x) { return set_erase_iterator<Container>(x); }


// Given two sets or sorted ranges, compute the length of their union without actually computing the union.  Linear time.
template<typename _InputIterator1, typename _InputIterator2>
size_t set_union_length(_InputIterator1 __first1, _InputIterator1 __last1, _InputIterator2 __first2, _InputIterator2 __last2);
template <typename T>					size_t set_union_length(const set<T>& s1, const set<T>& s2);
// Returns true if sub is a superset of S.
template <typename T>					bool is_subset(const set<T>& sub, const set<T>& s);

// PRIORITY Q BASED ON TWO MAPS
// You can reprioritize a single item in O(2LOGN) time by erasing it (by value) and then re-inserting it
// under a new priority.  

template <typename Priority, typename Value>
class pqueue;

// SEQUENCE STUFF
// Sequence uses () to know that it is valid.

template <typename T>
struct sequence_for_container {

	typedef typename T::value_type		value_type;
	typedef sequence_for_container<T>	self;

	typename T::iterator				begin;
	typename T::iterator				end;
	
	sequence_for_container(T& c) : begin(c.begin()), end(c.end()) { }
	self& operator++() { ++begin; return *this; }
	value_type& operator*() { return *begin; }
	bool operator()() { return begin != end; }

};	

template <typename T>
struct const_sequence_for_container {

	typedef typename T::value_type			value_type;
	typedef const_sequence_for_container<T>	self;

	typename T::const_iterator		begin;
	typename T::const_iterator		end;
	
	const_sequence_for_container(const T& c) : begin(c.begin()), end(c.end()) { }
	self& operator++() { ++begin; return *this; }
	const value_type& operator*() { return *begin; }
	bool operator()() { return begin != end; }

};	

template <typename S, typename F>
struct filtered_seq {

	typedef typename S::value_type	value_type;
	typedef filtered_seq<S,F>		self;
	
	filtered_seq(const S& seq, const F& filter=F()) : s(seq), f(filter) { while(s() && !f(*s)) ++s; }
	filtered_seq(const self& rhs) : s(rhs.s), f(rhs.f) { }
	self& operator=(const self& rhs) { s = rhs.s; f = rhs.f; return *this; }

	self& operator++() { ++s; while(s() && !f(*s)) ++s; return *this; }
	value_type& operator*() { return *s; }
	bool operator()()  { return s(); }

	S	s;
	F	f;
};

template <typename S, typename F, int N>
struct split_seq {

	typedef typename S::value_type	value_type;
	typedef split_seq<S,F,N>		self;
	
	split_seq(S src, F functor) : s(src), f(functor), i(0), c(0) {
		while (s() && i == c)
		{
			i = 0;
			c = f(*s, v);
			DebugAssert(c <= N);
			++s;			
		}
	}

	self& operator++() { ++i; 
		while (s() && i == c)
		{
			i = 0;
			c = f(*s, v);
			DebugAssert(c <= N);
			++s;			
		}
		return *this;
	}
	
	value_type& operator*() { return v[i]; }
	bool operator()()  { return i < c || s(); }

	S			s;
	F			f;
	int			i;		// current index for push
	int			c;		// count from current pull
	value_type	v[N];

};


template <typename T, typename S>
void sequence_push_back(T& container, S& seq);

/*
template <typename Priority, typename Value>
class pqueue {
public:
	typedef Priority								priority_type;
	typedef Value									value_type;

	pqueue();
	pqueue(const pqueue& rhs);
	pqueue& operator=(pqueue& rhs);

	map_iterator	insert(const priority_type& p, const value_type& v);
	bool			erase(const value_type& v);
	void			pop_front();
	priority_type	front_priority() const;
	value_type		front_value() const;
	int				count(const value_type& v) const;
	bool			empty(void) const;
	size_t			size(void) const;
};
*/

//--Case insensitive string implementation-------------------------------------
//Thanks Herb Stutter http://www.gotw.ca/gotw/029.htm
struct ci_char_traits : public std::char_traits<char>
{
	static bool eq(char c1, char c2);
	static bool lt(char c1, char c2);
	static int compare(const char* s1, const char* s2, size_t n);
	static const char* find(const char* s, size_t n, char a);
};

typedef std::basic_string<char, ci_char_traits> ci_string;

static std::ostream& operator<<(std::ostream& os, const ci_string& str);
//---------------------------------------------------------------------------//

//------------------------------------------------------------------------------------------------------------------------



template<typename T>
void trim(T& v)
{
	if(v.size() != v.capacity())
	{
		T c(v);
		c.swap(v);
	}
}

template <typename I>
I nth_from(const I& i, int n)
{
	I r(i);
	advance(r,n);
	return r;
}

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

//template <class Container>
//inline set_insert_iterator<Container> set_inserter(Container& x) { return set_insert_iterator<Container>(x); }



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

//template <class Container>
//inline set_erase_iterator<Container> set_eraser(Container& x) { return set_erase_iterator<Container>(x); }


template <typename Circulator>
int count_circulator(Circulator circ)
{
	int r = 0;
	Circulator stop(circ);
	do {
		++r;
	} while (++circ != stop);
	return r;
}

template <typename Circulator>
int circulator_distance_to(Circulator circ, Circulator ent)
{
	if(circ == ent) return 0;
	int r = 0;
	Circulator stop(circ);
	do {
		++r;
		++circ;
	} while (circ != stop && circ != ent);
	if(circ == stop) return -1;
	return r;
	
}


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

template<class InputIterator, class Separator, class OutputIterator>
void tokenize_string(InputIterator begin, InputIterator end, OutputIterator oi, Separator sep)
{
	typedef typename OutputIterator::container_type::value_type WordType;
	while(begin != end)
	{
		while(begin != end && *begin == sep) ++begin;
		
		InputIterator mark1(begin);
		
		while(begin != end && *begin != sep) ++begin;
		
		InputIterator mark2(begin);
		
		if(mark1 != mark2)
			*oi = WordType(mark1, mark2);		
	}
}

template<class InputIterator, class Separator, class OutputIterator>
void tokenize_string_func(InputIterator begin, InputIterator end, OutputIterator oi, Separator sep)
{
	typedef typename OutputIterator::container_type::value_type WordType;
	while(begin != end)
	{
		while(begin != end && sep(*begin)) ++begin;
		
		InputIterator mark1(begin);
		
		while(begin != end && !sep(*begin)) ++begin;
		
		InputIterator mark2(begin);
		
		if(mark1 != mark2)
			*oi = WordType(mark1, mark2);		
	}
}

template <class InputIterator, class String>
bool filter_match(const String& haystack, InputIterator begin, InputIterator end)
{
	typename String::size_type hlen = haystack.size();
	
	for (InputIterator needle = begin; needle != end; ++needle)
	if(needle->size() <= hlen)
	{
		typename String::size_type maxpos = hlen - needle->size();
		for(typename String::size_type offset = 0; offset <= maxpos; ++offset)
		{
			typename String::size_type ni = 0;
			for(ni = 0; ni < needle->size(); ++ni)
			if(toupper((*needle)[ni]) != toupper(haystack[ni+offset]))
				break;
			if(ni == needle->size())
				return true;
		}		
	}
	return false;
}


template<typename K, typename V>
K highest_key(const map<K,V>& histo)
{
	typename map<K,V>::const_iterator best = histo.begin();
	typename map<K,V>::const_iterator i(best);
	++i;
	while(i != histo.end())
	{
		if(i->second > best->second)
			best = i;
		++i;
	}
	return best->first;
}

template <class T>
void nuke_container(T& v)
{
	T e;
	v.swap(e);
}


template<typename _InputIterator1, typename _InputIterator2>
size_t set_union_length(_InputIterator1 __first1, _InputIterator1 __last1,
						_InputIterator2 __first2, _InputIterator2 __last2)
{
	size_t result = 0;
	while (__first1 != __last1 && __first2 != __last2)
	{
		if (*__first1 < *__first2)
		{
			++__first1;
		}
		else if (*__first2 < *__first1)
		{
			++__first2;
		}
		else
		{
			++__first1;
			++__first2;
		}
		++result;
	}
	return result + distance(__first1,__last1) + distance(__first2,__last2);
}


template <typename T>
bool is_subset(const set<T>& sub, const set<T>& s)
{
	return includes(s.begin(),s.end(),sub.begin(),sub.end());
}

template <typename T>
size_t set_union_length(const set<T>& s1, const set<T>& s2)
{
	return set_union_length(s1.begin(),s1.end(),s2.begin(),s2.end());
}


template <typename K, typename V>
V reverse_histo(const map<K,V>& in_histo, multimap<V,K>& out_histo)
{
	V total = 0;
	out_histo.clear();
	for(typename map<K,V>::const_iterator h = in_histo.begin(); h != in_histo.end(); ++h)
	{
		total += h->second;
		out_histo.insert(typename multimap<V,K>::value_type(h->second,h->first));
	}
	return total;
}

template <typename T, typename S>
void sequence_push_back(T& container, S& seq)
{
	while(seq())
	{
		container.push_back(*seq);
		++seq;
	}
}

#endif /* STLUtils_H */
