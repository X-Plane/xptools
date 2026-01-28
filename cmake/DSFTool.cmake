add_executable(DSFTool
    src/DSF/DSFLib.cpp
    src/DSF/DSFLib.h
    src/DSF/DSFLib_Print.cpp
    src/DSF/DSFLibWrite.cpp
    src/DSF/DSFPointPool.cpp
    src/DSF/DSFPointPool.h
    src/DSF/DSFDefs.h
    src/DSFTools/DSFToolCmdLine.cpp
    src/DSFTools/DSF2Text.cpp
    src/DSFTools/DSF2Text.h
    src/Utils/AssertUtils.cpp
    src/Utils/AssertUtils.h
    src/Utils/EndianUtils.c
    src/Utils/EndianUtils.h
    src/Utils/FileUtils.cpp
    src/Utils/FileUtils.h
	src/Utils/MemFileUtils.cpp
	src/Utils/MemFileUtils.h
    src/GUI/GUI_Unicode.cpp
    src/GUI/GUI_Unicode.h
    src/Utils/md5.c
    src/Utils/md5.h
    src/Utils/zip.c
    src/Utils/zip.h
    src/Utils/unzip.c
    src/Utils/unzip.h
    src/Utils/XChunkyFileUtils.cpp
    src/Utils/XChunkyFileUtils.h
    src/DSF/tri_stripper_101/tri_stripper.cpp
    src/DSF/tri_stripper_101/tri_stripper.h

    src/lzma19/C/7zArcIn.c
    src/lzma19/C/7zAlloc.c
    src/lzma19/C/7zAlloc.h
    src/lzma19/C/7zBuf.c
    src/lzma19/C/7zBuf.h
    src/lzma19/C/7zCrc.c
    src/lzma19/C/7zCrc.h
    src/lzma19/C/7zCrcOpt.c
    src/lzma19/C/7zDec.c
    src/lzma19/C/7zFile.c
    src/lzma19/C/7zFile.h
    src/lzma19/C/7zStream.c
    src/lzma19/C/Bcj2.c
    src/lzma19/C/Bcj2.h
    src/lzma19/C/Bra.c
    src/lzma19/C/Bra.h
    src/lzma19/C/Bra86.c
    src/lzma19/C/BraIA64.c
    src/lzma19/C/CpuArch.c
    src/lzma19/C/CpuArch.h
    src/lzma19/C/Delta.c
    src/lzma19/C/Delta.h
    src/lzma19/C/LzmaDec.c
    src/lzma19/C/LzmaDec.h
    src/lzma19/C/Lzma2Dec.c
    src/lzma19/C/Lzma2Dec.h
    src/lzma19/C/7zTypes.h
    src/lzma19/C/Compiler.h
)

if(MSVC)
	target_compile_options(DSFTool PRIVATE /FI ${CMAKE_SOURCE_DIR}/src/Obj/XDefs.h)
else()
	target_compile_options(DSFTool PRIVATE -include ${CMAKE_SOURCE_DIR}/src/Obj/XDefs.h)
endif()

target_compile_definitions(DSFTool PRIVATE ${BASIC_PLATFORM_DEFINES})
target_link_libraries(DSFTool PRIVATE ZLIB::ZLIB)
target_include_directories(DSFTool PRIVATE
    src/GUI
    src/Utils
	src/DSF
	src/DSF/tri_stripper_101
	src/lzma19/C
)

if (APPLE)
	target_link_libraries(DSFTool PRIVATE ${CARBON_FRAMEWORK})
endif()
