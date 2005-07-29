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
#if 0
#include "DSFPrint.h"
#include "DSFLib.h"
#include <stdio.h>

static	void	PrintStringVector(const vector<string>& v, const char * t, FILE * out)
{
	fprintf(out, "%s\n", t);
	int n = 0;
	for (vector<string>::const_iterator i = v.begin(); i != v.end(); ++i, ++n)
	{
		fprintf(out, "(%d) %s\n", n, i->c_str());
	}
}

void	PrintDSF(const char * inFileName, const char * outName)
{
	FILE * out = fopen(outName, "w");
	if (!out) return;
	
	DSFFileRef	dsf = DSFOpenFile(inFileName);
	if (dsf == NULL) 
	{
		fprintf(out, "Could not open %s\n", inFileName);
		fclose(out);
		return;
	}
	
	int	count3d = DSFCount3DPoints(dsf);
	int	count2d = DSFCount2DPoints(dsf);
	int countst = DSFCountSTPoints(dsf);
	
	fprintf(out, "File contains %d 3d pts, %d 2d pts, %d tex coords.\n", count3d, count2d, countst);

	vector<string>	objDefs, terDefs, protoDefs, netDefs;
	
	DSFGetObjectDefs	(dsf, objDefs);
	DSFGetTerrainDefs	(dsf, terDefs);
	DSFGetPrototypeDefs	(dsf, protoDefs);
	DSFGetNetworkDefs	(dsf, netDefs);
	
	PrintStringVector(objDefs, "Object definitions", out);
	PrintStringVector(terDefs, "Terrain definitions", out);
	PrintStringVector(protoDefs, "Prototype definitions", out);
	PrintStringVector(netDefs, "Network definitions", out);

	DSFCommandIterator	iter;
	iter.Init(dsf);
	
	unsigned int	index, pt, startI, endI, chainT, startC, endC, floors;
	float			heading, /*LODmin, */LODmax, v[3];
	vector<unsigned int>	shape, shapeCurve;
	vector<DSFTexturedGeoElement_t>		geoElemTex;
	vector<DSFProjectedGeoElement_t>	geoElemProj;
	vector<DSFMaskedGeoElement_t>		geoElemMask;
	int		zbuf, hard;
	
	int	count_obj = 0, count_obj_rot = 0, count_net = 0, count_chain = 0, count_proto = 0, 
		count_lod = 0, count_tex = 0, count_proj = 0, count_mask = 0;
	int tex_poly_count = 0, proj_poly_count = 0, mask_poly_count = 0;
	bool ok = true;
	
	while (!iter.Done() && ok) {
		switch(iter.GetType()) {
		case dsf_Cmd_Object:
			++count_obj;
			iter.ReadObject(&index, &pt, &heading);
			fprintf(out, "  Custom object %s at pt %d, heading %f\n", objDefs[index].c_str(), pt, heading);
			break;
		case dsf_Cmd_RotatedObject:
			++count_obj_rot;
			iter.ReadRotatedObject(&index, &pt, v, &heading);
			fprintf(out, "  Custom object %s at pt %d, heading [%f/%f/%f] %f\n", objDefs[index].c_str(), pt, v[0], v[1], v[2], heading);
			break;
		case dsf_Cmd_NetworkChain:
			++count_chain;
			iter.ReadNetworkChain(&index, &startI, &endI, &chainT, 
				&startC, &endC, &shape, &shapeCurve);
			fprintf(out, "  Network segment (%s/%d) from %d/%d to %d/%d, %d shape points.\n",
				netDefs[index].c_str(),chainT, startI, startC, endI, endC, shape.size());
			break;
		case dsf_Cmd_NetworkJunctionTable:
			++count_net;
			iter.ReadNetworkJunctionTable(&index, &shape);
			fprintf(out, "   Network type %s has %d nodes.\n", netDefs[index].c_str(), shape.size());
			break;
		case dsf_Cmd_Prototype:
			++count_proto;
			iter.ReadPrototype(&index, &floors, &shape);
			fprintf(out, "  Prototype %s, %d floors, %d pts\n", protoDefs[index].c_str(), floors, shape.size());
			break;			
		case dsf_Cmd_TerrainLOD:
			++count_lod;
			iter.ReadTerrainLOD(&index, /*&LODmin, */&LODmax);
			fprintf(out, "  Terrain patch LOD %f\n", /*LODmin, */LODmax);
			break;
		case dsf_Cmd_GeometryTextured:
			++count_tex;
			iter.ReadGeoTextured(&index, &zbuf, &hard, &geoElemTex);
			fprintf(out, "  Textured geometry %s, zbuf=%s,hard=%s, %d commands.\n",
				terDefs[index].c_str(), zbuf ? "overlay" : "write", hard ? "overlay" : "solid", geoElemTex.size());
			tex_poly_count += geoElemTex.size();
			break;
		case dsf_Cmd_GeometryProjected:
			++count_proj;
			iter.ReadGeoProjected(&index, &zbuf, &hard, &geoElemProj);
			fprintf(out, "  Projected geometry %s, zbuf=%s,hard=%s, %d commands.\n",
				terDefs[index].c_str(), zbuf ? "overlay" : "write", hard ? "overlay" : "solid", geoElemProj.size());
			proj_poly_count += geoElemProj.size();
			break;
		case dsf_Cmd_GeometryMasked:
			++count_mask;
			iter.ReadGeoMasked(&index, &zbuf, &hard, &geoElemMask);
			fprintf(out, "  Masked geometry %s, zbuf=%s,hard=%s, %d commands.\n",
				terDefs[index].c_str(), zbuf ? "overlay" : "write", hard ? "overlay" : "solid", geoElemMask.size());
			mask_poly_count += geoElemMask.size();
			break;
		default:
			fprintf(out, "Unknown command!\n");
			ok = false;
			break;
		}
	}

	fprintf(out, "Objects: %d\nRotated Objects: %d\nNetworks: %d\nNetwork Chains: %d\n Prototypes: %d\n"
				 "LOD changse: %d\nTextured Geo: %d\nProjected Geo: %d\nMasked Geo: %d\n",
				count_obj, count_obj_rot, count_net, count_chain, count_proto, 
				count_lod, count_tex, count_proj, count_mask);
	fprintf(out, "Textured polys: %d Projected Polys: %d Masked Polys: %d\n", tex_poly_count, proj_poly_count, mask_poly_count);

	DSFCloseFile(dsf);
	fclose(out);
}

#endif