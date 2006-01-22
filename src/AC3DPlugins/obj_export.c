THIS FILE IS OBSOLETE
/** OBJ7 exporter for AC3D **/

/*
	For Andrew:

	TODO: 
	
	** make sure we build an object so we do not have a group with geometry
	
	FEATURES
 - Lights.
 - Materials.
 - It will not build quad strips, tri strips, or tri fans.
 - There is not yet a way to preview night lighting inside AC3D.
 - There is no way to access "special" quads such as movies, hard
   surfaces, etc.
 - Calc LOD
	more cleanup



CAR SUPPORT:

1. Load and save car commands.
2. The car hiearchy is represented by the ACObject hierarchy!
3. The bones data for each car is in the user string.  So as we read the car file itself
	and load each object, we translate all data but the parent by putting it in the user string.
	
THAT DATA IS:
Type
Animation_Type
Current_Animation_Percent(0.0, 0.5, 1.0)
Filename (that we came from, relative)
(parent is encoded via the parent object!)
Start XYZ,Rot
End XYZ, Rot
	
OR...is the data created via an external side hash table that maps ACObject*s to their data?

PREMODS:
*	recursive object save must take a stop set.
*	Debone and rebone code
		Deboning moves objects from their viewed positions to their canoncial positions.
		Reboning does the opposite.
		So...let's say the XYZ of a child is 10,10,10
			Deboning subtracts XYZ from the position.
			Reboning adds XYZ to their position.
		Let's say that a child's rotation is 10 degrees CCW around the Y axis.
			Deboning rotates CW around the Y axis.
			Reboning rotates CCW around the Y axis.
		When we debone, we debone the child first, then the parent.
		When we rebone, we rebone the parent first, then the child.
		Deboning and reboning are always done on a rooted hierarchy.
		
		When we debone, we translate first, then rotate
		When we rebone, we rotate first, then translate.
------------
glTranslate(x,y,z) moves the current origin to x,y,z.
So if x,y,z is positive, things will appear positive to where they are to be drawn.
We would use glTranslate with the real xyz delta when drawing a boned object.
we would do a translate first and rotate second.
we go from the top down.

Some functions to write:
Auto utils
Ã	- declare a boning table
!!	- read car to boning table
!!	- write boning table to car
Ã	- get matrices


ON EXPORT:

1. Traverse the hierarchy, making a list of all top level bone objects and their
	parents.  This will become the 'boning table'.  Every object also ends up in a
	bone set.

2. Debone the model.

3. When we do our save, we pass in the 'stop set' (we use the bone set for this)...the stop set
	is the set that we do not recurse into.
   For each object in the bone set, we export based on its bone file name 
4. we then write the boning table to the separate car file.
5. Rebone the object

ON IMPORT:
1. We read in the car object, building the boning table.
2. For each object in the boning table we read in the object from the file
3. We nest the objects appropriately based on the boning table nesting indices.
4. Rebone the object

TO CHANGE THE ANIMATION VALUES
1. Debone the objects.
2. Change the aniamion values
3. Rebone the objects
	

	
*/

#include "ac_plugin.h"
#include "undoable.h"

#ifdef Boolean
#undef Boolean
#endif

#include "obj_export.h"
#include "ac_utils.h"

#include "XObjDefs.h"
#include "XObjReadWrite.h"


float	gRepTexX = 1.0;
float	gRepTexY = 1.0;
float	gOffTexX = 0.0;
float	gOffTexY = 0.0;

XObj	gObj;
bool	gSmooth;
bool	gTwoSided;
bool	gIsCockpit;
string	gTexName;
int		gErrMissingTex;
int		gHasTexNow;
bool	gErrDoubleTex;
bool	gErrBadCockpit;
List *	gBadSurfaces;
List *	gBadObjects;



/* Utilities */
static void	DecomposeObjCmd(const XObjCmd& inCmd, vector<XObjCmd>& outCmds, int maxValence);
static void	DecomposeObj(const XObj& inObj, XObj& outObj, int maxValence);

