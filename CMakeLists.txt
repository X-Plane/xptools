cmake_minimum_required(VERSION 3.10)
project(xptools)

include(cmake/xib_helper.cmake)
if (APPLE)
	include(cmake/mac.cmake)
endif()

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (APPLE)
	find_library(CARBON_FRAMEWORK Carbon REQUIRED)
	find_library(OPENGL_FRAMEWORK OpenGL REQUIRED)
	find_library(APPKIT_FRAMEWORK AppKit REQUIRED)
endif()

find_package(CGAL REQUIRED)
find_package(expat REQUIRED)
find_package(proj REQUIRED)
find_package(freetype REQUIRED)
find_package(ZLIB REQUIRED)
find_package(PNG REQUIRED)
find_package(TIFF REQUIRED)
find_package(geotiff REQUIRED)
find_package(JPEG REQUIRED)
find_package(libsquish REQUIRED)
find_package(shapelib REQUIRED)

if(WIN32)
    set(BASIC_PLATFORM_DEFINES
        LIN=0
        IBM=1
        APL=0
    )
elseif(APPLE)
    set(BASIC_PLATFORM_DEFINES
        LIN=0
        IBM=0
        APL=1
        _LIBCPP_ENABLE_CXX17_REMOVED_UNARY_BINARY_FUNCTION=1
    )
elseif(UNIX)
    set(BASIC_PLATFORM_DEFINES
        LIN=1
        IBM=0
        APL=0
        _LIBCPP_ENABLE_CXX17_REMOVED_UNARY_BINARY_FUNCTION=1
    )
endif()

# Build-type specific definitions
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	set(BASIC_PLATFORM_DEFINES "${BASIC_PLATFORM_DEFINES};DEV=1;BIG=0;LIL=1")
else()
	set(BASIC_PLATFORM_DEFINES "${BASIC_PLATFORM_DEFINES};DEV=0;BIG=0;LIL=1")
endif()

#NOTE: RenderFarm and RenderFarmUI intentionally not yet cmake'ified - not sure we'll need these tools going forward,
# and I think they're likely broken like MeshTool due to their reliance on CGAL.

include(cmake/DDSTool.cmake)
include(cmake/DSFTool.cmake)
include(cmake/MeshTool.cmake)
include(cmake/ObjView.cmake)
include(cmake/WED.cmake)
include(cmake/XGrinder.cmake)
