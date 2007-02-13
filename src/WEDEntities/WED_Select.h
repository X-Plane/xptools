#ifndef WED_SELECT_H
#define WED_SELECT_H

#include "WED_Thing.h"
#include "GUI_Broadcaster.h"

class	WED_Select : public WED_Thing, public GUI_Broadcaster { 

DECLARE_PERSISTENT(WED_Select)

public:

			bool			IsSelected(WED_Thing * who) const;

			void			Select(WED_Thing * who);
			void			Clear(void);
			void			Toggle(WED_Thing * who);
			void			Insert(WED_Thing * who);
			void			Erase(WED_Thing * who);


	virtual	void 			ReadFrom(IOReader * reader);
	virtual	void 			WriteTo(IOWriter * writer);
	virtual void			FromDB(sqlite3 * db);
	virtual void			ToDB(sqlite3 * db);
	
private:

	set<int>		mSelected;

};
	
#endif