/* OBJ7 import and export */
static void obj7_reset_properties();
static void obj7_output_triangle(Surface *s);
static void obj7_output_quad(Surface *s);
static void obj7_output_polyline(Surface *s);
static void obj7_output_polygon(Surface *s);


/***************************************************************************************************
 * UTILITIES
 ***************************************************************************************************/



void	DecomposeObjCmd(const XObjCmd& inCmd, vector<XObjCmd>& outCmds, int maxValence)
{
	XObjCmd	c;
	c.cmdType = type_Poly;
	c.cmdID = obj_Tri;
	switch(inCmd.cmdID) {
	case obj_Tri:
		// Triangles never need breaking down.
		outCmds.push_back(inCmd);
		break;
	case obj_Quad:
	case obj_Quad_Hard:
	case obj_Smoke_Black:
	case obj_Smoke_White:
	case obj_Movie:
		// Quads - split into triangles if necessary.
		if (maxValence > 3) {
			outCmds.push_back(inCmd);
			outCmds.back().cmdID = obj_Quad;
		} else {
			outCmds.push_back(inCmd);
			outCmds.back().cmdID = obj_Tri;
			outCmds.back().st.erase(outCmds.back().st.begin()+3);
			outCmds.push_back(inCmd);			
			outCmds.back().cmdID = obj_Tri;
			outCmds.back().st.erase(outCmds.back().st.begin()+1);
		}
		break;
	case obj_Polygon:
		// Polygons might be ok.  But if we have to break them down,
		// we generate N-2 triangles in a fan configuration.
		if (maxValence < inCmd.st.size())
		{
			c.st.push_back(inCmd.st[0]);
			c.st.push_back(inCmd.st[1]);
			c.st.push_back(inCmd.st[2]);
			for (int n = 2; n < inCmd.st.size(); ++n)
			{
				c.st[1] = inCmd.st[n-1];
				c.st[2] = inCmd.st[n  ];
				outCmds.push_back(c);
			}
		} else 
			outCmds.push_back(inCmd);
		break;
	case obj_Tri_Strip:
		// Triangle strips - every other triangle's vertices
		// are backward!
		c.st.push_back(inCmd.st[0]);
		c.st.push_back(inCmd.st[1]);
		c.st.push_back(inCmd.st[2]);
		for (int n = 2; n < inCmd.st.size(); ++n)
		{
			if (n%2)
			{
				c.st[0] = inCmd.st[n-2];
				c.st[1] = inCmd.st[n  ];
				c.st[2] = inCmd.st[n-1];
				outCmds.push_back(c);
			} else {
				c.st[0] = inCmd.st[n-2];
				c.st[1] = inCmd.st[n-1];
				c.st[2] = inCmd.st[n  ];
				outCmds.push_back(c);
			}
		}
		break;
	case obj_Tri_Fan:
		// Tri fan - run around the triangle fan emitting triangles.
		c.st.push_back(inCmd.st[0]);
		c.st.push_back(inCmd.st[1]);
		c.st.push_back(inCmd.st[2]);
		for (int n = 2; n < inCmd.st.size(); ++n)
		{
			c.st[1] = inCmd.st[n-1];
			c.st[2] = inCmd.st[n  ];
			outCmds.push_back(c);
		}
		break;
	case obj_Quad_Strip:
		// Quad strips can become either quads or triangles!!
		if (maxValence > 3)
		{
			c.cmdID = obj_Quad;
			c.st.push_back(inCmd.st[0]);
			c.st.push_back(inCmd.st[1]);
			c.st.push_back(inCmd.st[2]);
			c.st.push_back(inCmd.st[3]);
			for (int n = 2; n < inCmd.st.size(); n += 2)
			{
				c.st[0] = inCmd.st[n-2];
				c.st[1] = inCmd.st[n-1];
				c.st[2] = inCmd.st[n+1];
				c.st[3] = inCmd.st[n  ];
				outCmds.push_back(c);
			}
		} else {
			c.st.push_back(inCmd.st[0]);
			c.st.push_back(inCmd.st[1]);
			c.st.push_back(inCmd.st[2]);
			for (int n = 2; n < inCmd.st.size(); ++n)
			{
				if (n%2)
				{
					c.st[0] = inCmd.st[n-2];
					c.st[1] = inCmd.st[n  ];
					c.st[2] = inCmd.st[n-1];
					outCmds.push_back(c);
				} else {
					c.st[0] = inCmd.st[n-2];
					c.st[1] = inCmd.st[n-1];
					c.st[2] = inCmd.st[n  ];
					outCmds.push_back(c);
				}
			}
		}
		break;
	default:
		outCmds.push_back(inCmd);
	}
}

