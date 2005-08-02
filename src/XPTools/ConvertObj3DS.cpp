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
 
 //
 //	Another 3DS lib can be found at: http://c3ds.sourceforge.net/
 //
 
#include "ConvertObj3DS.h"
#include "ObjUtils.h"

#include <lib3ds/file.h>
#include <lib3ds/vector.h>
#include <lib3ds/matrix.h>
#include <lib3ds/material.h>
#include <lib3ds/mesh.h>


bool	ReadObj3DS(const char * inFilePath, XObj& obj, bool inReversePoly)
{
	obj.texture = "no_texture";
	Lib3dsFile * f=lib3ds_file_load(inFilePath);
	obj.cmds.clear();
	XObjCmd	lodCmd;
	lodCmd.cmdID = attr_LOD;
	lodCmd.cmdType = type_Attr;
	lodCmd.attributes.resize(2);
	bool	success = false;
	if (f)
	{
		map<string, Lib3dsMaterial*>	materials;
		for (Lib3dsMaterial * material = f->materials; material ; material = material->next)
		{
			materials.insert(pair<string,Lib3dsMaterial*>(material->name, material));
		}
	
		lib3ds_file_eval(f,0.0);	// I think this sets the time of a specified motion sequence.

		// We could iterate the nodes on the file, they would reference the meshes by name and we'd fetch
		// them.  We would then apply the node's matrix and pivot point, as follows:

//			mesh=lib3ds_file_mesh_by_name(f, node->name);					
//			Lib3dsMatrix N,M,X;
//			lib3ds_matrix_copy(N, node->matrix);
//			lib3ds_matrix_translate_xyz(N, -d->pivot[0], -d->pivot[1], -d->pivot[2]);
//			lib3ds_matrix_copy(M, mesh->matrix);
//			lib3ds_matrix_inv(M);
//			lib3ds_matrix_mul(X,N,M);

		// Iterate across all of the meshes in the 3DS file.
		for (Lib3dsMesh * mesh = f->meshes; mesh; mesh = mesh->next)
		{
			if (sscanf(mesh->name,"LOD %f/%f",&lodCmd.attributes[0], &lodCmd.attributes[1])==2)
			{
				obj.cmds.push_back(lodCmd);
			}
			// Transform each point by the matrix...
			for (int pp=0; pp<mesh->points; ++pp)
			{
				Lib3dsVector	c;
				lib3ds_vector_transform(c,mesh->matrix, mesh->pointL[pp].pos);
				mesh->pointL[pp].pos[0] = c[0];
				mesh->pointL[pp].pos[1] = c[1];
				mesh->pointL[pp].pos[2] = c[2];
			}

			// 3DS is made entirely of triangles...de-index each face and emit it.
	        for (int p=0; p<mesh->faces; ++p) 
	        {
	        	float	s_offset = 0.0;
	        	float	t_offset = 0.0;
	        	float	s_scale = 1.0;
	        	float	t_scale = 1.0;

				Lib3dsFace *face=&mesh->faceL[p];
				
				if (materials.find(face->material) != materials.end())
				{
					Lib3dsMaterial * material = materials[face->material];
					s_offset = material->texture1_map.offset[0];
					t_offset = material->texture1_map.offset[1];
					s_scale = material->texture1_map.scale[0];
					t_scale = material->texture1_map.scale[1];
				}
	        
				vec_tex	vv;
				XObjCmd	cmd;
				cmd.cmdType = type_Poly;
				cmd.cmdID = obj_Tri;
				if (mesh->texelL)
				{
					vv.st[0] = s_offset + s_scale * mesh->texelL[face->points[0]][0];
					vv.st[1] = t_offset + t_scale * mesh->texelL[face->points[0]][1];
				} else
					vv.st[0] = vv.st[1] = 0.0;
				vv.v[0] =  mesh->pointL[face->points[0]].pos[0];
				vv.v[1] =  mesh->pointL[face->points[0]].pos[1];
				vv.v[2] =  mesh->pointL[face->points[0]].pos[2];
				cmd.st.push_back(vv);

				if (mesh->texelL)
				{					
					vv.st[0] = s_offset + s_scale * mesh->texelL[face->points[1]][0];
					vv.st[1] = t_offset + t_scale * mesh->texelL[face->points[1]][1];
				} else
					vv.st[0] = vv.st[1] = 0.0;
				vv.v[0] =  mesh->pointL[face->points[1]].pos[0];
				vv.v[1] =  mesh->pointL[face->points[1]].pos[1];
				vv.v[2] =  mesh->pointL[face->points[1]].pos[2];							
				cmd.st.push_back(vv);


				if (mesh->texelL)
				{					
					vv.st[0] = s_offset + s_scale * mesh->texelL[face->points[2]][0];
					vv.st[1] = t_offset + t_scale * mesh->texelL[face->points[2]][1];
				} else
					vv.st[0] = vv.st[1] = 0.0;
				vv.v[0] =  mesh->pointL[face->points[2]].pos[0];
				vv.v[1] =  mesh->pointL[face->points[2]].pos[1];
				vv.v[2] =  mesh->pointL[face->points[2]].pos[2];
				cmd.st.push_back(vv);
				
				if (inReversePoly)
					ChangePolyCmdCW(cmd);
				obj.cmds.push_back(cmd);
			}
		}			
	 	lib3ds_file_free(f);
	 	success = true;
	 	
	 }
	 return success;
}

