##
# project wide include and library searchpaths
##############################################

LIBPATHS += -L./libs/local$(MULTI_SUFFIX)/lib

INCLUDEPATHS += -I./libs/local$(MULTI_SUFFIX)/include
INCLUDEPATHS += -I./libs/local$(MULTI_SUFFIX)/include/freetype2
INCLUDEPATHS += -I./src/linuxinit
INCLUDEPATHS += -I./src/WEDTCE
INCLUDEPATHS += -I./src/Env
INCLUDEPATHS += -I./src/DSF
INCLUDEPATHS += -I./src/GUI
INCLUDEPATHS += -I./src/GUI/mmenu
INCLUDEPATHS += -I./src/Interfaces
INCLUDEPATHS += -I./src/Obj
INCLUDEPATHS += -I./src/ObjEdit
INCLUDEPATHS += -I./src/OGLE
INCLUDEPATHS += -I./src/UI
INCLUDEPATHS += -I./src/WEDCore
INCLUDEPATHS += -I./src/WEDDocs
INCLUDEPATHS += -I./src/WEDEntities
INCLUDEPATHS += -I./src/WEDImportExport
INCLUDEPATHS += -I./src/WEDLayers
INCLUDEPATHS += -I./src/WEDLibrary
INCLUDEPATHS += -I./src/WEDMap
INCLUDEPATHS += -I./src/WEDProperties
INCLUDEPATHS += -I./src/WEDResources
INCLUDEPATHS += -I./src/WEDWindows
INCLUDEPATHS += -I./src/XPCompat
INCLUDEPATHS += -I./src/XPWidgets
INCLUDEPATHS += -I./src/Utils
INCLUDEPATHS += -I./src/XESCore
INCLUDEPATHS += -I./src/XESTools
INCLUDEPATHS += -I./src/Installer
INCLUDEPATHS += -I./src/Network
INCLUDEPATHS += -I./src/DSF/tri_stripper_101
INCLUDEPATHS += -I./src/RenderFarmUI
INCLUDEPATHS += -I./src/RawImport
INCLUDEPATHS += -I./src/Tiger
INCLUDEPATHS += -I./src/VPF
INCLUDEPATHS += -I./src/SDTS
INCLUDEPATHS += -I./SDK/ac3d

ifdef PLAT_LINUX
INCLUDEPATHS += -I./libs/local$(MULTI_SUFFIX)/include/mesa
endif #PLAT_LINUX