void	DecomposeObj(const XObj& inObj, XObj& outObj, int maxValence)
{
	outObj.cmds.clear();
	outObj.texture = inObj.texture;
	for (vector<XObjCmd>::const_iterator cmd = inObj.cmds.begin(); 
		cmd != inObj.cmds.end(); ++cmd)
	{
		vector<XObjCmd>		newCmds;
		DecomposeObjCmd(*cmd, newCmds, maxValence);
		outObj.cmds.insert(outObj.cmds.end(), newCmds.begin(), newCmds.end());
	}
}



/***************************************************************************************************
 * OBJ7 IMPORT AND EXPORT
 ***************************************************************************************************/

void obj7_reset_properties(void)
{
	XObjCmd	cmd;
	cmd.cmdType = type_Attr;
	cmd.cmdID = attr_Shade_Smooth;
	if (!gSmooth)
		gObj.cmds.push_back(cmd);
	gSmooth = true;
	cmd.cmdID = attr_Cull;
	if (gTwoSided)
		gObj.cmds.push_back(cmd);
	gTwoSided = false;
}

void obj7_output_triangle(Surface *s)
{
	if (gIsCockpit)
	{
		gErrBadCockpit = true;
		list_add_item_head(&gBadSurfaces, s);
	}
		
Vertex *p1, *p2, *p3;
SVertex * s1, * s2, *s3;
//int col;

    p1 = SVERTEX(s->vertlist->data);
    p2 = SVERTEX(s->vertlist->next->data);
    p3 = SVERTEX(s->vertlist->next->next->data);

	s1 = ((SVertex *)s->vertlist->data);
	s2 = ((SVertex *)s->vertlist->next->data);
	s3 = ((SVertex *)s->vertlist->next->next->data);

	XObjCmd	cmd;
	cmd.cmdType = type_Poly;
	cmd.cmdID = obj_Tri;
	vec_tex	st;
	
	st.v[0]  = p3->x;
	st.v[1]  = p3->y;
	st.v[2]  = p3->z;
	st.st[0] = s3->tx;
	st.st[1] = s3->ty;
	st.st[0] *= gRepTexX;
	st.st[0] += gOffTexX;
	st.st[1] *= gRepTexY;
	st.st[1] += gOffTexY;
	cmd.st.push_back(st);

	st.v[0]  = p2->x;
	st.v[1]  = p2->y;
	st.v[2]  = p2->z;
	st.st[0] = s2->tx;
	st.st[1] = s2->ty;
	st.st[0] *= gRepTexX;
	st.st[0] += gOffTexX;
	st.st[1] *= gRepTexY;
	st.st[1] += gOffTexY;
	cmd.st.push_back(st);

	st.v[0]  = p1->x;
	st.v[1]  = p1->y;
	st.v[2]  = p1->z;
	st.st[0] = s1->tx;
	st.st[1] = s1->ty;	
	st.st[0] *= gRepTexX;
	st.st[0] += gOffTexX;
	st.st[1] *= gRepTexY;
	st.st[1] += gOffTexY;
	cmd.st.push_back(st);

	gObj.cmds.push_back(cmd);
}

