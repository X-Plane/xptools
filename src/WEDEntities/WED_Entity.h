#ifndef WED_ENTITY_H
#define WED_ENTITY_H

#include "WED_Thing.h"

class	WED_Entity : public WED_Thing { 

DECLARE_PERSISTENT(WED_Entity)

public:

	virtual	void 			ReadFrom(IOReader * reader);
	virtual	void 			WriteTo(IOWriter * writer);
	virtual void			FromDB(sqlite3 * db);
	virtual void			ToDB(sqlite3 * db);
	
private:

	WED_PropBoolText			locked;
	WED_PropBoolText			hidden;

};
	

#endif /* WED_ENTITY_H */