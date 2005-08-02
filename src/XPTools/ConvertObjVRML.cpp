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
#include "ConvertObjVRML.h"
#include "ObjUtils.h"

#include <SceneGraph.h>

static	void	ErrCB(int nLine, void *info)
{
//	printf("WARNING: parse in line %d.\n", nLine);
}	


bool	ReadObjVRML(const char * inFilePath, XObj& obj)
{
	obj.cmds.clear();
	bool success = false;
	obj.texture = "no_texture";

	SceneGraph	sg;
	sg.load((char *) inFilePath, false, ErrCB, NULL);
	Node * n;
	for (Node * n = sg.getNodes();
		n; n = n->nextTraversal())
	{
		if (n->isImageTextureNode())
		{
			ImageTextureNode * tex = (ImageTextureNode *) n;
			if (tex->getNUrls() > 0)
			{
				obj.texture = tex->getUrl(0);
				
				string::size_type loc = obj.texture.rfind('.');
				if (loc != string::npos)
					obj.texture = obj.texture.substr(0, loc);
			}				
		}
		if (n->isIndexedFaceSetNode())
		{
			IndexedFaceSetNode * ifs = (IndexedFaceSetNode *) n;

			CoordinateNode * coords = ifs->getCoordinateNodes();
			
			TextureCoordinateNode *	texCoords = ifs->getTextureCoordinateNodes();
			
			if (coords)
			{
				int coord_count = ifs->getNCoordIndexes();
				int tex_count = ifs->getNTexCoordIndexes();
				XObjCmd	polyCmd;
				polyCmd.cmdType = type_Poly;
				
				SFMatrix		matrix;
				coords->getTransformMatrix(&matrix);
				
				for (int i = 0; i < coord_count; ++i)
				{
					int index = ifs->getCoordIndex(i);
					int	tindex = -1;
					if (tex_count == coord_count)
						tindex = ifs->getTexCoordIndex(i);
						
					if (index == -1)
					{
							 if (polyCmd.st.size() == 3) polyCmd.cmdID = obj_Tri;
						else if (polyCmd.st.size() == 4) polyCmd.cmdID = obj_Quad;
						else							 polyCmd.cmdID = obj_Polygon;
		
						if (ifs->getCCW())
							ChangePolyCmdCW(polyCmd);
		
						obj.cmds.push_back(polyCmd);
						polyCmd.st.clear();
					
					} else {
					
						float	pt[3];
						float	st[2];
						coords->getPoint(index, pt);
						matrix.multi(&pt[0], &pt[1], &pt[2]);
						vec_tex	vv;
						
						if (tindex != -1 && texCoords != NULL)
						{
							texCoords->getPoint(tindex, st);
							vv.st[0] = st[0];
							vv.st[1] = st[1];
						} else
							vv.st[0] = vv.st[1] = 0.0;
						vv.v[0] =  pt[0];
						vv.v[1] =  pt[1];
						vv.v[2] =  pt[2];
						
						polyCmd.st.push_back(vv);
					}
				}
				if (!polyCmd.st.empty())
				{
						 if (polyCmd.st.size() == 3) polyCmd.cmdID = obj_Tri;
					else if (polyCmd.st.size() == 4) polyCmd.cmdID = obj_Quad;
					else							 polyCmd.cmdID = obj_Polygon;
					
					if (ifs->getCCW())
						ChangePolyCmdCW(polyCmd);
					obj.cmds.push_back(polyCmd);
				}
			}
		}
	}
	
	if (!obj.cmds.empty())
	 	success = true;
	return success;	
}

bool	WriteObjVRML(const char * inFilePath, const XObj& inObj)
{
	return false;
}

