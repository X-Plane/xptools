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
#include "ConvertObjDXF.h"
#include <dime/convert/layerData.h>
#include <dime/Input.h>
#include <dime/Output.h>
#include <dime/Model.h>
#include <dime/entities/3DFace.h>
#include <dime/entities/Line.h>
#include <dime/entities/Point.h>
#include <dime/sections/TablesSection.h>
#include <dime/sections/EntitiesSection.h>
#include <dime/tables/Table.h>
#include <dime/tables/LayerTable.h>
#include <dime/State.h>
#include <dime/convert/convert.h>

#include "ObjUtils.h"

bool	ReadObjDXF(const char * inFilePath, XObj& obj, bool inReversePoly)
{
	obj.texture = "no_texture";
	obj.cmds.clear();
	dimeInput	in;
	bool	success = false;
	if (in.setFile(inFilePath))
	{
		dimeModel	model;
		if (model.read(&in))
		{
			dxfConverter converter;
			converter.findHeaderVariables(model);
			converter.setFillmode(true);	
			converter.setLayercol(true);
			if (converter.doConvert(model))
			{				
				success = true;

				// The DXF importer gives us a bunch of layers.  Go through and merge them
				// down into one object by appending the commands together.
				for (int i = 0; i < 255; i++) 
				{
					dxfLayerData *	l = converter.getLayerData(i+1);

					// First fetch all of the points and make lights.  The points
					// are just a big array of triplets, easy.
					XObjCmd		lightCmd;
					lightCmd.cmdID = obj_Light;
					lightCmd.cmdType = type_PtLine;
					lightCmd.rgb.push_back(vec_rgb());
					lightCmd.rgb[0].rgb[0] = 1.0;
					lightCmd.rgb[0].rgb[1] = 1.0;
					lightCmd.rgb[0].rgb[2] = 1.0;
					
					for (int n = 0; n < l->points.count(); ++n)
					{
						dimeVec3f	v = l->points[n];
						lightCmd.rgb[0].v[0] =  v.x;
						lightCmd.rgb[0].v[1] =  v.y;
						lightCmd.rgb[0].v[2] =  v.z;
						obj.cmds.push_back(lightCmd);
					}
					
					// Now do all of the lines.
					XObjCmd		lineCmd;
					lineCmd.cmdType = type_PtLine;
					lineCmd.cmdID = obj_Line;
					for (int n = 0; n < l->lineindices.count(); ++n)
					{
						int index = l->lineindices[n];
						if (index == -1)
						{
							lineCmd.rgb.clear();
						} else {
							vec_rgb		rgb;
							dimeVec3f	pt;
							l->linebsp.getPoint(index, pt);
							rgb.v[0] =  pt.x;	rgb.rgb[0] = 1.0;
							rgb.v[1] =  pt.y;	rgb.rgb[1] = 1.0;
							rgb.v[2] =  pt.z;	rgb.rgb[2] = 1.0;

							if (lineCmd.rgb.size() > 1)
								lineCmd.rgb.erase(lineCmd.rgb.begin());
							lineCmd.rgb.push_back(rgb);
							
							if (lineCmd.rgb.size() == 2)
							{
								obj.cmds.push_back(lineCmd);
							}
						}
					}
					
					// Now do polygons.
					XObjCmd		polyCmd;
					polyCmd.cmdType = type_Poly;
					for (int n = 0; n < l->faceindices.count(); ++n)
					{
						int index = l->faceindices[n];
						if (index == -1)
						{
							// -1 indicates a face set terminator.  Send out the existing geometry.
							if (polyCmd.st.size() == 3)
								polyCmd.cmdID = obj_Tri;
							else if (polyCmd.st.size() == 4)
								polyCmd.cmdID = obj_Quad;
							else
								polyCmd.cmdID = obj_Polygon;
							
							if (inReversePoly)
								ChangePolyCmdCW(polyCmd);								
							obj.cmds.push_back(polyCmd);
							polyCmd.st.clear();								
						} else {
							// Accum one ST pt.
							dimeVec3f pt;
							l->facebsp.getPoint(index, pt);
							vec_tex	vv;
							vv.st[0] = vv.st[1] = 0.0;
							vv.v[0] =  pt.x;
							vv.v[1] =  pt.y;
							vv.v[2] =  pt.z;
							polyCmd.st.push_back(vv);
						}
					}						
				}
			}
		}
	}
	return success;
}

