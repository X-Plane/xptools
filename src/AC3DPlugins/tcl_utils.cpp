/*
 * Copyright (c) 2007, Laminar Research.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include "tcl_utils.h"
#include <string.h>
#include <ac_plugin.h>




TCL_linked_vari::TCL_linked_vari(Tcl_Interp * iinterp, const char * tcl_name, TCL_changei_f change_cb, void * iref, int initial)
{
	setting = 0;
	var = initial;
	interp = iinterp;
	cb = change_cb;
	ref = iref;
	var_name = (char *) malloc(strlen(tcl_name)+1);
	strcpy(var_name,tcl_name);
	set(initial);

	if (tcl_stubs.Tcl_TraceVar(interp, var_name, TCL_TRACE_WRITES, tcl_trace_cb, reinterpret_cast<ClientData>(this)) != TCL_OK)
		message_dialog((char*)"Internal error setting trace on %s.", var_name);
}

TCL_linked_vari::~TCL_linked_vari()
{
	tcl_stubs.Tcl_UntraceVar(interp, var_name, TCL_TRACE_WRITES, tcl_trace_cb, reinterpret_cast<ClientData>(this));
	free(var_name);
}

void	TCL_linked_vari::set(int value)
{
//	printf("Setting %s to %d\n", var_name, value);
	var = value;
	setting = 1;
	Tcl_Obj * obj = tcl_stubs.Tcl_NewIntObj(var);
	Tcl_IncrRefCount(obj);
	if (tcl_stubs.Tcl_SetVar2Ex(interp, var_name, NULL, obj, TCL_GLOBAL_ONLY) == NULL)
		message_dialog((char*)"Internal tcl error - could not create variable %s", var_name);
	Tcl_DecrRefCount(obj);
	setting = 0;
}

int		TCL_linked_vari::get(void) const
{
	return var;
}

char * TCL_linked_vari::tcl_trace_cb(ClientData clientData, Tcl_Interp *interp, CONST84 char *part1, CONST84 char *part2, int flags)
{
	TCL_linked_vari * me = reinterpret_cast<TCL_linked_vari*>(clientData);
//	printf("Trace on %s called with flags %x\n", me->var_name, flags);
	if (me->setting) return NULL;

    Tcl_Obj * result = tcl_stubs.Tcl_GetObjResult(interp);
    Tcl_IncrRefCount(result);

    Tcl_Obj * value = tcl_stubs.Tcl_GetVar2Ex(me->interp, me->var_name,NULL, TCL_GLOBAL_ONLY);
    if (value != NULL)
	{
		if (tcl_stubs.Tcl_GetIntFromObj(NULL/*me->interp*/, value, &me->var)== TCL_OK)
		{
			if (me->cb)
				me->cb(me->var, me->ref, me);
		} else
		{
			tcl_stubs.Tcl_SetObjResult(interp, result);
//			Tcl_Obj * tmpPtr = Tcl_NewObj();
//			Tcl_IncrRefCount(tmpPtr);
//			Tcl_SetVar2Ex(me->interp, me->var_name, NULL, tmpPtr, TCL_GLOBAL_ONLY);
//			Tcl_DecrRefCount(tmpPtr);
		}
	}
	Tcl_DecrRefCount(result);
	return NULL;
}



//------------------------------------------------------------------------------------------------------------------------




TCL_linked_vard::TCL_linked_vard(Tcl_Interp * iinterp, const char * tcl_name, TCL_changed_f change_cb, void * iref, double initial)
{
	setting = 0;
	var = initial;
	interp = iinterp;
	cb = change_cb;
	ref = iref;
	var_name = (char *) malloc(strlen(tcl_name)+1);
	strcpy(var_name,tcl_name);
	set(initial);

	if (tcl_stubs.Tcl_TraceVar(interp, var_name, TCL_TRACE_WRITES, tcl_trace_cb, reinterpret_cast<ClientData>(this)) != TCL_OK)
		message_dialog((char*)"Internal error setting trace on %s.", var_name);
}

TCL_linked_vard::~TCL_linked_vard()
{
	tcl_stubs.Tcl_UntraceVar(interp, var_name, TCL_TRACE_WRITES, tcl_trace_cb, reinterpret_cast<ClientData>(this));
	free(var_name);
}

