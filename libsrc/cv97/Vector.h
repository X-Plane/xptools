/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	Vector.h
*
******************************************************************/

#ifndef _CV97_VECTOR_H_
#define _CV97_VECTOR_H_

#include "LinkedList.h"

#ifdef SUPPORT_STL

#include <vector>

template <class T>
class VectorElement {
	bool	mbDelObj;
	T		*mObj;
public:
	VectorElement() {
		setObject(NULL);
		setObjectDeleteFlag(false);
	}
	VectorElement(T *obj, bool delObjFlag = true) {
		setObject(obj);
		setObjectDeleteFlag(delObjFlag);
	}
	~VectorElement() {
		if (mbDelObj)
			delete mObj;
	}
	void setObject(T *obj)	{
		mObj = obj;
	}
	T *getObject()	{
		return mObj;
	}
	void setObjectDeleteFlag(bool flag)
	{
		mbDelObj = flag;
	}
};

template<class T >
class Vector : public std::vector< VectorElement<T> *> {

public:

	Vector() {
	}

	~Vector() {
		clear();
	}

	void addElement(T *obj, bool delObjFlag = true)
	{
		VectorElement<T> *elem = new VectorElement<T>(obj, delObjFlag);
		push_back(elem);
	}

	void insertElementAt(T *obj, int index, bool delObjFlag = true)
	{
		VectorElement<T> *elem = new VectorElement<T>(obj, delObjFlag);
		insert(begin() + index, elem);
	}

	T *elementAt(int index)
	{
		if (index < 0)
			return (T *)NULL;
		if (size() < (index+1))
			return (T *)NULL;
		VectorElement<T> *elem = at(index);
		return elem->getObject();
	}

	void setElementAt(T *obj, int index) {
		if (index < 0)
			return;
		if (size() < (index+1))
			return;
		VectorElement<T> *elem = at(index);
		elem->setObject(obj);
	}

	int	indexOf(T *elem) {
		return indexOf(elem, 0);
	}

	int indexOf(T *elem, int index) {
		int cnt = size();
		for (int n=index; n<cnt; n++) {
			if (elem == elementAt(n))
				return n;
		}
		return -1;
	}

	void removeElement(T *obj) {
		removeElementAt(indexOf(obj));
	}

	void removeElementAt(int index)
	{
		if (index < 0)
			return;
		if (size() < (index+1))
			return;
		VectorElement<T> *elem = at(index);
		delete elem;
		erase(begin() + index);
	}

	void removeAllElements()
	{
		int cnt = size();
		for (int n=0; n<cnt; n++) {
			VectorElement<T> *elem = at(n);
			delete elem;
		}
		clear();
	}

	bool isEmpty()
	{
		return (size() == 0) ? false : true;
	}

	T *firstElement()
	{
		return elementAt(0);
	}

	T *lastElement()
	{
		return elementAt(size()-1);
	}
};

#else // SUPPORT_STL

template <class T>
class VectorElement : public LinkedListNode<T> {
	bool mbDelObj;
	T	*mObj;
public:
	VectorElement() : LinkedListNode<T>(true) {
		setObject(NULL);
		setObjectDeleteFlag(false);
	}
	VectorElement(T *obj, bool delObjFlag = true) : LinkedListNode<T>((bool)false) {
		setObject(obj);
		setObjectDeleteFlag(delObjFlag);
	}
	~VectorElement() {
		remove();
		if (mbDelObj)
			delete mObj;
	}
	void setObject(T *obj)	{
		mObj = obj;
	}
	T *getObject()	{
		return mObj;
	}
	void setObjectDeleteFlag(bool flag)
	{
		mbDelObj = flag;
	}
};

template <class T>
class Vector {
	LinkedList<T>	 mElementList;
public:

	Vector() {
	}

	~Vector() {
		removeAllElements();
	}

	void addElement(T *obj, bool delObjFlag = true) {
		VectorElement<T> *element = new VectorElement<T>(obj, delObjFlag);
		mElementList.addNode(element);
	}

	void insertElementAt(T *obj, int index, bool delObjFlag = true) {
		VectorElement<T> *element = (VectorElement<T> *)mElementList.getNode(index);
		if (element) {
			VectorElement<T> *newElement = new VectorElement<T>(obj, delObjFlag);
			newElement->insert((VectorElement<T> *)element->prev());
		}
	}

	int contains(void *elem) {
		int cnt = size();
		for (int n=0; n<cnt; n++) {
			if (elem == elementAt(n))
				return 1;
		}
		return 0;
	}

	T *elementAt(int index) {
		VectorElement<T> *element = (VectorElement<T> *)mElementList.getNode(index);
		return element ? element->getObject() : NULL;
	}

	T *firstElement() {
		VectorElement<T> *element = (VectorElement<T> *)mElementList.getNodes();
		return element ? element->getObject() : NULL;
	}

	int	indexOf(T *elem) {
		return indexOf(elem, 0);
	}

	int indexOf(T *elem, int index) {
		int cnt = size();
		for (int n=index; n<cnt; n++) {
			if (elem == elementAt(n))
				return n;
		}
		return -1;
	}

	bool isEmpty() {
		return mElementList.getNodes() ? false : true;
	}

	T *lastElement() {
		VectorElement<T> *element = (VectorElement<T> *)mElementList.getNode(size()-1);
		return element ? element->getObject() : NULL;
	}

	int	lastIndexOf(T *elem);
	int	lastIndexOf(T *elem, int index);

	void removeAllElements() {
		mElementList.deleteNodes();
	}

	void removeElement(T *obj) {
		removeElementAt(indexOf(obj));
	}

	void removeElementAt(int index) {
		VectorElement<T> *element = (VectorElement<T> *)mElementList.getNode(index);
		if (element)
			delete element;
	}

	void setElementAt(T *obj, int index) {
		VectorElement<T> *element = (VectorElement<T> *)mElementList.getNode(index);
		if (element)
			element->setObject(obj);
	}

	int	size() {
		return mElementList.getNNodes();
	}
};

#endif // SUPPORT_STL

#endif
