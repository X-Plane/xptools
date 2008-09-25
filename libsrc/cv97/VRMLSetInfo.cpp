/******************************************************************
*
*	CyberVRML97 for C++
*
*	Copyright (C) Satoshi Konno 1996-2002
*
*	File:	VRMLSetInfo.cpp
*
******************************************************************/

#include "SceneGraph.h"
#include "VRMLSetInfo.h"
#include "VRMLNodeType.h"

void AddSFColor(float color[3])
{
    switch (GetCurrentNodeType()) {
	case VRML_NODETYPE_COLOR:
		{
			((ColorNode *)GetCurrentNodeObject())->addColor(color);
		}
		break;
    case VRML_NODETYPE_BACKGROUND_GROUNDCOLOR:
		{
			BackgroundNode *bg = (BackgroundNode *)GetCurrentNodeObject();
			bg->addGroundColor(color);
		}
	    break;
    case VRML_NODETYPE_BACKGROUND_SKYCOLOR:
		{
			BackgroundNode *bg = (BackgroundNode *)GetCurrentNodeObject();
			bg->addSkyColor(color);
		}
	    break;
	case VRML_NODETYPE_INTERPOLATOR_KEYVALUE:
		switch (GetPrevNodeType()) {
		case VRML_NODETYPE_COLORINTERPOLATOR:
			{
				ColorInterpolatorNode *colorInterp = (ColorInterpolatorNode *)GetCurrentNodeObject();
				colorInterp->addKeyValue(color);
			}
			break;
		}
		break;
    }
}

void AddSFRotation(float rotation[4])
{
    switch (GetCurrentNodeType()) {
	case VRML_NODETYPE_EXTRUSION_ORIENTATION:
		{
			ExtrusionNode *ex = (ExtrusionNode *)GetCurrentNodeObject();
			ex->addOrientation(rotation);
		}
		break;
	case VRML_NODETYPE_INTERPOLATOR_KEYVALUE:
		switch (GetPrevNodeType()) {
		case VRML_NODETYPE_ORIENTATIONINTERPOLATOR:
			{
				OrientationInterpolatorNode *oriInterp = (OrientationInterpolatorNode *)GetCurrentNodeObject();
				oriInterp->addKeyValue(rotation);
			}
			break;
		}
	}
}

void AddSFVec3f(float vector[3])
{
    switch (GetCurrentNodeType()) {
	case VRML_NODETYPE_NORMAL:
		{
			((NormalNode *)GetCurrentNodeObject())->addVector(vector);
		}
	    break;
	case VRML_NODETYPE_COORDINATE:
		{
			((CoordinateNode *)GetCurrentNodeObject())->addPoint(vector);
		}
		break;
	case VRML_NODETYPE_INTERPOLATOR_KEYVALUE:
		switch (GetPrevNodeType()) {
		case VRML_NODETYPE_COORDINATEINTERPOLATOR:
			{
				CoordinateInterpolatorNode *coordInterp = (CoordinateInterpolatorNode *)GetCurrentNodeObject();
				coordInterp->addKeyValue(vector);
			}
			break;
		case VRML_NODETYPE_NORMALINTERPOLATOR:
			{
				NormalInterpolatorNode *normInterp = (NormalInterpolatorNode *)GetCurrentNodeObject();
				normInterp->addKeyValue(vector);
			}
			break;
		case VRML_NODETYPE_POSITIONINTERPOLATOR:
			{
				PositionInterpolatorNode *posInterp = (PositionInterpolatorNode *)GetCurrentNodeObject();
				posInterp->addKeyValue(vector);
			}
			break;
		}
		break;
	case VRML_NODETYPE_EXTRUSION_SPINE:
		{
			ExtrusionNode *ex = (ExtrusionNode *)GetCurrentNodeObject();
			ex->addSpine(vector);
		}
		break;
	}
}

void AddSFVec2f(float vector[2])
{
	switch (GetCurrentNodeType()) {
	case VRML_NODETYPE_TEXTURECOODINATE:
		{
			((TextureCoordinateNode *)GetCurrentNodeObject())->addPoint(vector);
		}
	    break;
	case VRML_NODETYPE_EXTRUSION_CROSSSECTION:
		{
			ExtrusionNode *ex = (ExtrusionNode *)GetCurrentNodeObject();
			ex->addCrossSection(vector);
		}
		break;
	case VRML_NODETYPE_EXTRUSION_SCALE:
		{
			ExtrusionNode *ex = (ExtrusionNode *)GetCurrentNodeObject();
			ex->addScale(vector);
		}
		break;
	}
}

