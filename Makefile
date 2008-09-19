CC=gcc
CPP=g++
LINK=g++
DEFINES=-DLIN=1 -DIBM=0 -DAPL=0 -DLIL=1 -DBIG=0 -DDEV=0 -DUSE_JPEG=1 -DUSE_TIF=1
INCLUDES=\
    -Ilibsrc/mesa-7.1/include \
	-Isrc/Env \
	-Isrc/DSF \
	-Isrc/GUI \
    -Isrc/GUI/mmenu \
	-Isrc/Interfaces \
	-Isrc/Obj \
	-Isrc/ObjEdit \
	-Isrc/OGLE \
	-Isrc/UI \
	-Isrc/WEDCore \
	-Isrc/WEDDocs \
	-Isrc/WEDEntities \
	-Isrc/WEDImportExport \
	-Isrc/WEDLayers \
	-Isrc/WEDMap \
	-Isrc/WEDProperties \
	-Isrc/WEDResources \
	-Isrc/WEDWindows \
	-Isrc/WorldEditor \
	-Isrc/XPCompat \
	-Isrc/XPWidgets \
	-Isrc/Utils \
	-Isrc/XESCore \
	-Isrc/XESTools \
	-Ilibsrc/squish-1.10 \
    -Ilibsrc/expat/xmlparse \
    -Ilibsrc/expat/xmltok \
	-I/usr/include/freetype2 \
    -I/usr/include/libshp \
    -I/usr/include/libgeotiff \
	-ISDK/PVR \
    -Isrc/Network \
    -Isrc/Installer

#CFLAGS=$(DEFINES) $(INCLUDES) -O0 -g -Wno-deprecated -include src/Obj/XDefs.h -include limits.h
CFLAGS=$(DEFINES) $(INCLUDES) -O2 -Wno-deprecated -include src/Obj/XDefs.h -include limits.h
CPPFLAGS=$(CFLAGS)

SRC_squish=\
libsrc/squish-1.10/alpha.o \
libsrc/squish-1.10/clusterfit.o \
libsrc/squish-1.10/colourblock.o \
libsrc/squish-1.10/colourfit.o \
libsrc/squish-1.10/colourset.o \
libsrc/squish-1.10/maths.o \
libsrc/squish-1.10/rangefit.o \
libsrc/squish-1.10/singlecolourfit.o \
libsrc/squish-1.10/squish.o


SRC_DSFTool=\
src/DSF/DSFLib.o \
src/DSF/DSFLibWrite.o \
src/DSF/DSFPointPool.o \
src/DSFTools/DSFToolCmdLine.o \
src/DSFTools/DSF2Text.o \
src/DSFTools/ENV2Overlay.o \
src/Env/EnvParser.o \
src/Env/Persistence.o \
src/Utils/AssertUtils.o \
src/Utils/EndianUtils.o \
src/Utils/FileUtils.o \
src/Utils/md5.o \
src/Utils/XChunkyFileUtils.o

SRC_DDSTool=\
src/XPTools/DDSTool.o \
src/Utils/BitmapUtils.o \
src/Utils/PlatformUtils.lin.o

SRC_ObjConverter=\
src/Obj/ObjConvert.o \
src/Obj/ObjPointPool.o \
src/Obj/XObjBuilder.o \
src/Obj/XObjDefs.o \
src/Obj/XObjReadWrite.o \
src/Obj/XObjWriteEmbedded.o \
src/Utils/ObjUtils.o \
src/Utils/AssertUtils.o \
src/Utils/MatrixUtils.o \
src/XPTools/ConvertObj3DS.o \
src/XPTools/ConvertObj.o \
src/XPTools/ConvertObjDXF.o \
SDK/PVR/PVRTTriStrip.o \
SDK/PVR/PVRTGeometry.o

