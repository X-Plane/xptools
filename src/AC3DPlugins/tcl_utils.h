#ifndef TCL_UTILS_H
#define TCL_UTILS_H

#include <string>
#include <vector>
using std::string;
using std::vector;
#include <tcl/tcl.h>

class	TCL_linked_vari;
class	TCL_linked_vard;
class	TCL_linked_vars;
class	TCL_linked_variv;
class	TCL_linked_vardv;
class	TCL_linked_varsv;

typedef	void (* TCL_changei_f)(int value, void * ref, TCL_linked_vari * who);
typedef	void (* TCL_changed_f)(double value, void * ref, TCL_linked_vard * who);
typedef	void (* TCL_changes_f)(const char * value, void * ref, TCL_linked_vars * who);

typedef	void (* TCL_changeiv_f)(int value, int n, void * ref, TCL_linked_variv * who);
typedef	void (* TCL_changedv_f)(double value, int n, void * ref, TCL_linked_vardv * who);
typedef	void (* TCL_changesv_f)(const char * value, int n, void * ref, TCL_linked_varsv * who);

//------------------------------------------------------------------------------------------------------------------------

class	TCL_linked_vari {
public:

	 TCL_linked_vari(Tcl_Interp * interp, const char * tcl_name, TCL_changei_f change_cb, void * ref, int initial);
	~TCL_linked_vari();
	
	void	set(int value);
	int		get(void) const;
	
private:

	int				var;
	char *			var_name;
	Tcl_Interp *	interp;
	TCL_changei_f	cb;
	void *			ref;
	int				setting;

	static char * tcl_trace_cb(ClientData clientData, Tcl_Interp *interp, CONST84 char *part1, CONST84 char *part2, int flags);

};

//------------------------------------------------------------------------------------------------------------------------

class	TCL_linked_vard {
public:

	 TCL_linked_vard(Tcl_Interp * interp, const char * tcl_name, TCL_changed_f change_cb, void * ref, double initial);
	~TCL_linked_vard();
	
	void		set(double value);
	double		get(void) const;
	
private:

	double			var;
	char *			var_name;
	Tcl_Interp *	interp;
	TCL_changed_f	cb;
	void *			ref;
	int				setting;

	static char * tcl_trace_cb(ClientData clientData, Tcl_Interp *interp, CONST84 char *part1, CONST84 char *part2, int flags);

};

//------------------------------------------------------------------------------------------------------------------------

class	TCL_linked_vars {
public:

	 TCL_linked_vars(Tcl_Interp * interp, const char * tcl_name, TCL_changes_f change_cb, void * ref, const char * initial);
	~TCL_linked_vars();
	
	void			set(const char * value);
	const char *	get(void) const;
	
private:

	string			var;
	char *			var_name;
	Tcl_Interp *	interp;
	TCL_changes_f	cb;
	void *			ref;
	int				setting;

	static char * tcl_trace_cb(ClientData clientData, Tcl_Interp *interp, CONST84 char *part1, CONST84 char *part2, int flags);

};

/*****************************************************************************************************************************/

class	TCL_linked_variv {
public:

	 TCL_linked_variv(Tcl_Interp * interp, const char * tcl_name, int n, TCL_changeiv_f change_cb, void * ref, int initial);
	~TCL_linked_variv();
	
	void	set(int n, int value);
	int		get(int n) const;
	
private:
	
	vector<TCL_linked_vari *>		vars;
	void *							ref;
	TCL_changeiv_f					cb;
	
	static void callback(int value, void * ref, TCL_linked_vari * who);

};

//------------------------------------------------------------------------------------------------------------------------

class	TCL_linked_vardv {
public:

	 TCL_linked_vardv(Tcl_Interp * interp, const char * tcl_name, int n, TCL_changedv_f change_cb, void * ref, double initial);
	~TCL_linked_vardv();
	
	void		set(int n, double value);
	double		get(int n) const;
	
private:

	vector<TCL_linked_vard *>		vars;
	void *							ref;
	TCL_changedv_f					cb;

	static void callback(double value, void * ref, TCL_linked_vard * who);

};

//------------------------------------------------------------------------------------------------------------------------

class	TCL_linked_varsv {
public:

	 TCL_linked_varsv(Tcl_Interp * interp, const char * tcl_name, int n, TCL_changesv_f change_cb, void * ref, const char * initial);
	~TCL_linked_varsv();
	
	void			set(int n, const char * value);
	const char *	get(int n) const;
	
private:

	vector<TCL_linked_vars *>		vars;
	void *							ref;
	TCL_changesv_f					cb;

	static void callback(const char * value, void * ref, TCL_linked_vars * who);


};


#endif
