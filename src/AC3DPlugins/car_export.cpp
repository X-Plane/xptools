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

#if WANT_CAR_BONES
#include "XCarBoneUtils.h"
#endif

#if WANT_CAR_BONES
XCarBones	gBones;
#endif


#if WANT_CAR_BONES
void	matrix_transform_object_recursive(ACObject * ob, double m[16], set<ACObject *> * stopset)
{
	List * vertices = ac_object_get_vertexlist(ob);
	for (List * i = vertices; i != NULL; i = i->next)
	{
        Vertex *s = (Vertex *)i->data;

        double v1[4], v2[4];
        v1[0] = s->x;
        v1[1] = s->y;
        v1[2] = s->z;
        v1[3] = 1.0;

        multMatrixVec(v2, m, v1);
        Point3	p;
        p.x = v2[0];
        p.y = v2[1];
        p.z = v2[2];
        vertex_set_point(s, &p);
	}

	List * kids = ac_object_get_childrenlist(ob);
    for (List * p = kids; p != NULL; p = p->next)
    {
    	ACObject * child = (ACObject *)p->data;
    	if (stopset == NULL || stopset->find(child) == stopset->end())
	        matrix_transform_object_recursive(child, m, stopset);
	}
}

void	debone_hierarchy(ACObject * ob, set<ACObject *> * bones)
{
	if (gBones.IsValid(ob))
	{
		double	m[16];
		gBones.GetDeboneMatrix(ob, m);
		matrix_transform_object_recursive(ob, m, bones);
	}

	List * kids = ac_object_get_childrenlist(ob);
    for (List * p = kids; p != NULL; p = p->next)
    {
    	ACObject * child = (ACObject *)p->data;
		debone_hierarchy(child, bones);
	}
}

void	rebone_hierarchy(ACObject * ob, set<ACObject *> * bones)
{
	if (gBones.IsValid(ob))
	{
		double	m[16];
		gBones.GetReboneMatrix(ob, m);
		matrix_transform_object_recursive(ob, m, bones);
	}

	List * kids = ac_object_get_childrenlist(ob);
    for (List * p = kids; p != NULL; p = p->next)
    {
    	ACObject * child = (ACObject *)p->data;
		rebone_hierarchy(child, bones);
	}
}
#endif

#if WANT_CAR_BONES
int do_car_save(char * fname, ACObject * root)
{
	int n;
	vector<ACObject *>	objs;
	set<ACObject *>		bones;

	find_all_objects(root, objs);
	for (n = 0; n < objs.size(); ++n)
		if (gBones.IsValid(objs[n]))
			bones.insert(objs[n]);

	debone_hierarchy(root, &bones);

	string	path(fname);
	string::size_type p = path.find_last_of("\\/");
	if (p != path.npos) path.erase(p+1);

	for (set<ACObject*>::iterator iter = bones.begin(); iter != bones.end(); ++iter)
	{
		gObj.cmds.clear();
		gTexName.clear();
		gSmooth = true;
		gTwoSided = false;
	    obj7_output_object(*iter, &bones);
		obj7_reset_properties();
	    p = gTexName.find_last_of("\\/");
	    if (p != gTexName.npos) gTexName.erase(0,p+1);
	    gObj.texture = gTexName;
	    string	local_name = path + gBones.GetBoneName(*iter);
		if (!XObjWrite(local_name.c_str(), gObj))
	        {
	        message_dialog("can't open file '%s' for writing", local_name.c_str());
	        return 0;
	        }
	    printf("Wrote %s\n", local_name.c_str());
	}

	if (!WriteBonesToFile(fname, gBones))
        {
        message_dialog("can't open car file '%s' for writing", fname);
        return 0;
        }

	printf("Wrote %s\n", fname);

	rebone_hierarchy(root, &bones);
	return 1;
}

ACObject *	do_car_load(char *filename)
{
	if (!ReadBonesFromFile(filename, gBones))
	{
		message_dialog("can't read CAR file '%s'", filename);
		return NULL;
	}

	string	path(filename);
	string::size_type p = path.find_last_of("\\/");
	if (p != path.npos) path.erase(p+1);


	vector<ACObject *>	objs;

	for (int n = 0; n < gBones.bones.size(); ++n)
	{
		string	fullpath = path + gBones.bones[n].name;
		ACObject * ob = do_obj7_load((char *) fullpath.c_str());
		if (ob == NULL)
		{
			message_dialog("can't read OBJ7 file '%s'", fullpath.c_str());
			return NULL;
		}
		gBones.bones[n].ref = ob;
		objs.push_back(ob);
		printf("Loaded %s\n", fullpath.c_str());
	}

	gBones.RebuildIndex();

	ACObject * root = NULL;
	for (int n = 0; n < gBones.bones.size(); ++n)
	{
		void * me = gBones.bones[n].ref;
		void * parent = gBones.GetParent(me);
		if (parent && me)
		{
			printf("%s is a child of %s\n", gBones.GetBoneName(me).c_str(), gBones.GetBoneName(parent).c_str());
			object_reparent((ACObject*) me, (ACObject*) parent);
		}
		if (me && !parent)
		{
			printf("%s is a possible root.\n", gBones.GetBoneName(me).c_str());
			root = (ACObject*) me;
		}
	}

	set<ACObject *>		bones;

	for (int n = 0; n < objs.size(); ++n)
		if (gBones.IsValid(objs[n]))
			bones.insert(objs[n]);

	printf("Has %d bones total.\n", bones.size());

	if (root == NULL)
	{
		message_dialog("Car has no root!");
		return NULL;
	}
	rebone_hierarchy(root, &bones);
	return root;
}
#endif
