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
#include "XCarBoneUtils.h"
#include "MatrixUtils.h"

bool		XCarBones::IsValid(void * w)
{
	return index.find(w) != index.end();
}

void *	XCarBones::GetParent(void * child)
{
	if (!IsValid(child)) return NULL;
	int me = index[child];
	int parent = bones[me].parent;
	if (parent == -1) return NULL;
	return bones[parent].ref;
}
	
void		XCarBones::GetChildren(void * p, vector<void *>& c)
{
	c.clear();
	if (!IsValid(p)) return;
	int me = index[p];
	for (vector<int>::iterator iter = bones[me].child_cache.begin();
		iter != bones[me].child_cache.end(); ++iter)
	{
		c.push_back(bones[*iter].ref);
	}
}

string		XCarBones::GetBoneName(void * v)
{
	if (!IsValid(v)) return string();
	return bones[index[v]].name;
}
	
void		XCarBones::GetDeboneMatrix(void * who, double m[16])
{
	if (!IsValid(who)) return;
	int me = index[who];
	void * parent = GetParent(who);	
	double parent_matrix[16];
	setIdentityMatrix(m);

	double	now = bones[me].preview_time;
	double	x = GetValueForTime(now, bones[me].xyz[0]);
	double	y = GetValueForTime(now, bones[me].xyz[1]);
	double	z = GetValueForTime(now, bones[me].xyz[2]);
	double	phi = GetValueForTime(now, bones[me].rot[0]);
	double	the = GetValueForTime(now, bones[me].rot[1]);
	double	psi = GetValueForTime(now, bones[me].rot[2]);

	applyRotation(m, psi, 0, 0,-1);	
	applyRotation(m, the, 1, 0, 0);
	applyRotation(m, phi, 0,-1, 0);
	applyTranslation(m, -x, -y, -z);

	if (parent)
	{
		double	temp[16];
		GetDeboneMatrix(parent, parent_matrix);
		multMatrices(temp, m, parent_matrix);
		copyMatrix(m, temp);
	} 	
}

void		XCarBones::GetReboneMatrix(void * who, double m[16])
{
	if (!IsValid(who)) return;
	int me = index[who];
	void * parent = GetParent(who);	
	if (parent)
	{
		GetReboneMatrix(parent, m);
	} else
		setIdentityMatrix(m);
	
	double	now = bones[me].preview_time;
	double	x = GetValueForTime(now, bones[me].xyz[0]);
	double	y = GetValueForTime(now, bones[me].xyz[1]);
	double	z = GetValueForTime(now, bones[me].xyz[2]);
	double	phi = GetValueForTime(now, bones[me].rot[0]);
	double	the = GetValueForTime(now, bones[me].rot[1]);
	double	psi = GetValueForTime(now, bones[me].rot[2]);

	applyTranslation(m, x, y, z);
	applyRotation(m, phi, 0, 1, 0);
	applyRotation(m, the,-1, 0, 0);
	applyRotation(m, psi, 0, 0, 1);	
}

void		XCarBones::RebuildChildCache(void)
{
	vector<XCarBone>::iterator iter;
	for (iter = bones.begin(); iter != bones.end(); ++iter)
	{
		iter->child_cache.clear();
	}
	
	int me;
	for (me = 0; me < bones.size(); ++me)
	{
		int my_parent = bones[me].parent;
		if (my_parent != -1)
			bones[my_parent].child_cache.push_back(me);
	}	
}

void		XCarBones::RebuildIndex(void)
{
	index.clear();
	for (int n = 0; n < bones.size(); ++n)
	{
		index.insert(map<void *, int>::value_type(bones[n].ref, n));
	}
}


