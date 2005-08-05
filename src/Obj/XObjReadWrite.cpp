/* 
 * Copyright (c) 2004, Laminar Research.
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
 
 // TODO
 // - Clean up writing code a bit to handle decimals better.
 
#include "XObjReadWrite.h"
#include "XObjDefs.h"

/****************************************************************************************
 * FILE SCANNING UTILS
 ****************************************************************************************/
 
typedef unsigned char 	xbyt;
typedef int				xint;
typedef float			xflt;
#define Xtrue			true
#define Xfals			false

// Identify chars that indicate a new line
inline xint  TXT_MAP_eoln(const xbyt* c)
{
	return(*c==0 || *c==13 || *c=='\n');							  
}

// Identify chars that separate with whitespace
inline xint  TXT_MAP_space(const xbyt* c)
{
	return(*c=='\t' || *c==' ');							  
}

// Consume all white space and newlines to get to the beginning of the next non-blank line.  
// This implies that leading and trailing white space for a newline is ignored, as are blank lines.
inline void  TXT_MAP_finish_line(xbyt*& c, const xbyt* c_max)
{
	while(c< c_max && (TXT_MAP_eoln(c) || TXT_MAP_space(c))) ++c;	// Scan through whitespace too - helps find blank lines with chars on them!
}

// Build a string from the first char to the end of the line (including trailing whitespace).
inline void TXT_MAP_str_scan_eoln(xbyt*& c,const xbyt* c_max,string* input)
{
	while(c< c_max && TXT_MAP_space(c)) ++c;

	xbyt* c1=c;
	while(c<c_max && !TXT_MAP_eoln(c)) ++c;
	xbyt* c2=c;
	
	if(input)*input=string(c1,c2);

	TXT_MAP_finish_line(c,c_max);	// call the proc to go to EOLN here since windows can have TWO eoln chars!
}

// Build a string of the first non-whitespace word.
inline void TXT_MAP_str_scan_space(xbyt*& c,const xbyt* c_max,string* input)
{
	while(c<c_max && TXT_MAP_space(c)) ++c;

	xbyt* c1=c;
	while(c<c_max && !TXT_MAP_space(c) && !TXT_MAP_eoln(c)) ++c;
	xbyt* c2=c;	
	
	if(input)*input=string(c1,c2);
}

inline xint TXT_MAP_str_match_space(xbyt*& c,const xbyt* c_max,const char* input, bool eol_ok)
{
	while(c<c_max && TXT_MAP_space(c)) ++c;
	
	xbyt* c1=c;
	while(c<c_max && *c==*input && *input != 0) { ++c; ++input; }
		
	if (*input==0 && 		   TXT_MAP_space(c)) return Xtrue;
	if (*input==0 && eol_ok && TXT_MAP_eoln (c)) return Xtrue;
	c=c1;
	return Xfals;
}

inline xflt TXT_MAP_flt_scan(xbyt*& c,const xbyt* c_max, bool go_next_line)
{
	while(c< c_max && (
		TXT_MAP_space(c) ||
		(go_next_line && TXT_MAP_eoln(c))))	++c;

	xflt ret_val	=0;
	xint decimals	=0;
	xint negative	=Xfals;
	xint has_decimal=Xfals;

	while(c<c_max && !TXT_MAP_space(c) && !TXT_MAP_eoln(c))
	{
			 if(*c=='-')negative	=Xtrue;
		else if(*c=='+')negative	=Xfals;
		else if(*c=='.')has_decimal	=Xtrue;
		else
		{
			ret_val=(10*ret_val)+*c-'0';
			if(has_decimal)decimals++;
		}
		++c;
	}
	return ret_val/pow((xflt)10,(xflt)decimals)*((negative)?-1.0:1.0);
}

inline xint TXT_MAP_int_scan(xbyt*& c,const xbyt* c_max, bool go_next_line)
{
	while(c< c_max && (
		(				 TXT_MAP_space(c)) ||
		(go_next_line && TXT_MAP_eoln(c))))		++c;

	xint retval	=0;
	xint sign_mult=1;

	if(c<c_max && *c=='-'){sign_mult=-1;	++c;}
	if(c<c_max && *c=='+'){sign_mult= 1;	++c;}

	while(c<c_max && !TXT_MAP_space(c) && !TXT_MAP_eoln(c))
	{
		retval=(10*retval)+*c-'0';
		++c;
	}
	return sign_mult * retval;
}

inline xint TXT_MAP_continue(const xbyt* c,const xbyt* c_max)
{
	return (c < c_max);
}

/****************************************************************************************
 * OBJ 2/7 REAd
 ****************************************************************************************/