void obj7_output_quad(Surface *s)
{
Vertex *p1, *p2, *p3, *p4;
SVertex * s1, * s2, *s3, *s4;

    p1 = SVERTEX(s->vertlist->data);
    p2 = SVERTEX(s->vertlist->next->data);
    p3 = SVERTEX(s->vertlist->next->next->data);
    p4 = SVERTEX(s->vertlist->next->next->next->data);

	s1 = ((SVertex *)s->vertlist->data);
	s2 = ((SVertex *)s->vertlist->next->data);
	s3 = ((SVertex *)s->vertlist->next->next->data);
	s4 = ((SVertex *)s->vertlist->next->next->next->data);

	XObjCmd	cmd;
	cmd.cmdType = type_Poly;
	cmd.cmdID = gIsCockpit ? obj_Quad_Cockpit : obj_Quad;
	vec_tex	st;
	
	st.v[0]  = p4->x;
	st.v[1]  = p4->y;
	st.v[2]  = p4->z;
	st.st[0] = s4->tx;
	st.st[1] = s4->ty;
	st.st[0] *= gRepTexX;
	st.st[0] += gOffTexX;
	st.st[1] *= gRepTexY;
	st.st[1] += gOffTexY;
	cmd.st.push_back(st);

	
	
	st.v[0]  = p3->x;
	st.v[1]  = p3->y;
	st.v[2]  = p3->z;
	st.st[0] = s3->tx;
	st.st[1] = s3->ty;
	st.st[0] *= gRepTexX;
	st.st[0] += gOffTexX;
	st.st[1] *= gRepTexY;
	st.st[1] += gOffTexY;
	cmd.st.push_back(st);

	st.v[0]  = p2->x;
	st.v[1]  = p2->y;
	st.v[2]  = p2->z;
	st.st[0] = s2->tx;
	st.st[1] = s2->ty;
	st.st[0] *= gRepTexX;
	st.st[0] += gOffTexX;
	st.st[1] *= gRepTexY;
	st.st[1] += gOffTexY;
	cmd.st.push_back(st);

	st.v[0]  = p1->x;
	st.v[1]  = p1->y;
	st.v[2]  = p1->z;
	st.st[0] = s1->tx;
	st.st[1] = s1->ty;	
	st.st[0] *= gRepTexX;
	st.st[0] += gOffTexX;
	st.st[1] *= gRepTexY;
	st.st[1] += gOffTexY;
	cmd.st.push_back(st);

	gObj.cmds.push_back(cmd);
}



void obj7_output_polyline(Surface *s)
{
Vertex *p1, *p2;
int n;
        XObjCmd	cmd;
        cmd.cmdType = type_PtLine;
        cmd.cmdID = obj_Line;
        vec_rgb v;
        v.rgb[0] = v.rgb[1] = v.rgb[2] = 0.0;

    for (n=0; n < s->numvert-1; n++)
        {
        p1 = SVERTEX(list_get_item(s->vertlist, n));
        p2 = SVERTEX(list_get_item(s->vertlist, n+1));
        v.v[0] = p1->x;
        v.v[1] = p1->y;
        v.v[2] = p1->z;
        cmd.rgb.push_back(v);
        v.v[0] = p2->x;
        v.v[1] = p2->y;
        v.v[2] = p2->z;
        cmd.rgb.push_back(v);
        gObj.cmds.push_back(cmd);
        cmd.rgb.clear();
        }

    if (surface_get_type(s) == SURFACE_CLOSEDLINE)
        {
        p1 = SVERTEX(list_get_item(s->vertlist, s->numvert-1));
        p2 = SVERTEX(list_get_item(s->vertlist, 0));
        v.v[0] = p1->x;
        v.v[1] = p1->y;
        v.v[2] = p1->z;
        cmd.rgb.push_back(v);
        v.v[0] = p2->x;
        v.v[1] = p2->y;
        v.v[2] = p2->z;
        cmd.rgb.push_back(v);
        gObj.cmds.push_back(cmd);
        }

}