void pool_get(ObjPointPool * pool, int idx, float xyz[3], float st[2])
{
	float * p = pool->get(idx);
	xyz[0] = p[0];
	xyz[1] = p[1];
	xyz[2] = p[2];
	st [0] = p[3];
	st [1] = p[4];
}

int pool_accumulate(ObjPointPool * pool, const float xyz[3], const float st[2])
{
	float d[5];
	d[0] = xyz[0];
	d[1] = xyz[1];
	d[2] = xyz[2];
	d[3] = st [0];
	d[4] = st [1];
	return pool->accumulate(d);
}

bool	WriteObj3DS(const char * inFilePath, const XObj& inObj, bool inReversePoly)
{
	ObjPointPool	pool;
	pool.clear(5);
	bool	success = false;
	Lib3dsFile * file = lib3ds_file_new();
	if (!file) return false;
	
	Lib3dsMesh * mesh;
	vector<int>	p1, p2, p3;
	static	char	meshName[256];
	sprintf(meshName,"Unnamed");
	
	XObj	obj;
	DecomposeObj(inObj,	obj, 3);
	
	for(vector<XObjCmd>::iterator cmd = obj.cmds.begin(); cmd != obj.cmds.end(); ++cmd)
	{
		switch(cmd->cmdType) {
		case type_Attr:
			switch(cmd->cmdID) {
			case attr_LOD:
				if (!p1.empty())
				{
					mesh = lib3ds_mesh_new(meshName);
					lib3ds_mesh_new_point_list(mesh, pool.count());
					lib3ds_mesh_new_texel_list(mesh, pool.count());
					lib3ds_mesh_new_face_list(mesh, p1.size());

					for (int i = 0; i < pool.count(); ++i)
						pool_get(&pool,i,mesh->pointL[i].pos,mesh->texelL[i]);
					for (int p = 0; p < p1.size(); ++p) {
						mesh->faceL[p].points[0] = p1[p];
						mesh->faceL[p].points[1] = p2[p];
						mesh->faceL[p].points[2] = p3[p];
					}
					
					lib3ds_file_insert_mesh(file, mesh);
					
					pool.clear(5);
					p1.clear();
					p2.clear();
					p3.clear();
				}
				sprintf(meshName,"LOD %f/%f\n",cmd->attributes[0], cmd->attributes[1]);
				break;
			}
			break;
		case type_Poly:
			if (inReversePoly)
				ChangePolyCmdCW(*cmd);		
			switch(cmd->cmdID) {
			case obj_Tri:
				p1.push_back(pool_accumulate(&pool,cmd->st[0].v, cmd->st[0].st));
				p2.push_back(pool_accumulate(&pool,cmd->st[1].v, cmd->st[1].st));
				p3.push_back(pool_accumulate(&pool,cmd->st[2].v, cmd->st[2].st));
				break;
			}
			break;
		}
	}

	if (!p1.empty())
	{
		mesh = lib3ds_mesh_new(meshName);
		lib3ds_mesh_new_point_list(mesh, pool.count());
		lib3ds_mesh_new_texel_list(mesh, pool.count());
		lib3ds_mesh_new_face_list(mesh, p1.size());

		for (int i = 0; i < pool.count(); ++i)
			pool_get(&pool,i,mesh->pointL[i].pos,mesh->texelL[i]);
		for (int p = 0; p < p1.size(); ++p) {
			mesh->faceL[p].points[0] = p1[p];
			mesh->faceL[p].points[1] = p2[p];
			mesh->faceL[p].points[2] = p3[p];
		}
		
		lib3ds_file_insert_mesh(file, mesh);
	}
	success = lib3ds_file_save(file, inFilePath);
	lib3ds_file_free(file);
	return success;
}