bool	XObjRead(const char * inFile, XObj& outObj)
{
		int 	count;
		vec_rgb	vrgb;
		vec_tex	vst;

	outObj.texture.clear();
	outObj.cmds.clear();

	float scanned_st_rgb[4][3]={0,0,0 , 0,0,0,// [corner][color or st]
                               0,0,0 , 0,0,0};	
	
	outObj.cmds.clear();

	/*********************************************************************
	 * READ FILE INTO MEM
	 *********************************************************************/
	
	FILE * objFile = fopen(inFile, "rb");
	if (!objFile) return false;
	fseek(objFile,0L,SEEK_END);
	int filesize = ftell(objFile);
	fseek(objFile,0L,SEEK_SET);
	unsigned char * mem_buf = (unsigned char *) malloc(filesize);
	if (mem_buf == NULL) { fclose(objFile); return false; }
	if (fread(mem_buf, 1, filesize, objFile) != filesize)
	{
		free(mem_buf); fclose(objFile); return false;
	}
	fclose(objFile);

	/*********************************************************************
	 * READ HEADER
	 *********************************************************************/
	
	unsigned char *	cur_ptr = mem_buf;
	unsigned char *	end_ptr = mem_buf + filesize;
	
	// LINE 1: A/I - who cares?!?
	TXT_MAP_str_scan_eoln(cur_ptr, end_ptr, NULL);
	
	// LINE 2: version
	int vers = TXT_MAP_int_scan(cur_ptr, end_ptr, Xfals);
	TXT_MAP_str_scan_eoln(cur_ptr, end_ptr, NULL);
	
	// LINE 3: "OBJ"
	if (vers == 700)
	{
		if (!TXT_MAP_str_match_space(cur_ptr, end_ptr, "OBJ", Xtrue))
			vers = 0;
		TXT_MAP_str_scan_eoln(cur_ptr, end_ptr, NULL);
	}
	
	// If we don't have a good version, bail.
	if (vers != 2 && vers != 700)
	{
		free(mem_buf);
		return false;
	}
	
	// LINE 4: Texture	
	TXT_MAP_str_scan_space(cur_ptr, end_ptr, &outObj.texture);
	TXT_MAP_str_scan_eoln(cur_ptr, end_ptr, NULL);
	
		
	/************************************************************
	 * READ GEOMETRIC COMMANDS
	 ************************************************************/

	bool	stop = false;

	while (!stop && TXT_MAP_continue(cur_ptr, end_ptr))
	{
		XObjCmd	cmd;
		
		/************************************************************
		 * OBJ2 SCANNING
		 ************************************************************/		
		if (vers == 2)
		{
			int obj2_op = TXT_MAP_int_scan(cur_ptr, end_ptr, Xfals);
			switch(obj2_op) {
			case 1:
			case 2:
				// Points and lines.  The header line contains the color x10.
				// The type (pt or line) tell how much geometry follows.
				cmd.cmdID = (obj2_op == 1) ? obj_Light : obj_Line;
				cmd.cmdType = type_PtLine;
				count = obj2_op;
				scanned_st_rgb[0][0]=scanned_st_rgb[1][0]=TXT_MAP_flt_scan(cur_ptr, end_ptr, Xfals)*0.1; // r
				scanned_st_rgb[0][1]=scanned_st_rgb[1][1]=TXT_MAP_flt_scan(cur_ptr, end_ptr, Xfals)*0.1; // g
				scanned_st_rgb[0][2]=scanned_st_rgb[1][2]=TXT_MAP_flt_scan(cur_ptr, end_ptr, Xfals)*0.1; // b
				TXT_MAP_str_scan_eoln(cur_ptr, end_ptr, NULL);

				// Sets of x,y,z follows.
				for (int t = 0; t < count; ++t)
				{
					vrgb.v[0] = TXT_MAP_flt_scan(cur_ptr, end_ptr, Xtrue);
					vrgb.v[1] = TXT_MAP_flt_scan(cur_ptr, end_ptr, Xtrue);
					vrgb.v[2] = TXT_MAP_flt_scan(cur_ptr, end_ptr, Xtrue);
					vrgb.rgb[0] = scanned_st_rgb[t][0];
					vrgb.rgb[1] = scanned_st_rgb[t][1];
					vrgb.rgb[2] = scanned_st_rgb[t][2];
					cmd.rgb.push_back(vrgb);
				}
				outObj.cmds.push_back(cmd);
				TXT_MAP_str_scan_eoln(cur_ptr, end_ptr, NULL);
				break;

			case 3:
			case 4:
			case 5:
				// Finite-size polygons.  The header line contains s1, s2, t1, t2.
				cmd.cmdID = (obj2_op == 5) ? obj_Quad_Hard : obj_Quad;
				if (obj2_op == 3) cmd.cmdID = obj_Tri;
				cmd.cmdType = type_Poly;
				count = obj2_op;
				if (count == 5) count = 4;				
				// Make sure to 'spread' the 4 S/T coords to 8 points.  This is 
				// because 
				scanned_st_rgb[2][0]=scanned_st_rgb[3][0]=TXT_MAP_flt_scan(cur_ptr, end_ptr, Xfals);	// s1
				scanned_st_rgb[0][0]=scanned_st_rgb[1][0]=TXT_MAP_flt_scan(cur_ptr, end_ptr, Xfals);	// s2
				scanned_st_rgb[1][1]=scanned_st_rgb[2][1]=TXT_MAP_flt_scan(cur_ptr, end_ptr, Xfals);	// t1
				scanned_st_rgb[0][1]=scanned_st_rgb[3][1]=TXT_MAP_flt_scan(cur_ptr, end_ptr, Xfals);  	// t2

				TXT_MAP_str_scan_eoln(cur_ptr, end_ptr, NULL);
				// Read sets of 3 points.
				for (int t = 0; t < count; ++t)
				{
					vst.v[0] =  TXT_MAP_flt_scan(cur_ptr, end_ptr, Xtrue);
					vst.v[1] =  TXT_MAP_flt_scan(cur_ptr, end_ptr, Xtrue);
					vst.v[2] =  TXT_MAP_flt_scan(cur_ptr, end_ptr, Xtrue);
					vst.st[0] = scanned_st_rgb[t][0];
					vst.st[1] = scanned_st_rgb[t][1];
					cmd.st.push_back(vst);
				}
				outObj.cmds.push_back(cmd);
				TXT_MAP_str_scan_eoln(cur_ptr, end_ptr, NULL);
				break;				
				
			case 99:
				TXT_MAP_str_scan_eoln(cur_ptr, end_ptr, NULL);
				stop = true;
				// 99 is the end token for obj2 files.
				break;
			default:
				// Negative numbers equal positive
				// quad strips.  The count is the number
				// of vertex pairs, since they are always even.
				if (obj2_op >= 0)
					return false;
				count = -obj2_op;
				count *= 2;
				cmd.cmdID = obj_Quad_Strip;
				cmd.cmdType = type_Poly;
				TXT_MAP_str_scan_eoln(cur_ptr, end_ptr, NULL);
				// Read a pair of x,y,z,s,t coords.
				while (count--)
				{
					vst.v[0] = TXT_MAP_flt_scan(cur_ptr, end_ptr, Xtrue);
					vst.v[1] = TXT_MAP_flt_scan(cur_ptr, end_ptr, Xtrue);
					vst.v[2] = TXT_MAP_flt_scan(cur_ptr, end_ptr, Xtrue);
					vst.st[0] = TXT_MAP_flt_scan(cur_ptr, end_ptr, Xtrue);
					vst.st[1] = TXT_MAP_flt_scan(cur_ptr, end_ptr, Xtrue);
					cmd.st.push_back(vst);
				}
				outObj.cmds.push_back(cmd);					
				TXT_MAP_str_scan_eoln(cur_ptr, end_ptr, NULL);
				break;
			}
			
		} else {

			/************************************************************
			 * OBJ7 SCANNING
			 ************************************************************/		
		
			string	cmd_name;
			TXT_MAP_str_scan_space(cur_ptr, end_ptr, &cmd_name);
			int cmd_id = FindObjCmd(cmd_name.c_str(), false);
			
			cmd.cmdType = gCmds[cmd_id].cmd_type;
			cmd.cmdID = gCmds[cmd_id].cmd_id;
			count = gCmds[cmd_id].elem_count;
			
			switch(gCmds[cmd_id].cmd_type) {
			case type_None:
				if (cmd.cmdID == obj_End)
					stop = true;
				TXT_MAP_str_scan_eoln(cur_ptr, end_ptr, NULL);
				break;
			case type_PtLine:
			
				TXT_MAP_str_scan_eoln(cur_ptr, end_ptr, NULL);
				while (count--)
				{
					vrgb.v[0] = TXT_MAP_flt_scan(cur_ptr, end_ptr, Xtrue);
					vrgb.v[1] = TXT_MAP_flt_scan(cur_ptr, end_ptr, Xtrue);
					vrgb.v[2] = TXT_MAP_flt_scan(cur_ptr, end_ptr, Xtrue);
					vrgb.rgb[0] = TXT_MAP_flt_scan(cur_ptr, end_ptr, Xtrue);
					vrgb.rgb[1] = TXT_MAP_flt_scan(cur_ptr, end_ptr, Xtrue);
					vrgb.rgb[2] = TXT_MAP_flt_scan(cur_ptr, end_ptr, Xtrue);
					
					cmd.rgb.push_back(vrgb);
				}
				outObj.cmds.push_back(cmd);
				TXT_MAP_str_scan_eoln(cur_ptr, end_ptr, NULL);				
				break;
				
			case type_Poly:

				if (count == 0) count = TXT_MAP_int_scan(cur_ptr, end_ptr, Xfals);
				TXT_MAP_str_scan_eoln(cur_ptr, end_ptr, NULL);

				while (count--)
				{
					vst.v[0] = TXT_MAP_flt_scan(cur_ptr, end_ptr, Xtrue);
					vst.v[1] = TXT_MAP_flt_scan(cur_ptr, end_ptr, Xtrue);
					vst.v[2] = TXT_MAP_flt_scan(cur_ptr, end_ptr, Xtrue);
					vst.st[0] = TXT_MAP_flt_scan(cur_ptr, end_ptr, Xtrue);
					vst.st[1] = TXT_MAP_flt_scan(cur_ptr, end_ptr, Xtrue);
					
					cmd.st.push_back(vst);
				}
				outObj.cmds.push_back(cmd);
				TXT_MAP_str_scan_eoln(cur_ptr, end_ptr, NULL);
				break;
			case type_Attr:
				
				for (int n = 0; n < count; ++n)
					cmd.attributes.push_back(TXT_MAP_flt_scan(cur_ptr, end_ptr, Xfals));
					
				outObj.cmds.push_back(cmd);
				TXT_MAP_str_scan_eoln(cur_ptr, end_ptr, NULL);
				break;
			}

		}	// Obj 7 Paraser

	} // While loop
	
	free(mem_buf);
	return true;
}