void AddSFInt32(int	value)
{
    switch (GetPrevNodeType()) {
    case VRML_NODETYPE_INDEXEDFACESET:
		{
			IndexedFaceSetNode *idxFaceSet = (IndexedFaceSetNode *)GetCurrentNodeObject();
			switch (GetCurrentNodeType()) {
			case VRML_NODETYPE_COLOR_INDEX:
				idxFaceSet->addColorIndex(value); break;
			case VRML_NODETYPE_COORDINATE_INDEX:
				idxFaceSet->addCoordIndex(value); break;
			case VRML_NODETYPE_NORMAL_INDEX:
				idxFaceSet->addNormalIndex(value); break;
			case VRML_NODETYPE_TEXTURECOODINATE_INDEX:
				idxFaceSet->addTexCoordIndex(value); break;
			}
		}
	    break;
    case VRML_NODETYPE_INDEXEDLINESET:
		{
			IndexedLineSetNode *idxLineSet = (IndexedLineSetNode *)GetCurrentNodeObject();
			switch (GetCurrentNodeType()) {
			case VRML_NODETYPE_COLOR_INDEX:
				idxLineSet->addColorIndex(value); break;
			case VRML_NODETYPE_COORDINATE_INDEX:
				idxLineSet->addCoordIndex(value); break;
			}
		}
		break;
    case VRML_NODETYPE_PIXELTEXTURE:
		{
			PixelTextureNode *pixTexture = (PixelTextureNode *)GetCurrentNodeObject();
			switch (GetCurrentNodeType()) {
			case VRML_NODETYPE_PIXELTEXTURE_IMAGE:
				pixTexture->addImage(value); break;
			}
		}
		break;
    }

}

void AddSFFloat(float value)
{
    switch (GetCurrentNodeType()) {
	case VRML_NODETYPE_ELEVATIONGRID_HEIGHT:
		{
			ElevationGridNode *elev = (ElevationGridNode *)GetCurrentNodeObject();
			elev->addHeight(value);
		}
		break;
    case VRML_NODETYPE_BACKGROUND_GROUNDANGLE:
		{
			BackgroundNode *bg = (BackgroundNode *)GetCurrentNodeObject();
			bg->addGroundAngle(value);
		}
	    break;
    case VRML_NODETYPE_BACKGROUND_SKYANGLE:
		{
			BackgroundNode *bg = (BackgroundNode *)GetCurrentNodeObject();
			bg->addSkyAngle(value);
		}
	    break;
	case VRML_NODETYPE_INTERPOLATOR_KEY:
		switch (GetPrevNodeType()) {
		case VRML_NODETYPE_COLORINTERPOLATOR:
			{
				ColorInterpolatorNode *colorInterp = (ColorInterpolatorNode *)GetCurrentNodeObject();
				colorInterp->addKey(value);
			}
			break;
		case VRML_NODETYPE_COORDINATEINTERPOLATOR:
			{
				CoordinateInterpolatorNode *coordInterp = (CoordinateInterpolatorNode *)GetCurrentNodeObject();
				coordInterp->addKey(value);
			}
			break;
		case VRML_NODETYPE_NORMALINTERPOLATOR:
			{
				NormalInterpolatorNode *normInterp = (NormalInterpolatorNode *)GetCurrentNodeObject();
				normInterp->addKey(value);
			}
			break;
		case VRML_NODETYPE_ORIENTATIONINTERPOLATOR:
			{
				OrientationInterpolatorNode *oriInterp = (OrientationInterpolatorNode *)GetCurrentNodeObject();
				oriInterp->addKey(value);
			}
			break;
		case VRML_NODETYPE_POSITIONINTERPOLATOR:
			{
				PositionInterpolatorNode *posInterp = (PositionInterpolatorNode *)GetCurrentNodeObject();
				posInterp->addKey(value);
			}
			break;
		case VRML_NODETYPE_SCALARINTERPOLATOR:
			{
				ScalarInterpolatorNode *scalarInterp = (ScalarInterpolatorNode *)GetCurrentNodeObject();
				scalarInterp->addKey(value);
			}
			break;
		}
		break;
	case VRML_NODETYPE_INTERPOLATOR_KEYVALUE:
		switch (GetPrevNodeType()) {
		case VRML_NODETYPE_SCALARINTERPOLATOR:
			{
				ScalarInterpolatorNode *scalarInterp = (ScalarInterpolatorNode *)GetCurrentNodeObject();
				scalarInterp->addKeyValue(value);
			}
			break;
		}
		break;
	case VRML_NODETYPE_LOD_RANGE:
		{
			((LodNode *)GetCurrentNodeObject())->addRange(value);
		}
		break;
	case VRML_NODETYPE_NAVIGATIONINFO_AVATARSIZE:
		{
			NavigationInfoNode *navInfo = (NavigationInfoNode *)GetCurrentNodeObject();
			navInfo->addAvatarSize(value);
		}
		break;
	case VRML_NODETYPE_TEXT_LENGTH:
		{
			TextNode *text = (TextNode *)GetCurrentNodeObject();
			text->addLength(value);
		}
		break;
    }
}