SRC_ObjView=\
src/Obj/ObjPointPool.o \
src/Obj/XObjDefs.o \
src/Obj/XObjReadWrite.o \
src/Obj/ObjDraw.o \
src/ObjEdit/OE_Zoomer3d.o \
src/Utils/ObjUtils.o \
src/Utils/BitmapUtils.o \
src/Utils/TexUtils.o \
src/Utils/trackball.o \
src/Utils/XUtils.o \
src/Utils/GeoUtils.o \
src/Utils/MatrixUtils.o \
src/Utils/PlatformUtils.lin.o \
src/UI/XWin.lin.o \
src/UI/XWinGL.lin.o \
src/UI/xdnd.o \
src/UI/XGUIApp.o \
src/XPTools/ViewObj.o

SRC_WED=\
src/UI/FontMgr.o \
src/UI/XWin.lin.o \
src/UI/XWinGL.lin.o \
src/UI/xdnd.o \
src/WEDCore/WED_AppMain.o \
src/WEDCore/WED_Application.o \
src/WEDCore/WED_PackageMgr.o \
src/WEDCore/WED_Package.o \
src/WEDCore/WED_Archive.o \
src/WEDCore/WED_Buffer.o \
src/WEDCore/WED_Document.o \
src/WEDCore/WED_EnumSystem.o \
src/WEDCore/WED_Errors.o \
src/WEDCore/WED_FastBuffer.o \
src/WEDCore/WED_Persistent.o \
src/WEDCore/WED_Properties.o \
src/WEDCore/WED_PropertyHelper.o \
src/WEDCore/WED_TexMgr.o \
src/WEDCore/WED_UndoLayer.o \
src/WEDCore/WED_UndoMgr.o \
src/WEDEntities/WED_Airport.o \
src/WEDEntities/WED_AirportBeacon.o \
src/WEDEntities/WED_AirportBoundary.o \
src/WEDEntities/WED_AirportChain.o \
src/WEDEntities/WED_AirportNode.o \
src/WEDEntities/WED_AirportSign.o \
src/WEDEntities/WED_ATCFrequency.o \
src/WEDEntities/WED_Entity.o \
src/WEDEntities/WED_GISChain.o \
src/WEDEntities/WED_GISComposite.o \
src/WEDEntities/WED_GISLine.o \
src/WEDEntities/WED_GISLine_Width.o \
src/WEDEntities/WED_GISPoint.o \
src/WEDEntities/WED_GISPoint_Bezier.o \
src/WEDEntities/WED_GISPoint_Heading.o \
src/WEDEntities/WED_GISPoint_HeadingWidthLength.o \
src/WEDEntities/WED_GISPolygon.o \
src/WEDEntities/WED_Group.o \
src/WEDEntities/WED_Helipad.o \
src/WEDEntities/WED_KeyObjects.o \
src/WEDEntities/WED_LightFixture.o \
src/WEDEntities/WED_ObjPlacement.o \
src/WEDEntities/WED_OverlayImage.o \
src/WEDEntities/WED_RampPosition.o \
src/WEDEntities/WED_Ring.o \
src/WEDEntities/WED_Root.o \
src/WEDEntities/WED_Runway.o \
src/WEDEntities/WED_RunwayNode.o \
src/WEDEntities/WED_Sealane.o \
src/WEDEntities/WED_Select.o \
src/WEDEntities/WED_Taxiway.o \
src/WEDEntities/WED_TextureNode.o \
src/WEDEntities/WED_Thing.o \
src/WEDEntities/WED_TowerViewpoint.o \
src/WEDEntities/WED_Windsock.o \
src/WEDImportExport/WED_AptIE.o \
src/WEDImportExport/WED_DSFExport.o \
src/WEDMap/WED_Colors.o \
src/WEDMap/WED_CreateLineTool.o \
src/WEDMap/WED_CreatePointTool.o \
src/WEDMap/WED_CreatePolygonTool.o \
src/WEDMap/WED_CreateToolBase.o \
src/WEDMap/WED_HandleToolBase.o \
src/WEDMap/WED_Map.o \
src/WEDMap/WED_MapBkgnd.o \
src/WEDMap/WED_MapLayer.o \
src/WEDMap/WED_MapPane.o \
src/WEDMap/WED_MapToolNew.o \
src/WEDMap/WED_MapZoomerNew.o \
src/WEDMap/WED_MarqueeTool.o \
src/WEDMap/WED_StructureLayer.o \
src/WEDMap/WED_TerraserverLayer.o \
src/WEDMap/WED_ToolInfoAdapter.o \
src/WEDMap/WED_ToolUtils.o \
src/WEDMap/WED_UIMeasurements.o \
src/WEDMap/WED_VertexTool.o \
src/WEDMap/WED_WorldMapLayer.o \
src/WEDProperties/WED_PropertyPane.o \
src/WEDProperties/WED_PropertyTable.o \
src/WEDWindows/WED_AboutBox.o \
src/WEDWindows/WED_DocumentWindow.o \
src/WEDWindows/WED_GroupCommands.o \
src/WEDWindows/WED_Menus.o \
src/WEDWindows/WED_PackageListAdapter.o \
src/WEDWindows/WED_PackageStatusPane.o \
src/WEDWindows/WED_PackageWindow.o \
src/WEDWindows/WED_StartWindow.o \
src/GUI/GUI_Broadcaster.o \
src/GUI/GUI_Button.o \
src/GUI/GUI_ChangeView.o \
src/GUI/GUI_Commander.o \
src/GUI/GUI_Control.o \
src/GUI/GUI_Destroyable.o \
src/GUI/GUI_DrawUtils.o \
src/GUI/GUI_Fonts.o \
src/GUI/GUI_GraphState.o \
src/GUI/GUI_Listener.o \
src/GUI/GUI_MemoryHog.o \
src/GUI/GUI_Packer.o \
src/GUI/GUI_PopupButton.o \
src/GUI/GUI_Prefs.o \
src/GUI/GUI_Resources.o \
src/GUI/GUI_ScrollBar.o \
src/GUI/GUI_ScrollerPane.o \
src/GUI/GUI_SimpleScroller.o \
src/GUI/GUI_SimpleTableGeometry.o \
src/GUI/GUI_Splitter.o \
src/GUI/GUI_TabControl.o \
src/GUI/GUI_Table.o \
src/GUI/GUI_TabPane.o \
src/GUI/GUI_TextField.o \
src/GUI/GUI_TextTable.o \
src/GUI/GUI_Timer.o \
src/GUI/GUI_ToolBar.o \
src/GUI/GUI_Window.o \
src/GUI/GUI_Pane.o \
src/GUI/GUI_Help.o \
src/GUI/GUI_Clipboard.o \
src/GUI/GUI_Application.o \
src/GUI/GUI_Laftfont.o \
src/GUI/mmenu/mmenu.o \
src/Utils/SQLUtils.o \
src/Utils/AssertUtils.o \
src/Utils/MemFileUtils.o \
src/Utils/FileUtils.o \
src/Utils/GISUtils.o \
src/Utils/BitmapUtils.o \
src/Utils/TexUtils.o \
src/Utils/PlatformUtils.lin.o \
src/Utils/EndianUtils.o \
src/Utils/md5.o \
src/Utils/XChunkyFileUtils.o \
src/Utils/CompGeomUtils.o \
src/Utils/PolyRasterUtils.o \
src/Utils/zip.o \
src/Utils/unzip.o \
src/Utils/XUtils.o \
src/Utils/BWImage.o \
src/Utils/ObjUtils.o \
src/Utils/XFileTwiddle.o \
src/Utils/XFileTwiddle.unix.o \
src/Utils/Skeleton.o \
src/Utils/perlin.o \
src/Utils/MatrixUtils.o \
src/Installer/ErrMsg.o \
src/Obj/ObjPointPool.o \
src/Obj/XObjDefs.o \
src/Obj/XObjReadWrite.o \
src/Obj/ObjDraw.o \
src/Network/Terraserver.o \
src/Network/HTTPClient.o \
src/Network/PCSBSocket.lin.o \
src/Network/PCSBSocketUDP.lin.o \
src/Network/XMLObject.o \
src/Network/b64.o \
src/DSF/DSFLib.o \
src/DSF/DSFLibWrite.o \
src/DSF/DSFPointPool.o \
src/DSFTools/DSF2Text.o \
src/DSFTools/ENV2Overlay.o \
src/OGLE/ogle.o \
libsrc/expat/xmlparse/xmlparse.o \
libsrc/expat/xmltok/xmlrole.o \
libsrc/expat/xmltok/xmltok.o \
src/Env/EnvParser.o \
src/Env/Persistence.o \
src/WorldEditor/WED_Assert.o \
src/XESCore/XESInit.o \
src/XESCore/DEMTables.o \
src/XESCore/Airports.o \
src/XESCore/AptIO.o \
src/XESCore/Beaches.o \
src/XESCore/ConfigSystem.o \
src/XESCore/DEMAlgs.o \
src/XESCore/DEMDefs.o \
src/XESCore/DEMToVector.o \
src/XESCore/DEMIO.o \
src/XESCore/DSFBuilder.o \
src/XESCore/EnumSystem.o \
src/XESCore/EuroRoads.o \
src/XESCore/Forests.o \
src/XESCore/GreedyMesh.o \
src/XESCore/Hydro.o \
src/XESCore/MapAlgs.o \
src/XESCore/MapDefs.o \
src/XESCore/MapIO.o \
src/XESCore/MeshAlgs.o \
src/XESCore/MeshDefs.o \
src/XESCore/MeshIO.o \
src/XESCore/NetPlacement.o \
src/XESCore/NetTables.o \
src/XESCore/ObjPlacement.o \
src/XESCore/ObjTables.o \
src/XESCore/ParamDefs.o \
src/XESCore/SceneryPackages.o \
src/XESCore/SimpleIO.o \
src/XESCore/TensorRoads.o \
src/XESCore/TriFan.o \
src/XESCore/WTPM.o \
src/XESCore/XESIO.o \
src/XESCore/Zoning.o \
src/XPWidgets/XPLMGraphics.o