/****************************************************************************************
 * OBJ 7 WRITE
 ****************************************************************************************/

bool	XObjWrite(const char * inFile, const XObj& inObj)
{
	FILE * fi = fopen(inFile, "w");
	if (!fi) return false;

	fprintf(fi,"%c" CRLF, APL ? 'A' : 'I');
	fprintf(fi,"700" CRLF);
	fprintf(fi,"OBJ" CRLF CRLF);
	fprintf(fi,"%s\t\t//" CRLF CRLF, inObj.texture.c_str());
	
	for (vector<XObjCmd>::const_iterator iter = inObj.cmds.begin(); iter != inObj.cmds.end(); ++iter)
	{
		int 	index	= FindIndexForCmd(iter->cmdID);
		switch(iter->cmdType) {
		case type_PtLine:
		
			if (gCmds[index].elem_count == 0)
				fprintf(fi,"%s %d\t\t//" CRLF, gCmds[index].name, iter->rgb.size());
			else
				fprintf(fi,"%s\t\t//" CRLF, gCmds[index].name);
			
			for (vector<vec_rgb>::const_iterator riter = iter->rgb.begin();
				riter != iter->rgb.end(); ++riter)
			{
				fprintf(fi,"%f %f %f    %f %f %f" CRLF,
					riter->v[0], riter->v[1], riter->v[2],
					riter->rgb[0], riter->rgb[1], riter->rgb[2]);
			}
			fprintf(fi,CRLF);
			break;
			
			
		case type_Poly:
		
			if (gCmds[index].elem_count == 0)
				fprintf(fi,"%s %d\t\t//" CRLF, gCmds[index].name, iter->st.size());
			else
				fprintf(fi,"%s\t\t//" CRLF, gCmds[index].name);
			
			for (vector<vec_tex>::const_iterator siter = iter->st.begin();
				siter != iter->st.end(); ++siter)
			{
				fprintf(fi,"%f %f %f    %f %f" CRLF,
					siter->v[0], siter->v[1], siter->v[2],
					siter->st[0], siter->st[1]);
			}
			fprintf(fi,CRLF);
			break;

		
		case type_Attr:
			fprintf(fi,"%s",gCmds[index].name);
			for (vector<float>::const_iterator aiter = iter->attributes.begin();
				aiter != iter->attributes.end(); ++aiter)
			{
				fprintf(fi," %f", *aiter);
			}
			fprintf(fi, "\t\t//" CRLF CRLF);
			break;
		}	
	}

	fprintf(fi,"end\t\t//" CRLF);
	
	fclose(fi);
	return true;
}

