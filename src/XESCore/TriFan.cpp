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
#include "TriFan.h"
#include "DSFLib.h"	// for tri fan defs

TriFanBuilder::TriFanBuilder(CDT * inMesh) : mesh(inMesh)
{
}

TriFanBuilder::~TriFanBuilder()
{
}

void		TriFanBuilder::AddTriToFanPool(CDT::Face_handle inFace)
{
//	if (inFace->info().flag)
//		printf("WARNING: flag is NOT set up right.\n");
	inFace->info().flag = true;
	vertices.insert(inFace->vertex(0));
	vertices.insert(inFace->vertex(1));
	vertices.insert(inFace->vertex(2));
//	faces.insert(inFace);
}

void		TriFanBuilder::CalcFans(void)
{
	for (set<CDT::Vertex_handle>::iterator v = vertices.begin(); v != vertices.end(); ++v)
	{
		CDT::Face_circulator	stop, f;
		stop = f = mesh->incident_faces(*v);
		TriFan_t * fan = NULL;
		bool	circular = true;
		do {
			if (f->info().flag && !mesh->is_infinite(f))
			{
//				if (faces.count(f) == 0)
//					printf("ERROR - WEIRD FACE!\n");
				if (!fan)
				{
					fan = new TriFan_t;
					fan->circular = false;
					fan->center = *v;
				}
				fan->faces.push_back(f);
				index.insert(TriFanTable::value_type(f, fan));
			} else {
				circular = false;
				if (fan)
					fan->self = queue.insert(TriFanQueue::value_type(fan->faces.size(), fan));
				fan = NULL;
			}
			--f;
		} while (stop != f);
		if (fan)
		{
			fan->circular = circular;
			fan->self = queue.insert(TriFanQueue::value_type(fan->faces.size(), fan));
		}		
	}
#if DEV	
	Validate();
#endif	
}

TriFan_t *		TriFanBuilder::GetNextFan(void)
{
	if (queue.empty()) return NULL;
	TriFanQueue::iterator i = queue.end();
	--i;
	if (i->first == 1) return NULL;
#if DEV
	Validate();	
#endif		
	TriFan_t *	best = i->second;
	queue.erase(i);
	for (list<CDT::Face_handle>::iterator f = best->faces.begin(); f != best->faces.end(); ++f)
	{
		pair<TriFanTable::iterator,TriFanTable::iterator> range = index.equal_range(*f);
		for (TriFanTable::iterator k = range.first; k != range.second; ++k)
		{
			if (k->second != best)
				PullFaceFromFan(*f,	k->second);
		}
		index.erase(range.first, range.second);
	}
#if DEV
	Validate();	
#endif	
	return best;
}

void			TriFanBuilder::DoneWithFan(TriFan_t * inFan)
{
	for (list<CDT::Face_handle>::iterator f = inFan->faces.begin(); f != inFan->faces.end(); ++f)
	{
#if DEV
		if (!(*f)->info().flag)
			printf("ERROR - this face was already used once.\n");
#endif			
		(*f)->info().flag = false;
	}
	delete inFan;
}

CDT::Face_handle 	TriFanBuilder::GetNextRemainingTriangle(void)
{
	if (queue.empty()) return NULL;
	TriFan_t * f = queue.begin()->second;
#if DEV
	if (f->faces.size() != 1)
		printf("ASSERT FAILED, REMAINING TRI CALLED ON A TRI FAN!!\n");
#endif	
	queue.erase(queue.begin());
	CDT::Face_handle	ff = f->faces.front();
	delete f;
#if DEV
	if (!ff->info().flag)
		printf("ERROR - this face was already used once.\n");
#endif		
	ff->info().flag = false;
	
	// OOPS!  We have to nuke all remaining triangles from the queue!!
	for (TriFanQueue::iterator n = queue.begin(); n != queue.end(); )
	{
		if (n->second->faces.front() == ff)
		{
			TriFanQueue::iterator kill = n;
			++n;
			queue.erase(kill);
		} else
			++n;
	}	
	return ff;
}

int			TriFanBuilder::GetNextPrimitive(list<CDT::Vertex_handle>& out_handles)
{
	out_handles.clear();
	GetNextTriFan(out_handles);
	if(!out_handles.empty())	return dsf_TriFan;
	
	GetRemainingTriangles(out_handles);
								return dsf_Tri;
}


