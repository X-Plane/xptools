#ifndef WED_KEYOBJECTS_H
#define WED_KEYOBJECTS_H

#include "WED_Thing.h"
#include "IDirectory.h"

class	WED_KeyObjects : public WED_Thing, public virtual IDirectoryEdit {

DECLARE_PERSISTENT(WED_KeyObjects)

public:

	virtual	IUnknown *	Directory_Find(const char * name);
	virtual	void		Directory_Edit(const char * name, IUnknown * who);

	virtual	void 			ReadFrom(IOReader * reader);
	virtual	void 			WriteTo(IOWriter * writer);
	virtual void			FromDB(sqlite3 * db);
	virtual void			ToDB(sqlite3 * db);
	
private:

		map<string,int>		choices;
		
};

#endif
