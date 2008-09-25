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
#include <list>
#include "XUtils.h"
#include <time.h>
#include "Persistence.h"
#include "EnvParser.h"
#include "XGrinderApp.h"
#include "EnvWrite.h"

/*

TODO: figure out lat lon order!!

CMDS:

ALIAS obj obj1 chance obj2 chance obj3 chance
OBJECT	lon lon lat lat heading heading chance name
GROUP chance
SWITCH
END

In a group or the top level file, all chances are independent.
But in a switch chances are mutually exclusive (and should add up to 100!!)

Compatibility

LIT lon lat displacement chance elev r g b
TRE lon lat dispalcement chance type
OBJ lon lat displacement chance heading name

FOR lon lat displacement chance type

FOR  0.50 0.50 0.40 1.00      3


NOTE: for trees you must have an alias to an object named: DEF_TRE_<type>
for each tree used.
NOTE: for lights you must have an alias to an object named: DEF_LIT_<e>_<r>_<g>_<b>
for each light used.
NOTE: aliases do work for objects.

*/

typedef	pair<double, double>	DoubleRange;
typedef	void ( * InsertObj_f)(double lat, double lon, double heading, const string& name, void * ref);

typedef	pair<string, double>		AliasChance;
typedef vector<AliasChance>			AliasList;
typedef	map<string, AliasList>	AliasMap;

// These 8 doubles define the extent of our current quad in global world terms
double	gLat_SW;	double	gLat_NW;	double	gLat_SE;	double	gLat_NE;
double	gLon_SW;	double	gLon_NW;	double	gLon_SE;	double	gLon_NE;

// These 4 doubles define the extent of our current quad in local texture terms
double	gY_S, gY_N, gX_W, gX_E;

// The rotation of our current quad
double	gRotation;

inline	double interp(double i1, double i2, double i, double o1, double o2)
{
	return o1 + (i - i1) * (o2 - o1) / (i2 - i1);
}

static	string	LookupObjectAlias(const string& inName);

class	SceneryNode;

typedef	map<string, SceneryNode *>	SceneryMap;

SceneryMap					gSceneryMap;
AliasMap					gAliases;

class	SceneryNode {
public:

	virtual 		~SceneryNode();
	virtual	void	AddObjects(InsertObj_f addFunc, void * ref)=0;
	virtual	double	GetChance(void)=0;
	virtual	bool	AcceptNode(SceneryNode * inNode)=0;

};

typedef vector<SceneryNode*>	NodeVector;

class	ObjectNode : public SceneryNode {
public:

					ObjectNode(DoubleRange lat, DoubleRange lon, DoubleRange heading, double chance, const string& name);
	virtual 		~ObjectNode();
	virtual	void	AddObjects(InsertObj_f addFunc, void * ref);
	virtual	double	GetChance(void);
	virtual	bool	AcceptNode(SceneryNode * inNode);

private:

		DoubleRange	mLat;
		DoubleRange	mLon;
		DoubleRange	mHeading;
		double		mChance;
		string		mObject;

};

class	GroupNode : public SceneryNode {
public:

					GroupNode(double inChance);
	virtual 		~GroupNode();
	virtual	void	AddObjects(InsertObj_f addFunc, void * ref);
	virtual	double	GetChance(void);
	virtual	bool	AcceptNode(SceneryNode * inNode);

private:

		NodeVector	mItems;
		double		mChance;
};

class	SwitchNode : public SceneryNode {
public:

					SwitchNode(double inChance);
	virtual 		~SwitchNode();
	virtual	void	AddObjects(InsertObj_f addFunc, void * ref);
	virtual	double	GetChance(void);
	virtual	bool	AcceptNode(SceneryNode * inNode);

private:

		double		mChance;
		NodeVector	mItems;

};

