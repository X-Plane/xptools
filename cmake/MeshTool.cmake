# NOTE: Currently fails to build, at least on apple, due to template errors somewhere deep inside CGAL - not investigating for now, unless we really need this tool.

add_executable(MeshTool
    src/MeshTool/MeshTool.cpp
    src/MeshTool/MeshTool_Create.cpp
    src/Obj/ObjPointPool.cpp
    src/Obj/XObjBuilder.cpp
    src/Obj/XObjDefs.cpp
    src/Obj/XObjReadWrite.cpp
    src/DSF/DSFLib.cpp
    src/DSF/DSFLibWrite.cpp
    src/DSF/DSFPointPool.cpp
    src/XESCore/XESInit.cpp
    src/XESCore/DEMTables.cpp
    src/XESCore/ForestTables.cpp
    src/XESCore/AptAlgs.cpp
    src/XESCore/AptIO.cpp
    src/XESCore/Beaches.cpp
    src/XESCore/BezierApprox.cpp
    src/XESCore/BlockAlgs.cpp
    src/XESCore/BlockFill.cpp
    src/XESCore/ConfigSystem.cpp
    src/XESCore/DEMAlgs.cpp
    src/XESCore/DEMDefs.cpp
    src/XESCore/DEMGrid.cpp
    src/XESCore/DEMToVector.cpp
    src/XESCore/DEMIO.cpp
    src/XESCore/DSFBuilder.cpp
    src/XESCore/EnumSystem.cpp
    src/XESCore/GreedyMesh.cpp
    src/XESCore/MapHelpers.cpp
    src/XESCore/MapAlgs.cpp
    src/XESCore/MapBuffer.cpp
    src/XESCore/MapCreate.cpp
    src/XESCore/MapIO.cpp
    src/XESCore/MapOverlay.cpp
    src/XESCore/MapPolygon.cpp
    src/XESCore/MapTopology.cpp
    src/XESCore/MeshAlgs.cpp
    src/XESCore/MeshDefs.cpp
    src/XESCore/MeshIO.cpp
    src/XESCore/MeshSimplify.cpp
    src/XESCore/NetPlacement.cpp
    src/XESCore/NetHelpers.cpp
    src/XESCore/NetAlgs.cpp
    src/XESCore/NetTables.cpp
    src/XESCore/ObjTables.cpp
    src/XESCore/ParamDefs.cpp
    src/XESCore/SceneryPackages.cpp
    src/XESCore/SimpleIO.cpp
    src/XESCore/TensorRoads.cpp
    src/XESCore/TriFan.cpp
    src/XESCore/XESIO.cpp
    src/XESCore/Zoning.cpp
    src/XESTools/GISTool_Globals.cpp
    src/Utils/AssertUtils.cpp
    src/Utils/MemFileUtils.cpp
    src/Utils/FileUtils.cpp
    src/GUI/GUI_Unicode.cpp
    src/Utils/GISUtils.cpp
    src/Utils/BitmapUtils.cpp
    src/Utils/EndianUtils.c
    src/Utils/md5.c
    src/Utils/XChunkyFileUtils.cpp
    src/Utils/CompGeomUtils.cpp
    src/Utils/PolyRasterUtils.cpp
    src/Utils/zip.c
    src/Utils/unzip.c
    src/Utils/XUtils.cpp
    src/Utils/BWImage.cpp
    src/Utils/ObjUtils.cpp
    src/Utils/Skeleton.cpp
    src/Utils/perlin.cpp
    src/Utils/MatrixUtils.cpp
    src/Utils/ProgressUtils.cpp
    src/RawImport/ShapeIO.cpp
    src/DSF/tri_stripper_101/tri_stripper.cpp

    SDK/libtess2/Source/tess.c
    SDK/libtess2/Source/bucketalloc.c
    SDK/libtess2/Source/dict.c
    SDK/libtess2/Source/mesh.c
    SDK/libtess2/Source/priorityq.c
    SDK/libtess2/Source/sweep.c
    SDK/libtess2/Source/geom.c

    src/lzma19/C/7zArcIn.c
    src/lzma19/C/7zAlloc.c
    src/lzma19/C/7zBuf.c
    src/lzma19/C/7zCrc.c
    src/lzma19/C/7zCrcOpt.c
    src/lzma19/C/7zDec.c
    src/lzma19/C/7zFile.c
    src/lzma19/C/7zStream.c
    src/lzma19/C/Bcj2.c
    src/lzma19/C/Bra.c
    src/lzma19/C/Bra86.c
    src/lzma19/C/BraIA64.c
    src/lzma19/C/CpuArch.c
    src/lzma19/C/Delta.c
    src/lzma19/C/LzmaDec.c
    src/lzma19/C/Lzma2Dec.c
)

target_compile_options(MeshTool PRIVATE -include ${CMAKE_SOURCE_DIR}/src/Obj/XDefs.h)
target_compile_definitions(MeshTool PRIVATE ${BASIC_PLATFORM_DEFINES} -DUSE_JPEG=1 -DUSE_TIF=1)
target_include_directories(MeshTool PRIVATE
	src/lzma19/C
	Src/DSF/tri_stripper_101
	Src/Utils
	Src/XESCore
	Src/XESTools
)

target_link_libraries(MeshTool PRIVATE
    CGAL::CGAL
	geotiff_library
    libsquish::libsquish
    shapelib::shp
    PROJ::proj
    TIFF::TIFF
    JPEG::JPEG
    PNG::PNG
    ZLIB::ZLIB
)

if (APPLE)
	target_link_libraries(MeshTool PRIVATE ${CARBON_FRAMEWORK})
endif()