void obj7_output_polygon(Surface *s)
{
	if (!gHasTexNow && !gIsCockpit)
		++gErrMissingTex;
	XObjCmd	cmd;
	cmd.cmdType = type_Attr;	
	bool	is_two_sided = surface_get_twosided(s);
	if (is_two_sided != gTwoSided)
	{
		if (is_two_sided)
			cmd.cmdID = attr_NoCull;
		else
			cmd.cmdID = attr_Cull;
		gObj.cmds.push_back(cmd);
		gTwoSided = is_two_sided;
	}
	
	bool	is_smooth = surface_get_shading(s);
	if (is_smooth != gSmooth)
	{
		if (is_smooth)
			cmd.cmdID = attr_Shade_Smooth;
		else
			cmd.cmdID = attr_Shade_Flat;
		gObj.cmds.push_back(cmd);
		gSmooth = is_smooth;		
	}

    if (s->numvert > 4)
       {
       /** triangluate the polygon - this returns a list of new surfaces
           which must be freed (along with the list) after use **/
       List *slist = (List *)surface_get_triangulations(s);
       if (slist != NULL)
           {
           List *t;

           for (t = slist; t != NULL; t = t->next)
               {
               obj7_output_triangle((Surface *)t->data); 
               surface_free((Surface *)t->data);
               }
           list_free(&slist);
           }
        }
    else
        if (s->numvert == 4)
            obj7_output_quad(s); 
    else
        if (s->numvert == 3)
            obj7_output_triangle(s); 

}



void obj7_output_object(ACObject *ob, set<ACObject *> * stopset)
{
int numvert, numsurf, numkids;
List *vertices, *surfaces, *kids;
List *p;

    printf("outputing %s\n", ac_object_get_name(ob));

    ac_object_get_contents(ob, &numvert, &numsurf, &numkids,
        &vertices, &surfaces, &kids); 

	float	lod_start, lod_end;
	if (sscanf(ac_object_get_name(ob), "LOD %f/%f",&lod_start, &lod_end)==2)
	{
		obj7_reset_properties();
		XObjCmd	cmd;
		cmd.cmdType = type_Attr;
		cmd.cmdID = attr_LOD;
		cmd.attributes.push_back(lod_start);
		cmd.attributes.push_back(lod_end);
		gObj.cmds.push_back(cmd);		
	}
	
	bool bad_obj = false;
	
	if (ac_object_has_texture(ob))
	{	
		string tex = texture_id_to_name(ac_object_get_texture_index(ob));
		gHasTexNow = true;
		if (strstrnocase(tex.c_str(), "cockpit/-PANELS-/panel."))
			gIsCockpit = true;
		else
			gIsCockpit = false;
		if (!gIsCockpit)
		{
			if (tex != gTexName && !gTexName.empty())
			{
				gErrDoubleTex = true;
				list_add_item_head(&gBadObjects, ob);
				bad_obj = true;
			} 
			gTexName = tex;
		}
	} else {
		gIsCockpit = false;
		gHasTexNow = false;
	}

	gRepTexX = ac_object_get_texture_repeat_x(ob);
	gRepTexY = ac_object_get_texture_repeat_y(ob);
	gOffTexX = ac_object_get_texture_offset_x(ob);
	gOffTexY = ac_object_get_texture_offset_y(ob);
        
    int no_tex_count = gErrMissingTex;
    for (p = surfaces; p != NULL; p = p->next)
    {
        Surface *s = (Surface *)p->data;
        if (surface_get_type(s) == SURFACE_POLYGON)
            obj7_output_polygon(s);
        else
            obj7_output_polyline(s);
    }
    
    if (no_tex_count < gErrMissingTex && !bad_obj)
    	list_add_item_head(&gBadObjects, ob);

    for (p = kids; p != NULL; p = p->next)
    {
    	ACObject * child = (ACObject *)p->data;
    	if (stopset == NULL || stopset->find(child) == stopset->end())
	        obj7_output_object(child, stopset);
	}
}