SceneryNode *	ParseFile(const char * inFile)
{
	char	buf[512];

	StTextFileScanner	scanner(inFile, true);
	if (scanner.done())
	{
		XGrinder_ShowMessage("Unable to open the file %s", inFile);
		return NULL;
	}

	string 			line;
	vector<string>	args;

	GroupNode * masterGroup = new GroupNode(1.0);

	list<SceneryNode *>	stack;
	stack.push_back(masterGroup);

	while (GetNextNoComments(scanner, line))
	{
		BreakString(line, args);
		if (args.size() > 0)
		{
			if (args[0] == "ALIAS")
			{
				// ALIAS obj obj1 chance1 obj2 chance2...
				if (args.size() < 2)
				{
					XGrinder_ShowMessage("Unable to parse the line %s", line.c_str());
					return NULL;
				}
				AliasList	aliases;
				for (int n = 3; n < args.size(); n += 2)
				{
					aliases.push_back(AliasChance(args[n-1], atof(args[n].c_str())));
				}
				gAliases.insert(AliasMap::value_type(args[1], aliases));
			}
			else if (args[0] == "OBJECT")
			{
				// OBJECT lon1 lon2 lat1 lat2 heading1 heading2 chance object_name
				if (args.size() < 9)
				{
					XGrinder_ShowMessage("Unable to parse the line %s", line.c_str());
					return NULL;
				}
				ObjectNode * o = new ObjectNode(
					DoubleRange(atof(args[3].c_str()),atof(args[4].c_str())),	// lat
					DoubleRange(atof(args[1].c_str()),atof(args[2].c_str())),	// lon
					DoubleRange(atof(args[5].c_str()),atof(args[6].c_str())),	// heading
					atof(args[7].c_str()),
					args[8]);
				if (!stack.back()->AcceptNode(o))
				{
					XGrinder_ShowMessage("Unable to parse the line %s", line.c_str());
					return NULL;
				}
			}
			else if (args[0] == "GROUP")
			{
				// GROUP chance
				if (args.size() < 2)
				{
					XGrinder_ShowMessage("Unable to parse the line %s", line.c_str());
					return NULL;
				}
				GroupNode * grp = new GroupNode(atof(args[1].c_str()));
				if (!stack.back()->AcceptNode(grp))
				{
					XGrinder_ShowMessage("Unable to parse the line %s", line.c_str());
					return NULL;
				}
			}
			else if (args[0] == "SWITCH")
			{
				// SWITCH
				if (args.size() < 2)
				{
					XGrinder_ShowMessage("Unable to parse the line %s", line.c_str());
					return NULL;
				}
				SwitchNode * swt = new SwitchNode(atof(args[1].c_str()));
				if (!stack.back()->AcceptNode(swt))
				{
					XGrinder_ShowMessage("Unable to parse the line %s", line.c_str());
					return NULL;
				}
			}
			else if (args[0] == "END")
			{
				 // END
				 if (stack.size() < 2)
				 {
					XGrinder_ShowMessage("Unable to parse the line %s", line.c_str());
				 	return NULL;
				 }
				 stack.pop_back();
			}
			else if (args[0] == "LIT")
			{
				//LIT lon lat displacement chance elev r g b
				if (args.size() < 9)
				{
					XGrinder_ShowMessage("Unable to parse the line %s", line.c_str());
					return NULL;
				}
				sprintf(buf,"DEF_LIT_%d_%d_%d_%d",
					atoi(args[5].c_str()), atoi(args[6].c_str()), atoi(args[7].c_str()), atoi(args[8].c_str()));

				double	lon = atof(args[1].c_str());
				double 	lat = atof(args[2].c_str());
				double	var = atof(args[3].c_str());
				double	chance = atof(args[4].c_str());

				ObjectNode * o = new ObjectNode(
					DoubleRange(lat-var, lat+var),
					DoubleRange(lon-var, lon+var),
					DoubleRange(0, 0),
					chance,
					buf);
				if (!stack.back()->AcceptNode(o))
				{
					XGrinder_ShowMessage("Unable to parse the line %s", line.c_str());
					return NULL;
				}
			}
			else if (args[0] == "TRE" || args[0] == "FOR")
			{
				//TRE lon lat displacement chance type
				if (args.size() < 6)
				{
					XGrinder_ShowMessage("Unable to parse the line %s", line.c_str());
					return NULL;
				}
				sprintf(buf,"DEF_%s_%d", args[0].c_str(),
					atoi(args[5].c_str()));

				double	lon = atof(args[1].c_str());
				double 	lat = atof(args[2].c_str());
				double	var = atof(args[3].c_str());
				double	chance = atof(args[4].c_str());

				ObjectNode * o = new ObjectNode(
					DoubleRange(lat-var, lat+var),
					DoubleRange(lon-var, lon+var),
					DoubleRange(0.0, 359.0),
					chance,
					buf);
				if (!stack.back()->AcceptNode(o))
				{
					XGrinder_ShowMessage("Unable to parse the line %s", line.c_str());
					return NULL;
				}
			}
			else if (args[0] == "OBJ")
			{
				//OBJ lon lat displacement chance heading name
				if (args.size() < 7)
				{
					XGrinder_ShowMessage("Unable to parse the line %s", line.c_str());
					return NULL;
				}
				double	lon = atof(args[1].c_str());
				double 	lat = atof(args[2].c_str());
				double	var = atof(args[3].c_str());
				double	chance = atof(args[4].c_str());
				double	head = atof(args[5].c_str());

				ObjectNode * o = new ObjectNode(
					DoubleRange(lat-var, lat+var),
					DoubleRange(lon-var, lon+var),
					DoubleRange(head, head),
					chance,
					args[6]);
				if (!stack.back()->AcceptNode(o))
				{
					XGrinder_ShowMessage("Unable to parse the line %s", line.c_str());
					return NULL;
				}
			}
			else if (args[0] == "COM")
			{
			} else
			{
				XGrinder_ShowMessage("Unable to parse the line %s", line.c_str());
				return NULL;
			}
		}
	}

	return masterGroup;
}

