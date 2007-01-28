#ifndef WED_PROPERTIES_H
#define WED_PROPERTIES_H

#include <string>
#include "SQLUtils.h"
using std::string;

class	WED_Properties {
public:
	 WED_Properties(sqlite3 * db);
	~WED_Properties();
	
	int	exists(const char * key);

	double	getd(const char * key);
	double	geti(const char * key);
	string	gets(const char * key);
	
	void	setd(const char * key, double		 v);
	void	seti(const char * key, int			 v);
	void	sets(const char * key, const string& v);
	
	
private:

	sql_init		mMakeTable;
	sql_init		mMakeIndex;

	sql_command		mCountKeys;
	sql_command		mGetKey;
	sql_command		mSetKey;
	
};

#endif /* WED_PROPERTIES_H */
