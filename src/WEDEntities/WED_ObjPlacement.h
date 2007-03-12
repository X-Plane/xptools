#ifndef WED_OBJPLACEMENT_H
#define WED_OBJPLACEMENT_H

#include "WED_GISPoint_Heading.h"
#include "WED_PropertyHelper.h"

class	WED_ObjPlacement : public WED_GISPoint_Heading {

DECLARE_PERSISTENT(WED_ObjPlacement)

public:

	virtual	void 			ReadFrom(IOReader * reader);
	virtual	void 			WriteTo(IOWriter * writer);
	virtual void			FromDB(sqlite3 * db);
	virtual void			ToDB(sqlite3 * db);

private:

	int			model_id;

};


#endif