/****************************************************************************************
 * OBJ 8 READ
 ****************************************************************************************/
bool	XObj8Read(const char * inFile, XObj8& outObj)
{
		int 	n;

	outObj.texture.clear();
	outObj.texture_lit.clear();
	outObj.indices.clear();
	outObj.geo_tri.clear(8);
	outObj.geo_lines.clear(6);
	outObj.geo_lights.clear(6);
	outObj.animation.clear();
	outObj.lods.clear();

	/*********************************************************************
	 * READ FILE INTO MEM
	 *********************************************************************/
	
	FILE * objFile = fopen(inFile, "rb");
	if (!objFile) return false;
	fseek(objFile,0L,SEEK_END);
	int filesize = ftell(objFile);
	fseek(objFile,0L,SEEK_SET);
	unsigned char * mem_buf = (unsigned char *) malloc(filesize);
	if (mem_buf == NULL) { fclose(objFile); return false; }
	if (fread(mem_buf, 1, filesize, objFile) != filesize)
	{
		free(mem_buf); fclose(objFile); return false;
	}
	fclose(objFile);

	/*********************************************************************
	 * READ HEADER
	 *********************************************************************/
	
	unsigned char *	cur_ptr = mem_buf;
	unsigned char *	end_ptr = mem_buf + filesize;
	
	// LINE 1: A/I - who cares?!?
	TXT_MAP_str_scan_eoln(cur_ptr, end_ptr, NULL);
	
	// LINE 2: version
	int vers = TXT_MAP_int_scan(cur_ptr, end_ptr, Xfals);
	TXT_MAP_str_scan_eoln(cur_ptr, end_ptr, NULL);
	
	// LINE 3: "OBJ"
	if (!TXT_MAP_str_match_space(cur_ptr, end_ptr, "OBJ", Xtrue))
		vers = 0;
	TXT_MAP_str_scan_eoln(cur_ptr, end_ptr, NULL);
	
	// If we don't have a good version, bail.
	if (vers != 800)
	{
		free(mem_buf);
		return false;
	}
			
	/************************************************************
	 * READ GEOMETRIC COMMANDS
	 ************************************************************/

	bool	stop = false;
	
	int trimax, linemax, lightmax, idxmax;
	int tricount = 0, linecount = 0, lightcount = 0, idxcount = 0;
	float	stdat[8];
	
	XObjCmd8	cmd;
	XObjAnim8	animation;

	outObj.lods.push_back(XObjLOD8());
	outObj.lods.back().lod_near = outObj.lods.back().lod_far = 0;

	while (!stop && TXT_MAP_continue(cur_ptr, end_ptr))
	{
		// TEXTURE <tex>
		if (TXT_MAP_str_match_space(cur_ptr, end_ptr, "TEXTURE", Xfals))
		{
			TXT_MAP_str_scan_space(cur_ptr, end_ptr, &outObj.texture);
		}
		// TEXTURE_LIT <tex>
		else if (TXT_MAP_str_match_space(cur_ptr, end_ptr, "TEXTURE_LIT", Xfals))
		{
			TXT_MAP_str_scan_space(cur_ptr, end_ptr, &outObj.texture_lit);
		} 
		// POINT_COUNTS tris lines lites geo indices
		else if (TXT_MAP_str_match_space(cur_ptr, end_ptr, "POINT_COUNTS", Xfals))
		{
			trimax = TXT_MAP_flt_scan(cur_ptr, end_ptr, Xfals);
			linemax = TXT_MAP_flt_scan(cur_ptr, end_ptr, Xfals);
			lightmax = TXT_MAP_flt_scan(cur_ptr, end_ptr, Xfals);
			idxmax = TXT_MAP_flt_scan(cur_ptr, end_ptr, Xfals);

			outObj.indices.resize(idxmax);
			outObj.geo_tri.clear(8);
			outObj.geo_tri.resize(trimax);
			outObj.geo_lines.clear(6);
			outObj.geo_lines.resize(linemax);
			outObj.geo_lights.clear(6);
			outObj.geo_lights.resize(lightmax);
		}
		// VT <x> <y> <z> <nx> <ny> <nz> <s> <t>
		else if (TXT_MAP_str_match_space(cur_ptr, end_ptr, "VT", Xfals))
		{
			if (tricount >= trimax) break;
			stdat[0] = TXT_MAP_flt_scan(cur_ptr, end_ptr, Xfals);
			stdat[1] = TXT_MAP_flt_scan(cur_ptr, end_ptr, Xfals);
			stdat[2] = TXT_MAP_flt_scan(cur_ptr, end_ptr, Xfals);
			stdat[3] = TXT_MAP_flt_scan(cur_ptr, end_ptr, Xfals);
			stdat[4] = TXT_MAP_flt_scan(cur_ptr, end_ptr, Xfals);
			stdat[5] = TXT_MAP_flt_scan(cur_ptr, end_ptr, Xfals);
			stdat[6] = TXT_MAP_flt_scan(cur_ptr, end_ptr, Xfals);
			stdat[7] = TXT_MAP_flt_scan(cur_ptr, end_ptr, Xfals);
			outObj.geo_tri.set(tricount++, stdat);
		}
		// VLINE <x> <y> <z> <r> <g> <b>
		else if (TXT_MAP_str_match_space(cur_ptr, end_ptr, "VLINE", Xfals))
		{
			if (linecount >= linemax) break;
			stdat[0] = TXT_MAP_flt_scan(cur_ptr, end_ptr, Xfals);
			stdat[1] = TXT_MAP_flt_scan(cur_ptr, end_ptr, Xfals);
			stdat[2] = TXT_MAP_flt_scan(cur_ptr, end_ptr, Xfals);
			stdat[3] = TXT_MAP_flt_scan(cur_ptr, end_ptr, Xfals);
			stdat[4] = TXT_MAP_flt_scan(cur_ptr, end_ptr, Xfals);
			stdat[5] = TXT_MAP_flt_scan(cur_ptr, end_ptr, Xfals);
			outObj.geo_lines.set(linecount++, stdat);
		}
		// VLIGHT <x> <y> <z> <r> <g> <b>
		else if (TXT_MAP_str_match_space(cur_ptr, end_ptr, "VLIGHT", Xfals))
		{
			if (lightcount >= lightmax) break;
			stdat[0] = TXT_MAP_flt_scan(cur_ptr, end_ptr, Xfals);
			stdat[1] = TXT_MAP_flt_scan(cur_ptr, end_ptr, Xfals);
			stdat[2] = TXT_MAP_flt_scan(cur_ptr, end_ptr, Xfals);
			stdat[3] = TXT_MAP_flt_scan(cur_ptr, end_ptr, Xfals);
			stdat[4] = TXT_MAP_flt_scan(cur_ptr, end_ptr, Xfals);
			stdat[5] = TXT_MAP_flt_scan(cur_ptr, end_ptr, Xfals);
			outObj.geo_lights.set(lightcount++, stdat);
		}
		// IDX <n>
		else if (TXT_MAP_str_match_space(cur_ptr, end_ptr, "IDX", Xfals))
		{
			if (idxcount >= idxmax) break;
			outObj.indices[idxcount++] = TXT_MAP_flt_scan(cur_ptr, end_ptr, Xfals);
		}
		// IDX10 <n> x 10
		else if (TXT_MAP_str_match_space(cur_ptr, end_ptr, "IDX10", Xfals))
		{			
			if (idxcount >= idxmax) break;
			for (n = 0; n < 10; ++n)
				outObj.indices[idxcount++] = TXT_MAP_flt_scan(cur_ptr, end_ptr, Xfals);
		}
		// TRIS offset count
		else if (TXT_MAP_str_match_space(cur_ptr, end_ptr, "TRIS", Xfals))
		{
			cmd.cmd = obj8_Tris;
			cmd.idx_offset = TXT_MAP_int_scan(cur_ptr, end_ptr, Xfals);
			cmd.idx_count = TXT_MAP_int_scan(cur_ptr, end_ptr, Xfals);
			outObj.lods.back().cmds.push_back(cmd);
		}
		// LINES offset count
		else if (TXT_MAP_str_match_space(cur_ptr, end_ptr, "LINES", Xfals))
		{
			cmd.cmd = obj8_Lines;
			cmd.idx_offset = TXT_MAP_int_scan(cur_ptr, end_ptr, Xfals);
			cmd.idx_count = TXT_MAP_int_scan(cur_ptr, end_ptr, Xfals);
			outObj.lods.back().cmds.push_back(cmd);
		}
		// LIGHTS offset count
		else if (TXT_MAP_str_match_space(cur_ptr, end_ptr, "LIGHTS", Xfals))
		{
			cmd.cmd = obj8_Lights;
			cmd.idx_offset = TXT_MAP_int_scan(cur_ptr, end_ptr, Xfals);
			cmd.idx_count = TXT_MAP_int_scan(cur_ptr, end_ptr, Xfals);
			outObj.lods.back().cmds.push_back(cmd);
		}
		// ATTR_LOD near far
		else if (TXT_MAP_str_match_space(cur_ptr, end_ptr, "ATTR_LOD", Xtrue))
		{
			if (outObj.lods.back().lod_far != 0)	outObj.lods.push_back(XObjLOD8());
			outObj.lods.back().lod_near = TXT_MAP_int_scan(cur_ptr, end_ptr, Xfals);
			outObj.lods.back().lod_far = TXT_MAP_int_scan(cur_ptr, end_ptr, Xfals);
		} 
		// ANIM_rotate x y z r1 r2 v1 v2 dref
		else if (TXT_MAP_str_match_space(cur_ptr, end_ptr, "ANIM_rotate", Xfals))
		{
			cmd.cmd = anim_Rotate;
			cmd.idx_offset = outObj.animation.size();			
			outObj.lods.back().cmds.push_back(cmd);
			animation.xyzrv2[0] = animation.xyzrv1[0] = TXT_MAP_flt_scan(cur_ptr, end_ptr, Xfals);
			animation.xyzrv2[1] = animation.xyzrv1[1] = TXT_MAP_flt_scan(cur_ptr, end_ptr, Xfals);
			animation.xyzrv2[2] = animation.xyzrv1[2] = TXT_MAP_flt_scan(cur_ptr, end_ptr, Xfals);
			animation.xyzrv1[3] = TXT_MAP_flt_scan(cur_ptr, end_ptr, Xfals);
			animation.xyzrv2[3] = TXT_MAP_flt_scan(cur_ptr, end_ptr, Xfals);
			animation.xyzrv1[4] = TXT_MAP_flt_scan(cur_ptr, end_ptr, Xfals);
			animation.xyzrv2[4] = TXT_MAP_flt_scan(cur_ptr, end_ptr, Xfals);
			TXT_MAP_str_scan_space(cur_ptr, end_ptr, &animation.dataref);
			outObj.animation.push_back(animation);
		}
		// ANIM_translate x1 y1 z1 x2 y2 z2 v1 v2 dref
		else if (TXT_MAP_str_match_space(cur_ptr, end_ptr, "ANIM_translate", Xfals))
		{
			cmd.cmd = anim_Translate;
			cmd.idx_offset = outObj.animation.size();
			outObj.lods.back().cmds.push_back(cmd);
			animation.xyzrv1[0] = TXT_MAP_flt_scan(cur_ptr, end_ptr, Xfals);
			animation.xyzrv1[1] = TXT_MAP_flt_scan(cur_ptr, end_ptr, Xfals);
			animation.xyzrv1[2] = TXT_MAP_flt_scan(cur_ptr, end_ptr, Xfals);
			animation.xyzrv2[0] = TXT_MAP_flt_scan(cur_ptr, end_ptr, Xfals);
			animation.xyzrv2[1] = TXT_MAP_flt_scan(cur_ptr, end_ptr, Xfals);
			animation.xyzrv2[2] = TXT_MAP_flt_scan(cur_ptr, end_ptr, Xfals);
			animation.xyzrv1[4] = TXT_MAP_flt_scan(cur_ptr, end_ptr, Xfals);
			animation.xyzrv2[4] = TXT_MAP_flt_scan(cur_ptr, end_ptr, Xfals);
			TXT_MAP_str_scan_space(cur_ptr, end_ptr, &animation.dataref);
			outObj.animation.push_back(animation);
		}
		// ANIM_begin
		else if (TXT_MAP_str_match_space(cur_ptr, end_ptr, "ANIM_begin", Xfals))
		{
			cmd.cmd = anim_Begin;
			outObj.lods.back().cmds.push_back(cmd);
		}
		// ANIM_end
		else if (TXT_MAP_str_match_space(cur_ptr, end_ptr, "ANIM_end", Xfals))
		{
			cmd.cmd = anim_End;
			outObj.lods.back().cmds.push_back(cmd);
		}
		else
		// Common attribute handling:
		{
			string	attr_name;
			TXT_MAP_str_scan_space(cur_ptr, end_ptr, &attr_name);
			int cmd_idx = FindObjCmd(attr_name.c_str(), true);
			if (cmd_idx != attr_Max)
			{
				cmd.cmd = gCmds[cmd_idx].cmd_id;
				for (n = 0; n < gCmds[cmd_idx].elem_count; ++n)
				{
					cmd.params[n] = TXT_MAP_flt_scan(cur_ptr, end_ptr, Xfals);
				}
				outObj.lods.back().cmds.push_back(cmd);
			}			
			
		}
			
		TXT_MAP_str_scan_eoln(cur_ptr, end_ptr, NULL);
	} // While loop	
	
	free(mem_buf);
	return true;
}

