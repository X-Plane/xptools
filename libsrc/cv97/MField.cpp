/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	MField.cpp
*
******************************************************************/

#include "MField.h"

MField::MField()
{
}

MField::~MField()
{
}

int MField::getSize()
{
	return mFieldVector.size();
}

int MField::size()
{
	return mFieldVector.size();
}

void MField::add(Field *object)
{
	mFieldVector.addElement(object);
}

void MField::insert(int index, Field *object)
{
	mFieldVector.insertElementAt(object, index);
}

void MField::insert(Field *object, int index)
{
	insert(object, index);
}

void MField::clear()
{
	mFieldVector.removeAllElements();
}

void MField::remove(int index)
{
	mFieldVector.removeElementAt(index);
}

void MField::removeLastObject()
{
	int eleSize = getSize();
	mFieldVector.removeElementAt(eleSize-1);
}

void MField::removeFirstObject()
{
	mFieldVector.removeElementAt(0);
}

Field *MField::lastObject()
{
	return (Field *)mFieldVector.lastElement();
}

Field *MField::firstObject()
{
	return (Field *)mFieldVector.firstElement();
}

Field *MField::getObject(int index)
{
	return (Field *)mFieldVector.elementAt(index);
}

void MField::setObject(int index, Field *object)
{
	mFieldVector.setElementAt(object, index);
}

void MField::replace(int index, Field *object)
{
	setObject(index, object);
}

void MField::copy(MField *srcMField)
{
	clear();
	for (int n=0; n<srcMField->getSize(); n++) {
		add(srcMField->getObject(n));
	}
}

void MField::outputContext(ostream& printStream, char *indentString1, char *indentString2)
{
	char *indentString = new char[strlen(indentString1)+strlen(indentString2)+1];
	strcpy(indentString, indentString1);
	strcat(indentString, indentString2);
	outputContext(printStream, indentString);
	delete indentString;
}

////////////////////////////////////////////////
//	MField::setValue
////////////////////////////////////////////////

void MField::setValue(char *buffer)
{
	char value[128];
	char *bp = buffer;
	int nSize = getSize();
	for (int n=0; n<nSize; n++) {
		int l=0;
		while (bp[l] != ',' && bp[l] != '\0')
			l++;
		if (bp[l] == '\0')
			return;
		strncpy(value, bp, l);
		Field *field = getObject(n);
		field->setValue(value);
		bp += l;
	}
}

////////////////////////////////////////////////
//	MField::getValue
////////////////////////////////////////////////

char *MField::getValue(char *buffer, int bufferLen)
{
	buffer[0] = '\0';

	int		nString = 0;
	int		nSize = getSize();
	char	value[128];

	for (int n=0; n<nSize; n++) {
		Field *field = getObject(n);
		field->getValue(value);
		int l = strlen(value);
		if ((nString + l + 2) < bufferLen) {
			if (0 < nString)
				strcat(buffer, ", ");
			strcat(buffer, value);
			if (0 < nString)
				nString += (l + 2);
			else
				nString += l;
		}
		else
			break;
	}

	return buffer;
}
