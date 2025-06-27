set(dds_tool_sources
    src/Utils/AssertUtils.cpp
    src/XPTools/DDSTool.cpp
    src/Utils/EndianUtils.c
    src/Utils/zip.c
    src/Utils/unzip.c
    src/Utils/BitmapUtils.cpp
    src/Utils/QuiltUtils.cpp
    src/Utils/FileUtils.cpp
    src/GUI/GUI_Unicode.cpp
)

add_executable(DDSTool ${dds_tool_sources})
target_include_directories(DDSTool PRIVATE
	Src/Utils
)

target_compile_definitions(DDSTool PRIVATE
	${BASIC_PLATFORM_DEFINES}
    -DUSE_JPEG=1
    -DUSE_TIF=1
)

target_link_libraries(DDSTool PRIVATE
	libsquish::libsquish
	TIFF::TIFF
	JPEG::JPEG
	PNG::PNG
)

target_compile_options(DDSTool PRIVATE -include ${CMAKE_SOURCE_DIR}/src/Obj/XDefs.h)
if (APPLE)
	target_link_libraries(DDSTool PRIVATE ${CARBON_FRAMEWORK})
endif()