int do_obj7_save(char * fname, ACObject * obj)
{
	gObj.cmds.clear();	
	gTexName.clear();
	gErrMissingTex = 0;
	gHasTexNow = false;
	gErrDoubleTex = false;
	gErrBadCockpit = false;
	gBadSurfaces = NULL;
	gBadObjects = NULL;

	gSmooth = true;
	gTwoSided = false;
	gIsCockpit = false;
    obj7_output_object(obj, NULL);
	obj7_reset_properties();
    
    string::size_type p = gTexName.find_last_of("\\/");
    if (p != gTexName.npos) gTexName.erase(0,p+1);
    if (gTexName.size() > 4)
	    gTexName.erase(gTexName.size() - 4);
    gObj.texture = gTexName;

	if (!XObjWrite(fname, gObj))
    {
        message_dialog("can't open file '%s' for writing", fname);
        return 0;
    }
    
    if (gErrMissingTex)
    	message_dialog("Warning: %d objects did not have texturse assigned.  You must assign a texture to every object for X-Plane output.", gErrMissingTex);
    if (gErrDoubleTex)
    	message_dialog("This model uses more than one texture.  You may only use one texture for an X-Plane OBJ.");
    if (gErrBadCockpit)
    	message_dialog("This model has non-quad surfaces that use the panel texture.  Only quad surfaces may use the panel texture.");
    if (gBadSurfaces)
    {    	
		clear_selection();
		ac_selection_select_surfacelist(gBadSurfaces);
		list_free(&gBadSurfaces);
		gBadSurfaces = NULL;
		redraw_all();    	
    }
    else if (gBadObjects)
    {
		clear_selection();
		ac_selection_select_objectlist(gBadObjects);
		list_free(&gBadObjects);
		gBadObjects = NULL;
		redraw_all();    	
    }
    
    return 1;
}



