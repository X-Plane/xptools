/*
 * Copyright (c) 2011, mroe.
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

#ifndef WED_NWDEFS_H_INCLUDED
#define WED_NWDEFS_H_INCLUDED


#define DEFAULTPORT "10300"
#define MAX_BUF_SIZE 5000000
#define MAX_LINE_LEN 	1024
#define MAX_QUEUE_SIZE	1000
#define INTERVALT 1  				// servers interval time
#define MAX_WAIT_FORLOGIN  20 		// waittime = MAX_WAIT_FORLOGIN * servers interval time
#define CLIENT_NAME "WEDXPLUGIN"
#define MIN_CLIENT_VERS 100


// defines protocol ; just simple for now
// TODO:mroe should improved over time

// connection
/////////////////////////
#define WED_NWP_CON "con"
// con:con_login:id:"clientname":rev crlf 		(from client to login)
// con:con_leave:id:crlf						(from client if intend to leave)
// con:con_go_on:id:"pkgname"crlf				(from server if accepted)
// con:con_refused:id:crlf					    (from server if refused)

enum wed_nw_con {

	nw_con_none 		= 0	,
	nw_con_login  		= 1	,
	nw_con_leave 		= 2	,
	nw_con_go_on  		= 3	,
	nw_con_refused		= 4	,
};

// commands
/////////////////////////
#define WED_NWP_CMD "cmd"
// cmd:wed_nw_cmd:0:crlf					(request a operation)
// cmd:cmd_done:WED_nwt_cmdcrlf  		    (annswer from the other side if done)

enum wed_nw_cmd {

	nw_cmd_none  		= 0	,
	nw_cmd_done   		= 1	,
	nw_cmd_reloadscen  	= 2	,
	nw_cmd_viewcenter  	= 3	,

};

// messages
/////////////////////////
#define WED_NWP_MSG "msg"
// msg:WED_nwt_msg:0:"message"crlf			 (messages for any purpose)
enum wed_nw_msg {

	nw_msg_none   		= 0	,
	nw_msg_info  		= 1	,
	nw_msg_error 		= 2	,
	nw_msg_debug   		= 3	,

};

// object related
/////////////////////////
#define WED_NWP_ADD "add"
//add:obj_Object:id:lat:lon:[alt]:hdg:"name":"resourcename":crlf
//add:obj_Facade:id:childcnt:height:"name":"resourcename":crlf
//add:obj_FacadeRing:id:parent_id:childcnt:"name":crlf
//add:obj_FacadeNode:id:parent_id:lat:lon:[lat_lo]:[lon_lo]:[lat_hi]:[lon_hi]:wall:"name":crlf

#define WED_NWP_CHG "chg"
//chg:obj_Object:lat:lon:[alt]:hdg:["name"]:["resourcename"]:crlf
//chg:obj_Facade:id:height:"name":"resourcename":crlf
//chg:obj_FacadeRing:id:parent_id:"name":crlf
//chg:obj_FacadeNode:id:parent_id:lat:lon:[lat_lo]:[lon_lo]:[lat_hi]:[lon_hi]:[wall]:["name"]:crlf

#define WED_NWP_DEL "del"
//del:nw_obj_none:id:crlf
#define WED_NWP_GET "get"
//get:obj_type:id:crlf

enum wed_nw_obj{

	nw_obj_none   		= 0	,
	nw_obj_Object 		= 1	,
	nw_obj_Facade	  	= 2	,
	nw_obj_FacadeNode	= 3	,
	nw_obj_FacadeRing  	= 4	,
};


#define WED_NWP_SEL "sel"
//sel:nw_obj_none:id:crlf


#endif // WED_NWDEFS_H_INCLUDED