double	GetValueForTime(double inTime, const XCarBone::KeyTable& inTable)
{
	// If our table is empty, we cannot interpolate!
	if (inTable.empty()) return 0;

	// Find the earliest object that is equivalent or higher than us
	XCarBone::KeyTable::const_iterator lb = inTable.lower_bound(inTime);
	
	// If we're off the end, then return the very last value in the data structure.
	if (lb == inTable.end())
	{
		--lb;
		return lb->second;
	}
	
	// If we're at the beginning, return the very first value.
	if (lb == inTable.begin()) return lb->second;
	
	// If we hit the node right on the nose, just return its value.
	if (lb->first == inTime) return lb->second;
	
	// Our hit must be slightly after the time we want.  Find the previous key frame.
	XCarBone::KeyTable::const_iterator prev = lb;
	--prev;
	
	// If the previous key frame is at the same time as our time, that's a serious
	// programming error...not only is our key table bad, but our map is broken.
	// avoid a divide by zero just in case!
	if (lb->first == prev->first) return lb->second;
	
	// Okay we have distinct times.  Find out how far through time we are between
	// these key frames
	double	tr = (inTime - prev->first) / (lb->first - prev->first);
	
	// Interpolate the value!
	return (prev->second + tr * (lb->second - prev->second));
}

bool	ReadBonesFromFile(const char * inFileName, XCarBones& outBones)
{
	outBones.bones.clear();
	outBones.index.clear();
	
	FILE * fi = fopen(inFileName, "r");
	if (!fi) return false;
	int count;
	fscanf(fi, "%d", &count);
	while(count--)
	{
		if (feof(fi)) { fclose(fi); return false; }
		// name parent part anim xyz1 rot11 xyz2 rot2
		XCarBone	bone;
		char		nameBuf[100];
		float		flts[12];
		fscanf(fi,"%s %d %d %d %f %f %f %f %f %f %f %f %f %f %f %f",
			nameBuf, 
			&bone.parent, &bone.part_type, &bone.animation_type,
			&flts[0], &flts[ 1], &flts[ 2],
			&flts[3], &flts[ 4], &flts[ 5],
			&flts[6], &flts[ 7], &flts[ 8],
			&flts[9], &flts[10], &flts[11]);
			
		bone.xyz[0].insert(XCarBone::KeyTable::value_type(0.0, flts[0]));
		bone.xyz[1].insert(XCarBone::KeyTable::value_type(0.0, flts[1]));
		bone.xyz[2].insert(XCarBone::KeyTable::value_type(0.0, flts[2]));
		bone.rot[0].insert(XCarBone::KeyTable::value_type(0.0, flts[3]));
		bone.rot[1].insert(XCarBone::KeyTable::value_type(0.0, flts[4]));
		bone.rot[2].insert(XCarBone::KeyTable::value_type(0.0, flts[5]));
		bone.xyz[0].insert(XCarBone::KeyTable::value_type(1.0, flts[6]));
		bone.xyz[1].insert(XCarBone::KeyTable::value_type(1.0, flts[7]));
		bone.xyz[2].insert(XCarBone::KeyTable::value_type(1.0, flts[8]));
		bone.rot[0].insert(XCarBone::KeyTable::value_type(1.0, flts[9]));
		bone.rot[1].insert(XCarBone::KeyTable::value_type(1.0, flts[10]));
		bone.rot[2].insert(XCarBone::KeyTable::value_type(1.0, flts[11]));

		bone.name = nameBuf;
		outBones.bones.push_back(bone);
	}
	outBones.RebuildChildCache();
	fclose(fi);
	return true;
}

bool	WriteBonesToFile(const char * inFileName, XCarBones& outBones)
{
	FILE * fi = fopen(inFileName, "w");
	if (!fi) return false;
	fprintf(fi, "%d\n", outBones.bones.size());
	for (int n = 0; n < outBones.bones.size(); ++n)
	{
		fprintf(fi,"%s %d %d %d %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf\n",
			outBones.bones[n].name.c_str(),
			outBones.bones[n].parent, outBones.bones[n].part_type, outBones.bones[n].animation_type,
			outBones.bones[n].xyz[0][0.0],
			outBones.bones[n].xyz[1][0.0],
			outBones.bones[n].xyz[2][0.0],
			outBones.bones[n].rot[0][0.0],
			outBones.bones[n].rot[1][0.0],
			outBones.bones[n].rot[2][0.0],
			outBones.bones[n].xyz[0][1.0],
			outBones.bones[n].xyz[1][1.0],
			outBones.bones[n].xyz[2][1.0],
			outBones.bones[n].rot[0][1.0],
			outBones.bones[n].rot[1][1.0],
			outBones.bones[n].rot[2][1.0]);
	}
	fclose(fi);
	return true;
}

