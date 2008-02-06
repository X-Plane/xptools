/* 
 * Copyright (c) 2008, Laminar Research.
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

#ifndef TclStubs_H
#define TclStubs_H

#if defined(AC_PLUGIN)
#error YOU MUST INCLUDE THIS BEFORE ac_plugin.h
#endif

struct Tcl_Interp;
struct Tcl_Obj {
    int refCount;		/* When 0 the object will be freed. */
};
typedef void * ClientData;
#define CONST84 const
#define CONST const
#define _TCL
#define EXTERN extern

#define TCL_GLOBAL_ONLY		 1
#define TCL_NAMESPACE_ONLY	 2
#define TCL_APPEND_VALUE	 4
#define TCL_LIST_ELEMENT	 8
#define TCL_TRACE_READS		 0x10
#define TCL_TRACE_WRITES	 0x20
#define TCL_TRACE_UNSETS	 0x40
#define TCL_TRACE_DESTROYED	 0x80
#define TCL_INTERP_DESTROYED	 0x100
#define TCL_LEAVE_ERR_MSG	 0x200
#define TCL_TRACE_ARRAY		 0x800
#define TCL_TRACE_OLD_STYLE	 0x1000
#define TCL_TRACE_RESULT_DYNAMIC 0x8000
#define TCL_TRACE_RESULT_OBJECT  0x10000

#define TCL_OK		0
#define TCL_ERROR	1
#define TCL_RETURN	2
#define TCL_BREAK	3
#define TCL_CONTINUE	4


#define Tcl_IncrRefCount(objPtr) \
	++(objPtr)->refCount
#define Tcl_DecrRefCount(objPtr) \
	if (--(objPtr)->refCount <= 0) tcl_stubs.TclFreeObj(objPtr)
#define Tcl_IsShared(objPtr) \
	((objPtr)->refCount > 1)


typedef char *(Tcl_VarTraceProc) (ClientData clientData,
	Tcl_Interp *interp, CONST84 char *part1, CONST84 char *part2, int flags);

typedef void (Tcl_ExitProc) (ClientData clientData);

#define	TCL_PROCS \
	TCL_PROC(int,Tcl_TraceVar,(Tcl_Interp * interp, CONST char * varName, int flags, Tcl_VarTraceProc * proc, ClientData clientData)) \
	TCL_PROC(void,Tcl_UntraceVar, (Tcl_Interp * interp, CONST char * varName, int flags, Tcl_VarTraceProc * proc, ClientData clientData)) \
	TCL_PROC(Tcl_Obj *,Tcl_NewIntObj, (int intValue)) \
	TCL_PROC(Tcl_Obj *,Tcl_SetVar2Ex, (Tcl_Interp * interp, CONST char * part1, CONST char * part2, Tcl_Obj * newValuePtr, int flags)) \
	TCL_PROC(Tcl_Obj *,Tcl_GetObjResult, (Tcl_Interp * interp)) \
	TCL_PROC(Tcl_Obj *,Tcl_GetVar2Ex, (Tcl_Interp * interp, CONST char * part1, CONST char * part2, int flags)) \
	TCL_PROC(int,Tcl_GetIntFromObj, (Tcl_Interp * interp, Tcl_Obj * objPtr, int * intPtr)) \
	TCL_PROC(void,Tcl_SetObjResult, (Tcl_Interp * interp, Tcl_Obj * resultObjPtr)) \
	TCL_PROC(Tcl_Obj *,Tcl_NewStringObj, (CONST char * bytes, int length)) \
	TCL_PROC(int,Tcl_GetDoubleFromObj, (Tcl_Interp * interp, Tcl_Obj * objPtr, double * doublePtr)) \
	TCL_PROC(Tcl_Obj *,Tcl_NewObj, (void)) \
	TCL_PROC(char *,Tcl_GetStringFromObj, (Tcl_Obj * objPtr, int * lengthPtr)) \
	TCL_PROC(void,Tcl_CreateExitHandler, (Tcl_ExitProc * proc, ClientData clientData)) \
	TCL_PROC(void,TclFreeObj, (Tcl_Obj * objPtr)) 


#define TCL_PROC(ret,name,args)					ret (*name) args;
struct TCL_stubs {
	TCL_PROCS
};
#undef TCL_PROC
extern TCL_stubs tcl_stubs;

const char * TCL_init_stubs(void);

#endif /* TclStubs_H */