SRC_FONTTOOL=\
src/fonttool/fonttool.o


SRC_ObjView+=$(SRC_squish)
SRC_DDSTool+=$(SRC_squish)
SRC_WED+=$(SRC_squish)

SRC_MeshTool=\
src/MeshTool/MeshTool.o

#.SILENT:

all:	objview dsftool ddstool objconverter

%.o: %.cp
	$(CPP) $(CPPFLAGS) -c $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@	

%.o: %.cpp
	$(CPP) $(CPPFLAGS) -c $< -o $@

objview: $(SRC_ObjView)
	$(LINK) -o ObjView -lGL -lGLU -lpng -ljpeg -ltiff $(SRC_ObjView)

dsftool: $(SRC_DSFTool)
	$(LINK) -o DSFTool $(SRC_DSFTool)

ddstool: $(SRC_DDSTool)
	$(LINK) -o DDSTool -lpng -ljpeg -ltiff $(SRC_DDSTool)

objconverter: $(SRC_ObjConverter)
	$(LINK) -o ObjConverter -l3ds -ldime $(SRC_ObjConverter)

meshtool: $(SRC_MeshTool)
	$(LINK) -o MeshTool $(SRC_MeshTool)

wed: $(SRC_WED)
	$(LINK) -o WED -lXi -lGL -lGLU -lfreetype -lz -lpng -lsqlite3 -lCGAL -lgeotiff -lproj -lshp -ljpeg -ltiff $(SRC_WED)

fonttool: $(SRC_FONTTOOL)
	$(LINK) -o fonttool -lfreetype -lz -lpng -s $(SRC_FONTTOOL)

clean:
	rm -f $(SRC_DSFTool) $(SRC_DDSTool) $(SRC_ObjConverter) $(SRC_MeshTool) $(SRC_ObjView) $(SRC_WED) $(SRC_FONTTOOL)

distclean:	clean
	-rm -f ObjView
	-rm -f DDSTool
	-rm -f DSFTool
	-rm -f MeshTool
	-rm -f WED
	-rm -f ObjConverter
	-rm -f fonttool

