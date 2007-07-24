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



#endif /* STLUtils_H */