string	LookupObjectAlias(const string& inName)
{
	AliasMap::iterator i = gAliases.find(inName);
	if (i == gAliases.end())
		return inName;

	vector<double>	probs;
	for (AliasList::iterator av = i->second.begin();
		av != i->second.end(); ++av)
		probs.push_back(av->second);

	int index = PickRandom(probs);

	if (index < i->second.size())
		return i->second[index].first;
	return string();
}

void	AccumObject(double lat, double lon, double heading, const string& name, void * ref)
{
		double			plat, plon;
		ObjectInfo		oi;

	switch((int) gRotation) {
	case 90:
		plat = 1.0 - lon;
		plon = lat;
		break;
	case 180:
		plat = 1.0 - lat;
		plon = 1.0 - lon;
		break;
	case 270:
		plat = lon;
		plon = 1.0 - lat;
		break;
	case 0:
	case 360:
	default:
		plat = lat;
		plon = lon;
		break;
	}

	oi.latitude = interp(
						gY_S,
						gY_N,
						plat,
						interp(gX_W, gX_E, plon, gLat_SW, gLat_SE),
						interp(gX_W, gX_E, plon, gLat_NW, gLat_NE));
	oi.longitude = interp(
						gX_W,
						gX_E,
						plon,
						interp(gY_S, gY_N, plat, gLon_SW, gLon_NW),
						interp(gY_S, gY_N, plat, gLon_SE, gLon_NE));
	oi.kind = 8;
	oi.elevation = ((int) (heading + gRotation)) % 360;
	oi.name = name;

	if ((plon >= gX_W) && (plon <= gX_E) && (plat >= gY_S) && (plat <= gY_N))
		gObjects.push_back(oi);
}

void	ApplyObjects(void)
{
	for (int y = 0; y < 200; ++y)
	for (int x = 0; x < 150; ++x)
	{
		int	sw = y * 151 + x;
		int	se = sw + 1;
		int nw = sw + 151;
		int ne = nw + 1;

		if (gVertices[sw].custom)
		{
			int ter_index = gVertices[sw].landUse;
			if (ter_index < gTextures.size())
			{
				string	tex_name = gTextures[ter_index];
				StripPath(tex_name);
				StringToUpper(tex_name);
				if (gSceneryMap.find(tex_name) != gSceneryMap.end())
				{
					SceneryNode * p = gSceneryMap[tex_name];
					if (p)
					{
						VertexInfo&	vsw = gVertices[sw];
						VertexInfo&	vnw = gVertices[nw];
						VertexInfo&	vse = gVertices[se];
						VertexInfo&	vne = gVertices[ne];

						gX_W = ((float) (vsw.xOff)) / ((float) (vsw.scale + 1));
						gX_E = ((float) (vsw.xOff + 1)) / ((float) (vsw.scale + 1));
						gY_S = ((float) (vsw.yOff)) / ((float) (vsw.scale + 1));
						gY_N = ((float) (vsw.yOff + 1)) / ((float) (vsw.scale + 1));

						gRotation = vsw.rotation;
						gLat_SW = vsw.latitude;
						gLat_SE = vse.latitude;
						gLat_NW = vnw.latitude;
						gLat_NE = vne.latitude;
						gLon_SW = vsw.longitude;
						gLon_SE = vse.longitude;
						gLon_NW = vnw.longitude;
						gLon_NE = vne.longitude;

						p->AddObjects(AccumObject, NULL);
					}
				}
			}
		}
	}
}

void	XGrindInit(string& outString)
{
	outString = "AddObjects";
	XGrinder_ShowMessage("Drag an .env file and .opf file to apply textures.");
}

int	XGrinderMenuPick(xmenu menu, int item)
{
	return 0;
}