ACObject *	do_obj7_load(char *filename)
{
	Point3	p3;
	char	lodStrBuf[256];
	
	gObj.cmds.clear();
	if (!XObjRead(filename, gObj))
	{
//		message_dialog("can't read OBJ7 file '%s'", filename);
		return NULL;
	}
	
    ACObject * group = new_object(OBJECT_NORMAL);	
    string	fname(filename);
    string::size_type p = fname.find_last_of("\\/");	
    string justName = (p == fname.npos) ? fname : fname.substr(p+1);
    string justPath = fname.substr(0,p+1);
	string::size_type p2 = gObj.texture.find_last_of("\\/:");
	string texName = (p2 == gObj.texture.npos) ? gObj.texture : gObj.texture.substr(p2+1);    
    string texNameBmp = texName + ".bmp";
    string texNamePng = texName + ".png";    
    
    bool	has_cockpit_cmd = false;
	for(vector<XObjCmd>::iterator cmd = gObj.cmds.begin(); cmd != gObj.cmds.end(); ++cmd)
	{
    	if (cmd->cmdID == obj_Quad_Cockpit)
    	{
    		has_cockpit_cmd = true;
    		break;
    	}
    }
    object_set_name(group,(char *) justName.c_str());
    {
	    printf("Trying to load tex: %s\n", texNameBmp.c_str());
		if (!search_and_set_texture(group, filename, (char *) texNameBmp.c_str()))
		{
			search_and_set_texture(group, filename, (char *) texNamePng.c_str());
		}
	}
	
    ACObject * cockpit_group = NULL;
	ACObject * curLOD = NULL;
	ACObject * target = group;
    
    if (has_cockpit_cmd)
    {
    	cockpit_group = new_object(OBJECT_NORMAL);
    	object_set_name(cockpit_group, "Panel Textured quads");
    	curLOD = target = new_object(OBJECT_NORMAL);

    	object_set_name(target, "Cockpit Object Textured Geometry");
		if (!search_and_set_texture(target, filename, (char *) texNameBmp.c_str()))
			search_and_set_texture(target, filename, (char *) texNamePng.c_str());    	

    	string	cockpit_bitmap_name = "cockpit/-PANELS-/panel.png";
    	printf("Trying to set cockpit texture: %s\n", cockpit_bitmap_name.c_str());
    	if (!search_and_set_texture(cockpit_group, filename, (char *) cockpit_bitmap_name.c_str()))
    	{
    		cockpit_bitmap_name.erase(cockpit_bitmap_name.size() - 3);
    		cockpit_bitmap_name += "bmp";
	   	 	printf("Trying to set cockpit texture: %s\n", cockpit_bitmap_name.c_str());
	    	if (!search_and_set_texture(cockpit_group, filename, (char *) cockpit_bitmap_name.c_str()))
	    	{
	    		message_dialog("Unable to find custom panel texture: %s", cockpit_bitmap_name.c_str());
	    	}
	    }	    
    }
    
	
    
	XObj	obj;
	DecomposeObj(gObj,	obj, 100);
	
	bool	is_two_sided = false;
	bool	is_smooth = true;
	
	for(vector<XObjCmd>::iterator cmd = obj.cmds.begin(); cmd != obj.cmds.end(); ++cmd)
	{
		switch(cmd->cmdType) {
		case type_Attr:
			switch(cmd->cmdID) {
			case attr_Shade_Smooth:
				is_smooth = true;
				break;
			case attr_Shade_Flat:
				is_smooth = false;
				break;
			case attr_Cull:
				is_two_sided = false;
				break;
			case attr_NoCull:
				is_two_sided = true;
				break;
			case attr_LOD:
				if (curLOD)
				{
					object_add_child(group, curLOD);
				}
				curLOD = new_object(OBJECT_NORMAL);
				target = curLOD;
				sprintf(lodStrBuf, "LOD %f/%f",cmd->attributes[0], cmd->attributes[1]);
				object_set_name(target, lodStrBuf);
				if (!search_and_set_texture(curLOD, filename, (char *) texNameBmp.c_str()))
					search_and_set_texture(curLOD, filename, (char *) texNamePng.c_str());
				break;
			}
			break;
		case type_Poly:
			{
//				printf("Handling poly verts=%d  Is cockpit? %s\n", cmd->st.size(), (cmd->cmdID == obj_Quad_Cockpit) ? "yes" : "no");
				vector<Vertex *> verts;
				int i;
				for (i = 0; i < cmd->st.size(); ++i)
				{
					p3.x = cmd->st[i].v[0];
					p3.y = cmd->st[i].v[1];
					p3.z = cmd->st[i].v[2];
					verts.push_back(object_add_new_vertex_head((cmd->cmdID == obj_Quad_Cockpit) ? cockpit_group : target, &p3));
				}
				
		        Surface * s = new_surface();
				for (i = 0; i < cmd->st.size(); ++i)
				{
			        surface_add_vertex_head(s, verts[i], cmd->st[i].st[0], cmd->st[i].st[1]);
			    }
			    surface_set_type(s, SURFACE_POLYGON);
			    surface_set_twosided(s, is_two_sided);
			    surface_set_shading(s, is_smooth);
		        object_add_surface_head((cmd->cmdID == obj_Quad_Cockpit) ? cockpit_group : target, s);				
			}			
			// also set the texture appropriately.
			break;
		case type_PtLine:
			switch(cmd->cmdID) {
			case obj_Light:
				{
					p3.x = cmd->rgb[0].v[0];
					p3.y = cmd->rgb[0].v[1];
					p3.z = cmd->rgb[0].v[2];
					/*Vertex * lightVertex = */object_add_new_vertex_head(target, &p3);
				}
				break;
			case obj_Line:
				{
					p3.x = cmd->rgb[0].v[0];
					p3.y = cmd->rgb[0].v[1];
					p3.z = cmd->rgb[0].v[2];
					Vertex * lv1 = object_add_new_vertex_head(target, &p3);
					p3.x = cmd->rgb[1].v[0];
					p3.y = cmd->rgb[1].v[1];
					p3.z = cmd->rgb[1].v[2];
					Vertex * lv2 = object_add_new_vertex_head(target, &p3);
					Surface * s = new_surface();
					surface_add_vertex_head(s, lv1, 0, 0);
					surface_add_vertex_head(s, lv2, 0, 0);
				    surface_set_twosided(s, is_two_sided);
				    surface_set_shading(s, is_smooth);
				    surface_set_type(s, SURFACE_LINE);					
					object_add_surface_head(target, s);
				}
				break;
			}
			break;
		}
	}
	if (curLOD)
		object_add_child(group, curLOD);
	if (cockpit_group)
    	object_add_child(group, cockpit_group);
		

	object_calc_normals_force(group);

    return group;
}