void AddSFString(char *string)
{
	switch (GetCurrentNodeType()) {
	case VRML_NODETYPE_ANCHOR_PARAMETER:
		{
			((AnchorNode *)GetCurrentNodeObject())->addParameter(string);
		}
		break;
	case VRML_NODETYPE_ANCHOR_URL:
		{
			((AnchorNode *)GetCurrentNodeObject())->addUrl(string);
		}
		break;
	case VRML_NODETYPE_INLINE_URL:
		{
			((InlineNode *)GetCurrentNodeObject())->addUrl(string);
		}
		break;
	case VRML_NODETYPE_AUDIOCLIP_URL:
		{
			AudioClipNode *aclip = (AudioClipNode *)GetCurrentNodeObject();
			aclip->addUrl(string);
		}
		break;
	case VRML_NODETYPE_BACKGROUND_BACKURL:
		{
			BackgroundNode *bg = (BackgroundNode *)GetCurrentNodeObject();
			bg->addBackUrl(string);
		}
		break;
	case VRML_NODETYPE_BACKGROUND_BOTTOMURL:
		{
			BackgroundNode *bg = (BackgroundNode *)GetCurrentNodeObject();
			bg->addBottomUrl(string);
		}
		break;
	case VRML_NODETYPE_BACKGROUND_FRONTURL:
		{
			BackgroundNode *bg = (BackgroundNode *)GetCurrentNodeObject();
			bg->addFrontUrl(string);
		}
		break;
	case VRML_NODETYPE_BACKGROUND_LEFTURL:
		{
			BackgroundNode *bg = (BackgroundNode *)GetCurrentNodeObject();
			bg->addLeftUrl(string);
		}
		break;
	case VRML_NODETYPE_BACKGROUND_RIGHTURL:
		{
			BackgroundNode *bg = (BackgroundNode *)GetCurrentNodeObject();
			bg->addRightUrl(string);
		}
		break;
	case VRML_NODETYPE_BACKGROUND_TOPURL:
		{
			BackgroundNode *bg = (BackgroundNode *)GetCurrentNodeObject();
			bg->addTopUrl(string);
		}
		break;
	case VRML_NODETYPE_FONTSTYLE_JUSTIFY:
		{
			FontStyleNode *fs = (FontStyleNode *)GetCurrentNodeObject();
			fs->addJustify(string);
		}
		break;
	case VRML_NODETYPE_IMAGETEXTURE_URL:
		{
			ImageTextureNode *image = (ImageTextureNode *)GetCurrentNodeObject();
			image->addUrl(string);
		}
		break;
	case VRML_NODETYPE_MOVIETEXTURE_URL:
		{
			MovieTextureNode *image = (MovieTextureNode *)GetCurrentNodeObject();
			image->addUrl(string);
		}
		break;
	case VRML_NODETYPE_NAVIGATIONINFO_TYPE:
		{
			NavigationInfoNode *navInfo = (NavigationInfoNode *)GetCurrentNodeObject();
			navInfo->addType(string);
		}
		break;
	case VRML_NODETYPE_SCRIPT_URL:
		{
			ScriptNode *script = (ScriptNode *)GetCurrentNodeObject();
			script->addUrl(string);
		}
		break;
	case VRML_NODETYPE_TEXT_STRING:
		{
			TextNode *text = (TextNode *)GetCurrentNodeObject();
			text->addString(string);
		}
		break;
	case VRML_NODETYPE_WORLDINFO_INFO:
		{
			WorldInfoNode *worldInfo = (WorldInfoNode *)GetCurrentNodeObject();
			worldInfo->addInfo(string);
		}
		break;
	}
}
