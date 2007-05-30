#ifndef IDIRECTORY_H
#define IDIRECTORY_H

/*

	These interfaces map subcomponents by string keys - sort of a hash-table of containment.

*/

#include "IBase.h"

class	IDirectory : public virtual IBase {
public:

	virtual	IBase *	Directory_Find(const char * name)=0;
	
};

class	IDirectoryEdit : public virtual IBase {
public:

	virtual	void		Directory_Edit(const char * name, IBase * who)=0;
	
};

#endif /* IDIRECTORY_H */