void	TCL_linked_vard::set(double value)
{
//	printf("Setting %s to %lf\n", var_name, value);
	var = value;
	setting = 1;
//	Tcl_Obj * obj = Tcl_NewDoubleObj(var);
char buf[200];
	sprintf(buf,"%.03lf", value);
	Tcl_Obj * obj = tcl_stubs.Tcl_NewStringObj(buf,-1);

	Tcl_IncrRefCount(obj);
	if (tcl_stubs.Tcl_SetVar2Ex(interp, var_name, NULL, obj, TCL_GLOBAL_ONLY) == NULL)
		message_dialog((char*)"Internal tcl error - could not create variable %s", var_name);
	Tcl_DecrRefCount(obj);
	setting = 0;
}

double		TCL_linked_vard::get(void) const
{
	return var;
}

char * TCL_linked_vard::tcl_trace_cb(ClientData clientData, Tcl_Interp *interp, CONST84 char *part1, CONST84 char *part2, int flags)
{
	TCL_linked_vard * me = reinterpret_cast<TCL_linked_vard*>(clientData);
//	printf("Trace on %s called with flags %x\n", me->var_name, flags);
	if (me->setting) return NULL;

    Tcl_Obj * result = tcl_stubs.Tcl_GetObjResult(interp);
    Tcl_IncrRefCount(result);

    Tcl_Obj * value = tcl_stubs.Tcl_GetVar2Ex(me->interp, me->var_name,NULL, TCL_GLOBAL_ONLY);
    if (value != NULL)
	{
		Tcl_IncrRefCount(value);
		if (tcl_stubs.Tcl_GetDoubleFromObj(NULL/*me->interp*/, value, &me->var)== TCL_OK)
		{
			if (me->cb)
				me->cb(me->var, me->ref, me);
		}
		else
		{
			tcl_stubs.Tcl_SetObjResult(interp, result);

//			Tcl_Obj * tmpPtr = Tcl_NewObj();
//			Tcl_IncrRefCount(tmpPtr);
//			Tcl_SetVar2Ex(me->interp, me->var_name, NULL, tmpPtr, TCL_GLOBAL_ONLY);
//			Tcl_DecrRefCount(tmpPtr);
		}
		Tcl_DecrRefCount(value);
	}
	Tcl_DecrRefCount(result);
	return NULL;
}



//------------------------------------------------------------------------------------------------------------------------


TCL_linked_vars::TCL_linked_vars(Tcl_Interp * iinterp, const char * tcl_name, TCL_changes_f change_cb, void * iref, const char * initial)
{
	setting = 0;
	var = string(initial);
	interp = iinterp;
	cb = change_cb;
	ref = iref;
	var_name = (char *) malloc(strlen(tcl_name)+1);
	strcpy(var_name,tcl_name);
	set(initial);

	if (tcl_stubs.Tcl_TraceVar(interp, var_name, TCL_TRACE_WRITES, tcl_trace_cb, reinterpret_cast<ClientData>(this)) != TCL_OK)
		message_dialog((char*)"Internal error setting trace on %s.", var_name);
}

TCL_linked_vars::~TCL_linked_vars()
{
	tcl_stubs.Tcl_UntraceVar(interp, var_name, TCL_TRACE_WRITES, tcl_trace_cb, reinterpret_cast<ClientData>(this));
	free(var_name);
}

void	TCL_linked_vars::set(const char * value)
{
//	printf("Setting %s to %s\n", var_name, value);
	var = value;
	if (var.empty()) var = string("none");
	setting = 1;
	Tcl_Obj * obj = var.empty() ? tcl_stubs.Tcl_NewObj() : tcl_stubs.Tcl_NewStringObj(var.c_str(), -1);
	Tcl_IncrRefCount(obj);
	if (tcl_stubs.Tcl_SetVar2Ex(interp, var_name, NULL, obj, TCL_GLOBAL_ONLY)==NULL)
		message_dialog((char*)"Internal tcl error - could not create variable %s", var_name);
	Tcl_DecrRefCount(obj);
	setting = 0;
}

const char *		TCL_linked_vars::get(void) const
{
	return var.c_str();
}

