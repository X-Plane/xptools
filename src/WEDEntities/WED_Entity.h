#ifndef WED_ENTITY
#define WED_ENTITY

#include "WED_Persistent.h"

class	WED_Entity : public WED_Persistent {

DECLARE_PERSISTENT(WED_Entity)

public:

	virtual	void 			ReadFrom(IOReader * reader);
	virtual	void 			WriteTo(IOWriter * writer);
	virtual void			FromDB(sqlite3 * db);
	virtual void			ToDB(sqlite3 * db);
	
	
private:

	int				parent_id;
	vector<int>		child_id;
	
	int				locked;
	int				hidden;
	string			name;

};
	

#endif /* WED_ENTITY */