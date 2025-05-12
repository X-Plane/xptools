add_executable(DSFTool
    src/DSF/DSFLib.cpp
    src/DSF/DSFLibWrite.cpp
    src/DSF/DSFPointPool.cpp
    src/DSFTools/DSFToolCmdLine.cpp
    src/DSFTools/DSF2Text.cpp
    src/Utils/AssertUtils.cpp
    src/Utils/EndianUtils.c
    src/Utils/FileUtils.cpp
    src/GUI/GUI_Unicode.cpp
    src/Utils/md5.c
    src/Utils/zip.c
    src/Utils/unzip.c
    src/Utils/XChunkyFileUtils.cpp
    src/DSF/tri_stripper_101/tri_stripper.cpp

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

target_compile_definitions(DSFTool PRIVATE ${BASIC_PLATFORM_DEFINES})
target_compile_options(DSFTool PRIVATE -include ${CMAKE_SOURCE_DIR}/src/Obj/XDefs.h)
target_link_libraries(DSFTool PRIVATE ZLIB::ZLIB)
target_include_directories(DSFTool PRIVATE
	Src/Utils
	Src/DSF
	Src/DSF/tri_stripper_101
	src/lzma19/C
)

if (APPLE)
	target_link_libraries(DSFTool PRIVATE ${CARBON_FRAMEWORK})
endif()