bool	WriteObjDXF(const char * inFilePath, const XObj& inObj, bool inReversePoly)
{
	dimeOutput out;
	out.setFilename(inFilePath);
	dimeModel model;

	dimeTablesSection * tables = new dimeTablesSection;
	model.insertSection(tables);

	dimeTable * layers = new dimeTable(NULL);

	int colnum = 1;
	dimeLayerTable * layer = new dimeLayerTable;
	layer->setLayerName("Basic", NULL);
	layer->setColorNumber(colnum++);
	// ???
	dimeParam param;
	param.string_data = "CONTINUOUS";
	layer->setRecord(6, param);
	param.int16_data = 64;
	layer->setRecord(70, param);
	// ???

	layer->registerLayer(&model);
	layers->insertTableEntry(layer);
	tables->insertTable(layers); 
	
	const dimeLayer * theLayer =  model.getLayer("Basic");

	dimeEntitiesSection * entities = new dimeEntitiesSection;
	model.insertSection(entities);
	

	static char lodStr[256];

	XObj	obj;
	DecomposeObj(inObj,	obj, 4);

	for(vector<XObjCmd>::iterator cmd = obj.cmds.begin(); cmd != obj.cmds.end(); ++cmd)
	{
		switch(cmd->cmdType) {
		case type_Attr:
			switch(cmd->cmdID) {
			case attr_LOD:
				sprintf(lodStr,"LOD %f/%f\n", cmd->attributes[0], cmd->attributes[1]);
				layer = new dimeLayerTable;
				layer->setLayerName(lodStr, NULL);
				layer->setColorNumber(colnum++);
				// ???
				dimeParam param;
				param.string_data = "CONTINUOUS";
				layer->setRecord(6, param);
				param.int16_data = 64;
				layer->setRecord(70, param);
				// ???

				layer->registerLayer(&model);
				layers->insertTableEntry(layer);				
				
				theLayer =  model.getLayer(lodStr);
				break;
			}
			break;
		case type_Poly:
			if (inReversePoly)
				ChangePolyCmdCW(*cmd);
			switch(cmd->cmdID) {
			case obj_Tri:
				{
					// filled, create dime3DFace
					int i;

					// DIME: create a 3DFACE entity, and set it to contain a triangle
					dime3DFace * face = new dime3DFace;
					if (theLayer) {
						face->setLayer(theLayer);
					}
					dimeVec3f v[3];

					for (i = 0; i < 3; i++) {
					v[i].x = cmd->st[i].v[0];
					v[i].y = cmd->st[i].v[1];
					v[i].z = cmd->st[i].v[2];
					}
					face->setTriangle(v[0], v[1], v[2]);

					// DIME: create a unique handle for this entity.
					const int BUFSIZE = 1024;
					char buf[BUFSIZE];
					const char * handle = model.getUniqueHandle(buf, BUFSIZE);

					dimeParam param;
					param.string_data = handle;
					face->setRecord(5, param);

					// DIME: add entity to model
					model.addEntity(face);
				}
				break;
			case obj_Quad:
				{
					// filled, create dime3DFace
					int i;

					// DIME: create a 3DFACE entity, and set it to contain a triangle
					dime3DFace * face = new dime3DFace;
					if (theLayer) {
						face->setLayer(theLayer);
					}
					dimeVec3f v[4];

					for (i = 0; i < 4; i++) {
					v[i].x = cmd->st[i].v[0];
					v[i].y = cmd->st[i].v[1];
					v[i].z = cmd->st[i].v[2];
					}
					face->setQuad(v[0], v[1], v[2], v[3]);

					// DIME: create a unique handle for this entity.
					const int BUFSIZE = 1024;
					char buf[BUFSIZE];
					const char * handle = model.getUniqueHandle(buf, BUFSIZE);

					dimeParam param;
					param.string_data = handle;
					face->setRecord(5, param);

					// DIME: add entity to model
					model.addEntity(face);
				}
				break;
			}
			break;
		case type_PtLine:
			switch(cmd->cmdID) {
			case obj_Line:
				{
					// filled, create dime3DFace
					int i;

					// DIME: create a 3DFACE entity, and set it to contain a triangle
					dimeLine * face = new dimeLine;
					if (theLayer) {
						face->setLayer(theLayer);
					}
					dimeVec3f v;

					for (i = 0; i < 2; i++) {
					v.x = cmd->rgb[i].v[0];
					v.y = cmd->rgb[i].v[1];
					v.z = cmd->rgb[i].v[2];
					face->setCoords(i, v);
					}

					// DIME: create a unique handle for this entity.
					const int BUFSIZE = 1024;
					char buf[BUFSIZE];
					const char * handle = model.getUniqueHandle(buf, BUFSIZE);

					dimeParam param;
					param.string_data = handle;
					face->setRecord(5, param);

					// DIME: add entity to model
					model.addEntity(face);
				}
				break;
			case obj_Light:
				{
					// filled, create dime3DFace
					int i;

					// DIME: create a 3DFACE entity, and set it to contain a triangle
					dimePoint * face = new dimePoint;
					if (theLayer) {
						face->setLayer(theLayer);
					}
					dimeVec3f v;

					v.x = cmd->rgb[0].v[0];
					v.y = cmd->rgb[0].v[1];
					v.z = cmd->rgb[0].v[2];
					face->setCoords(v);

					// DIME: create a unique handle for this entity.
					const int BUFSIZE = 1024;
					char buf[BUFSIZE];
					const char * handle = model.getUniqueHandle(buf, BUFSIZE);

					dimeParam param;
					param.string_data = handle;
					face->setRecord(5, param);

					// DIME: add entity to model
					model.addEntity(face);
				}
				break;
				
			}
			break;
		}
	}	
	
	return model.write(&out);
}