void	XGrindFiles(const vector<string>& files)
{
	gSceneryMap.clear();
	srand(clock());

	int opfCount = 0;

	for (vector<string>::const_iterator f = files.begin(); f != files.end(); ++f)
	{
		if (HasExtNoCase(*f, ".opf"))
		{
			SceneryNode * p = ParseFile(f->c_str());
			if (p)
			{
				string	t = f->substr(0,f->length()-4);
				StripPath(t);
				StringToUpper(t);
				gSceneryMap.insert(SceneryMap::value_type(t, p));
				++opfCount;
			} else
				return;
		}
	}

	if (opfCount == 0)
	{
		XGrinder_ShowMessage("Pleaes drag at least one .opf file as well as one or more .env files to add objects.");
		return;
	}

	int total = 0;
	int	envCount = 0;
	for (vector<string>::const_iterator f = files.begin(); f != files.end(); ++f)
	{
		if (HasExtNoCase(*f, ".env"))
		{
			ClearEnvData();
			if (ReadEnvFile(f->c_str()) == 0)
			{
				long	firstCount = gObjects.size();
				ApplyObjects();
				total += (gObjects.size() - firstCount);
				firstCount = gObjects.size() - firstCount;
				string	nn = f->substr(0, f->length() - 4) + "_new.env";
				if (EnvWrite(nn.c_str()))
				{
					XGrinder_ShowMessage("Could not write %s.", nn.c_str());
					return;
				} else {
					++envCount;
					XGrinder_ShowMessage("Added %d objects to %s.", firstCount, nn.c_str());
				}
			} else {
				XGrinder_ShowMessage("Could not open file %s.", f->c_str());
				return;
			}
		}
	}
	if (envCount == 0)
		XGrinder_ShowMessage("Please also drag an .env file at the same time to add objects.");
	if (envCount > 1)
		XGrinder_ShowMessage("Added %d objects to %d files.\n", total, envCount);
}

#pragma mark -

SceneryNode::~SceneryNode()
{
}

ObjectNode::ObjectNode(
	DoubleRange lat,
	DoubleRange lon,
	DoubleRange heading,
	double chance,
	const string& name) :
	mLat(lat), mLon(lon),mHeading(heading),mChance(chance), mObject(name)
{
}

ObjectNode::~ObjectNode()
{
}

void	ObjectNode::AddObjects(InsertObj_f addFunc, void * ref)
{
	string o = LookupObjectAlias(mObject);
	if (!o.empty())
		addFunc(
			RandRange(mLat.first,mLat.second),
			RandRange(mLon.first,mLon.second),
			RandRange(mHeading.first,mHeading.second),
			o,
			ref);
}

double	ObjectNode::GetChance(void)
{
	return mChance;
}

bool	ObjectNode::AcceptNode(SceneryNode * inNode)
{
	return false;
}

GroupNode::GroupNode(double inChance) :
	mChance(inChance)
{
}

GroupNode::~GroupNode()
{
	for (NodeVector::iterator i = mItems.begin();
		i != mItems.end(); ++i)
	{
		delete (*i);
	}
}

void	GroupNode::AddObjects(InsertObj_f addFunc, void * ref)
{
	for (NodeVector::iterator i = mItems.begin();
		i != mItems.end(); ++i)
	{
		SceneryNode * n = *i;
		double	chance = n->GetChance();
		if (RollDice(chance))
			n->AddObjects(addFunc, ref);
	}
}

double	GroupNode::GetChance(void)
{
	return mChance;
}

bool	GroupNode::AcceptNode(SceneryNode * inNode)
{
	mItems.push_back(inNode);
	return true;
}

SwitchNode::SwitchNode(double inChance) : mChance(inChance)
{
}

SwitchNode::~SwitchNode()
{
	for (NodeVector::iterator i = mItems.begin();
		i != mItems.end(); ++i)
	{
		delete (*i);
	}
}

void	SwitchNode::AddObjects(InsertObj_f addFunc, void * ref)
{
	vector<double>	probs;

	for (NodeVector::iterator i = mItems.begin();
		i != mItems.end(); ++i)
	{
		SceneryNode * n = *i;
		double	chance = n->GetChance();
		probs.push_back(chance);
	}

	int index = PickRandom(probs);
	if (index < mItems.size())
	{
		mItems[index]->AddObjects(addFunc, ref);
	}
}

double	SwitchNode::GetChance(void)
{
	return mChance;
}

bool	SwitchNode::AcceptNode(SceneryNode * inNode)
{
	mItems.push_back(inNode);
	return true;
}