/****************************************************************************************
 * OBJ 8 WRITE
 ****************************************************************************************/
bool	XObj8Write(const char * inFile, const XObj8& outObj)
{
	int n;
	
	FILE * fi = fopen(inFile, "w");
	if (fi == NULL) return false;
	const float * v;
	
	// HEADER
	fprintf(fi, "%c" CRLF "800" CRLF "OBJ" CRLF CRLF, APL ? 'A' : 'I');

	// TEXTURES	
									fprintf(fi, "TEXTURE %s" CRLF, outObj.texture.c_str());
	if (!outObj.texture_lit.empty())fprintf(fi, "TEXTURE_LIT %s" CRLF, outObj.texture_lit.c_str());

	// POINT POOLS

	fprintf(fi, "POINT_COUNTS %d %d %d %d" CRLF, outObj.geo_tri.count(), outObj.geo_lines.count(), outObj.geo_lights.count(), outObj.indices.size());

	for (n = 0; n < outObj.geo_tri.count(); ++n)
	{
		v = outObj.geo_tri.get(n);
		fprintf(fi, "VT %f %f %f %f %f %f %f %f" CRLF, v[0], v[1], v[2], v[3], v[4], v[5], v[6], v[7]);
	}

	for (n = 0; n < outObj.geo_lines.count(); ++n)
	{
		v = outObj.geo_lines.get(n);
		fprintf(fi, "VLINE %f %f %f %f %f %f" CRLF, v[0], v[1], v[2], v[3], v[4], v[5]);
	}

	for (n = 0; n < outObj.geo_lights.count(); ++n)
	{
		v = outObj.geo_lights.get(n);
		fprintf(fi, "VLIGHT %f %f %f %f %f %f" CRLF, v[0], v[1], v[2], v[3], v[4], v[5]);
	}

	int extra = outObj.indices.size() % 10;
	int trans = outObj.indices.size() - extra;

	for (n = 0; n < outObj.indices.size(); ++n)
	{
		if (n >= trans) fprintf(fi, "IDX");
		else if ((n % 10) == 0) fprintf(fi, "IDX10");
		fprintf(fi, " %d", outObj.indices[n]);
		if (n >= trans) fprintf(fi, "\n");
		else if ((n % 10) == 9) fprintf(fi, "\n");
	}

	// CMDS
	
	for (vector<XObjLOD8>::const_iterator lod = outObj.lods.begin(); lod != outObj.lods.end(); ++lod)
	{
		if (lod->lod_far != 0.0)
		{
			fprintf(fi, "ATTR_LOD %f %f" CRLF, lod->lod_near, lod->lod_far);
		}
		
		for (vector<XObjCmd8>::const_iterator cmd = lod->cmds.begin(); cmd != lod->cmds.end(); ++cmd)
		{
			switch(cmd->cmd) {
			case anim_Rotate:
				fprintf(fi, "ANIM_rotate %f %f %f %f %f %f %f %s" CRLF,
					outObj.animation[cmd->idx_offset].xyzrv1[0],
					outObj.animation[cmd->idx_offset].xyzrv1[1],
					outObj.animation[cmd->idx_offset].xyzrv1[2],
					outObj.animation[cmd->idx_offset].xyzrv1[3],
					outObj.animation[cmd->idx_offset].xyzrv2[3],
					outObj.animation[cmd->idx_offset].xyzrv1[4],
					outObj.animation[cmd->idx_offset].xyzrv2[4],
					outObj.animation[cmd->idx_offset].dataref.c_str());
				break;
			case anim_Translate:
				fprintf(fi, "ANIM_translate %f %f %f %f %f %f %f %s" CRLF,
					outObj.animation[cmd->idx_offset].xyzrv1[0],
					outObj.animation[cmd->idx_offset].xyzrv1[1],
					outObj.animation[cmd->idx_offset].xyzrv1[2],
					outObj.animation[cmd->idx_offset].xyzrv2[0],
					outObj.animation[cmd->idx_offset].xyzrv2[1],
					outObj.animation[cmd->idx_offset].xyzrv2[2],
					outObj.animation[cmd->idx_offset].xyzrv1[4],
					outObj.animation[cmd->idx_offset].xyzrv2[4],
					outObj.animation[cmd->idx_offset].dataref.c_str());
				break;
			case obj8_Tris:
				fprintf(fi, "TRIS %d %d" CRLF, cmd->idx_offset, cmd->idx_count);
				break;
			case obj8_Lines:
				fprintf(fi, "LINES %d %d" CRLF, cmd->idx_offset, cmd->idx_count);
				break;
			case obj8_Lights:
				fprintf(fi, "LIGHTS %d %d" CRLF, cmd->idx_offset, cmd->idx_count);
				break;
			default: 
				{
					int idx = FindIndexForCmd(cmd->cmd);
					if (idx != attr_Max)
					{
						fprintf(fi, "%s", gCmds[idx].name);
						for (n = 0; n < gCmds[idx].elem_count; ++n)
							fprintf(fi, " %f", cmd->params[n]);
						fprintf(fi, CRLF);
					}
				}
				break;
			}
		}
	}
	fclose(fi);
	return true;
}