void		TriFanBuilder::GetNextTriFan(list<CDT::Vertex_handle>& out_handles)
{
	out_handles.clear();
	TriFan_t * fan = GetNextFan();
	if(fan)
	{
		out_handles.push_back(fan->center);
		CDT::Vertex_handle avert = (*fan->faces.begin())->vertex(CDT::cw((*fan->faces.begin())->index(fan->center)));
		out_handles.push_back(avert);
		for (list<CDT::Face_handle>::iterator nf = fan->faces.begin(); nf != fan->faces.end(); ++nf)
		{
			avert = (*nf)->vertex(CDT::ccw((*nf)->index(fan->center)));
			out_handles.push_back(avert);				
		}
		DoneWithFan(fan);
	}
}

void		TriFanBuilder::GetRemainingTriangles(list<CDT::Vertex_handle>& out_handles)
{
	out_handles.clear();
	CDT::Face_handle	f;
	while(1)
	{
		f = GetNextRemainingTriangle();
		if(f == NULL) 
			break;
		for (int v = 2; v >= 0; --v)
		{
			out_handles.push_back(f->vertex(v));
		}
	}
}


void TriFanBuilder::PullFaceFromFan(CDT::Face_handle f, TriFan_t * victim)
{
#if DEV
	if (find(victim->faces.begin(), victim->faces.end(), f) == victim->faces.end())
	{
		printf("ERROR: TRIANGLE NOT PRESENT IN FAN.\n");
		return;
	}
#endif	
	// Victim is CERTAINLY going to change valence, pull him now!!
	queue.erase(victim->self);
	
	// CASE 1 - circular tri fan - rotate it around to be continuous and pop out the one tri
	if (victim->circular)
	{
		while (victim->faces.back() != f)
		{
			victim->faces.push_front(victim->faces.back());
			victim->faces.pop_back();
		}
		victim->faces.pop_back();
	} else 
	// CASE 2 - tri is in front
	if (victim->faces.front() == f)
	{
		victim->faces.pop_front();
	} else
	// CASE 3 - tri is on the back
	if (victim->faces.back() == f) 
	{
		victim->faces.pop_back();
	} else {
		list<CDT::Face_handle>::iterator i = find(victim->faces.begin(), victim->faces.end(), f);
#if !DEV		
		if (i == victim->faces.end()) return;
#endif		
		TriFan_t *	new_fan = new TriFan_t;
		new_fan->circular = false;
		new_fan->center = victim->center;
		new_fan->faces.insert(new_fan->faces.begin(), victim->faces.begin(), i);
		++i;
		victim->faces.erase(victim->faces.begin(), i);
		new_fan->self = queue.insert(TriFanQueue::value_type(new_fan->faces.size(), new_fan));
		
		// This is a huge pain, we have to migrate our index. :-(
		for (list<CDT::Face_handle>::iterator m = new_fan->faces.begin(); m != new_fan->faces.end(); ++m)
		{
			pair<TriFanTable::iterator,TriFanTable::iterator> r = index.equal_range(*m);
			for (TriFanTable::iterator e = r.first; e != r.second; ++e)
			{
				if (e->second == victim) e->second = new_fan;
			}
		}
	}

	// Pop the old fan back in the queue, unless of course it is DEAD
	if (victim->faces.empty())
		delete victim;
	else {
		victim->circular = false;
		victim->self = queue.insert(TriFanQueue::value_type(victim->faces.size(), victim));
	}
}

void				TriFanBuilder::Validate(void)
{
	set<TriFan_t *>			fans;
	set<CDT::Face_handle>	tris;
	for (TriFanQueue::iterator q = queue.begin(); q != queue.end(); ++q)
	{
		if (q->first < 1)
			printf("VALIDATION FAILED: empty tri in queue!\n");
		if (q->first != q->second->faces.size())
			printf("VALIDATOIN FAILED: element not indexed by its valence!\n");
		fans.insert(q->second);
		for (list<CDT::Face_handle>::iterator l = q->second->faces.begin(); l != q->second->faces.end(); ++l)
			tris.insert(*l);
	}
	
	for (TriFanTable::iterator tr = index.begin(); tr != index.end(); ++tr)
	{
		if (find(tr->second->faces.begin(), tr->second->faces.end(), tr->first) == tr->second->faces.end())
			printf("VALIDATION FAILED: tri index table references a fan that is AWOL.\n");
	}
	
	for (set<TriFan_t *>::iterator fan = fans.begin(); fan != fans.end(); ++fan)
	{
		for (list<CDT::Face_handle>::iterator f = (*fan)->faces.begin(); f != (*fan)->faces.end(); ++f)
		{
			bool	found = false;
			pair<TriFanTable::iterator,TriFanTable::iterator> p = index.equal_range(*f);
			for (TriFanTable::iterator pp = p.first; pp != p.second; ++pp)
			{
				if (pp->second == *fan) found = true;
			}
			if (!found)
				printf("VALIDATION FAILED: tri fan's tri is not in the index!\n");
		}
	}	
}
