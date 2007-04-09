#ifndef IDIRECTORY_H
#define IDIRECTORY_H

/*

	These interfaces map subcomponents by string keys - sort of a hash-table of containment.

*/

#include "IUnknown.h"

class	IDirectory : public virtual IUnknown {
public:

	virtual	IUnknown *	Directory_Find(const char * name)=0;
	
};

class	IDirectoryEdit : public virtual IUnknown {
public:

	virtual	void		Directory_Edit(const char * name, IUnknown * who)=0;
	
};

#endif /* IDIRECTORY_H */