char * TCL_linked_vars::tcl_trace_cb(ClientData clientData, Tcl_Interp *interp, CONST84 char *part1, CONST84 char *part2, int flags)
{
	TCL_linked_vars * me = reinterpret_cast<TCL_linked_vars*>(clientData);
//	printf("Trace on %s called with flags %x\n", me->var_name, flags);
	if (me->setting) return NULL;

    Tcl_Obj * result = tcl_stubs.Tcl_GetObjResult(interp);
    Tcl_IncrRefCount(result);

    Tcl_Obj * value = tcl_stubs.Tcl_GetVar2Ex(me->interp, me->var_name,NULL, TCL_GLOBAL_ONLY);
    if (value != NULL)
	{
		int len;
		const char * c = tcl_stubs.Tcl_GetStringFromObj(value, &len);
		if (c)
		{
			me->var = string(c,c+len);
			if (me->cb)
				me->cb(me->var.c_str(), me->ref, me);
			if (len == 0)
			{
//				Tcl_Obj * tmpPtr = Tcl_NewObj();
//				Tcl_IncrRefCount(tmpPtr);
//				Tcl_SetVar2Ex(me->interp, me->var_name, NULL, tmpPtr, TCL_GLOBAL_ONLY);
//				Tcl_DecrRefCount(tmpPtr);
			}
		} else
		{
			tcl_stubs.Tcl_SetObjResult(interp, result);
			message_dialog((char*)"Ben has not hit this case yet - trace call wtih no string conv in a string-linked var!\n");
//			Tcl_ResetResult(interp);
		}
	}
	Tcl_DecrRefCount(result);
	return NULL;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
#pragma mark -
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------


TCL_linked_variv::TCL_linked_variv(Tcl_Interp * interp, const char * tcl_name, int n, TCL_changeiv_f change_cb, void * iref, int initial)
{
	vars.resize(n);
	for(int k = 0; k < n; ++k)
	{
		char buf[100];
		sprintf(buf,"%s%d",tcl_name,k);
		vars[k] = new TCL_linked_vari(interp, buf, callback, (void*) this, initial);
	}
	ref = iref;
	cb = change_cb;
}

TCL_linked_variv::~TCL_linked_variv()
{
	for (int k = 0; k < vars.size(); ++k)
		delete vars[k];
}

void	TCL_linked_variv::set(int n, int value)
{
	vars[n]->set(value);
}

int		TCL_linked_variv::get(int n) const
{
	return vars[n]->get();
}

void TCL_linked_variv::callback(int value, void * iref, TCL_linked_vari * who)
{
	TCL_linked_variv * me = (TCL_linked_variv*) iref;
	for (int k = 0; k < me->vars.size(); ++k)
	{
		if (me->vars[k] == who)
		{
			if (me->cb)
				me->cb(value, k, me->ref, me);
		}
	}
}





TCL_linked_vardv::TCL_linked_vardv(Tcl_Interp * interp, const char * tcl_name, int n, TCL_changedv_f change_cb, void * iref, double initial)
{
	vars.resize(n);
	for(int k = 0; k < n; ++k)
	{
		char buf[100];
		sprintf(buf,"%s%d",tcl_name,k);
		vars[k] = new TCL_linked_vard(interp, buf, callback, (void*) this, initial);
	}
	ref = iref;
	cb = change_cb;
}

TCL_linked_vardv::~TCL_linked_vardv()
{
	for (int k = 0; k < vars.size(); ++k)
		delete vars[k];
}

void	TCL_linked_vardv::set(int n, double value)
{
	vars[n]->set(value);
}

double		TCL_linked_vardv::get(int n) const
{
	return vars[n]->get();
}

void TCL_linked_vardv::callback(double value, void * iref, TCL_linked_vard * who)
{
	TCL_linked_vardv * me = (TCL_linked_vardv*) iref;
	for (int k = 0; k < me->vars.size(); ++k)
	{
		if (me->vars[k] == who)
		{
			if (me->cb)
				me->cb(value, k, me->ref, me);
		}
	}
}


TCL_linked_varsv::TCL_linked_varsv(Tcl_Interp * interp, const char * tcl_name, int n, TCL_changesv_f change_cb, void * iref, const char * initial)
{
	vars.resize(n);
	for(int k = 0; k < n; ++k)
	{
		char buf[100];
		sprintf(buf,"%s%d",tcl_name,k);
		vars[k] = new TCL_linked_vars(interp, buf, callback, (void*) this, initial);
	}
	ref = iref;
	cb = change_cb;
}

TCL_linked_varsv::~TCL_linked_varsv()
{
	for (int k = 0; k < vars.size(); ++k)
		delete vars[k];
}

void	TCL_linked_varsv::set(int n, const char * value)
{
	vars[n]->set(value);
}

const char *		TCL_linked_varsv::get(int n) const
{
	return vars[n]->get();
}

void TCL_linked_varsv::callback(const char * value, void * iref, TCL_linked_vars * who)
{
	TCL_linked_varsv * me = (TCL_linked_varsv*) iref;
	for (int k = 0; k < me->vars.size(); ++k)
	{
		if (me->vars[k] == who)
		{
			if (me->cb)
				me->cb(value, k, me->ref, me);
		}
	}
}


