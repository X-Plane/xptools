#ifndef WED_THING_H
#define WED_THING_H

#include "WED_Persistent.h"
#include "WED_PropertyHelper.h"

class	WED_Thing : public WED_Persistent, public WED_PropertyHelper {

DECLARE_PERSISTENT(WED_Thing)

public:

			int					CountChildren(void) const;
			WED_Thing *			GetNthChild(int n) const;
			WED_Thing *			GetNamedChild(const string& s) const;
			WED_Thing *			GetParent(void) const;
			void				SetParent(WED_Thing * parent, int nth);

			void				GetName(string& name) const;
			void				SetName(const string& name);

	virtual	void 			ReadFrom(IOReader * reader);
	virtual	void 			WriteTo(IOWriter * writer);
	virtual void			FromDB(sqlite3 * db);
	virtual void			ToDB(sqlite3 * db);
	
	
	// From WED_PropertyHelper...
	virtual	void			PropEditCallback(void);	
	
private:
			void				AddChild(int id, int n);
			void				RemoveChild(int id);

	int				parent_id;
	vector<int>		child_id;
	
	WED_PropStringText			name;

};
	

#endif /* WED_THING_H */
