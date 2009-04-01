##
# global environment
####################

.PHONY: all

PLATFORM	:= $(shell uname)
ARCHITECTURE	:= $(shell uname -m)
WD		:= $(PWD)
ifneq (, $(findstring MINGW, $(PLATFORM)))
	PLATFORM	:= Mingw
	PLAT_MINGW	:= Yes
endif
ifeq ($(PLATFORM), Linux)
	PLAT_LINUX	:= Yes
endif
ifeq ($(PLATFORM), Darwin)
	PLAT_DARWIN	:= Yes
endif

ifeq ($(ARCHITECTURE), i386)
	ARCH_I386	:= Yes
endif
ifeq ($(ARCHITECTURE), i686)
	ARCH_I386	:= Yes
endif
ifeq ($(ARCHITECTURE), x86_64)
	ARCH_X86_64	:= Yes
endif

ifeq ($(cross), mingw64)
	MULTI_SUFFIX	:= 64
	CROSSPREFIX	:= x86_64-pc-mingw32-
	ARCHITECTURE	:= x86_64
endif
ifeq ($(cross), m32)
	MULTI_SUFFIX	:= 32
	M32_SWITCH	:= -m32
endif

ifndef PLAT_DARWIN
	ECHOFLAGS	:= -e
endif

clearscreen	:= echo $(ECHOFLAGS) "\033[2J\033[H"
print_clean	:= echo $(ECHOFLAGS) "\033[0;31m[ RM  ]\033[0m"
print_link	:= echo $(ECHOFLAGS) "\033[0;34m[ LD  ]\033[0m:"
print_comp_cc	:= echo $(ECHOFLAGS) "\033[0;32m[ CC  ]\033[0m:"
print_comp_cxx	:= echo $(ECHOFLAGS) "\033[0;32m[ CXX ]\033[0m:"
print_comp_res	:= echo $(ECHOFLAGS) "\033[0;32m[ RES ]\033[0m:"
print_arch	:= echo $(ECHOFLAGS) "\033[0;34m[ AR  ]\033[0m:"
print_so	:= echo $(ECHOFLAGS) "\033[0;34m[ SO  ]\033[0m:"
print_dep	:= echo $(ECHOFLAGS) "\033[0;34m[ DEP ]\033[0m:"
print_finished	:= echo $(ECHOFLAGS) "\033[0;32m finished.\033[0m"
print_error	:= (echo $(ECHOFLAGS) "\033[0;31m[ --FAILED-- ]\033[0m" && false)


##
# tools
#############################

ifdef PLAT_DARWIN
CC	:= gcc-4.2
CXX	:= g++-4.2
LD	:= g++-4.2
AR	:= libtool
else
CC	:= $(CROSSPREFIX)gcc
CXX	:= $(CROSSPREFIX)g++
LD	:= $(CROSSPREFIX)g++
AR	:= $(CROSSPREFIX)ar
OBJCOPY	:= $(CROSSPREFIX)objcopy
STRIP	:= $(CROSSPREFIX)strip
WINDRES	:= $(CROSSPREFIX)windres
endif


##
# target specific environment
#############################

include ./makerules/$(shell basename $(TARGET))
ifdef REAL_TARGET
REAL_TARGET := $(REAL_TARGET)$(MULTI_SUFFIX)
else
REAL_TARGET := $(TARGET)$(MULTI_SUFFIX)
endif

##
# architecture specific environment
###################################


ifdef ARCH_I386
	OBJFORMAT = elf32-i386
	BINFORMAT = i386
endif
ifdef ARCH_X86_64
	OBJFORMAT = elf64-x86-64
	BINFORMAT = i386:x86-64
endif

ifdef PLAT_MINGW
# uname -m gives i686, regardless if on win32 or win64
ifdef ARCH_I386
	OBJFORMAT = pei-i386
	BINFORMAT = i386
endif
ifdef ARCH_X86_64
	OBJFORMAT = pei-x86-64
	BINFORMAT = i386
endif
endif


##
# platform specific environment
###############################

ifdef PLAT_LINUX
# if someone has a ppc linux machine, please define -DLIL/-DBIG in the code,
# remove them here and use the __ppc__ macro to resolve endianess issues
	DEFINES		:= $(DEFINES) -DLIN=1 -DIBM=0 -DAPL=0 -DLIL=1 -DBIG=0
	CFLAGS		:= $(CFLAGS) $(M32_SWITCH) -fvisibility=hidden -Wno-multichar
	CXXFLAGS	:= $(CXXFLAGS) $(M32_SWITCH) -fvisibility=hidden -Wno-deprecated -Wno-multichar
	LDFLAGS		:= $(LDFLAGS) $(M32_SWITCH) -rdynamic
endif
ifdef PLAT_DARWIN
# -DLIL/-DBIG have to be defined in the code itself to support universal builds
	DEFINES		:= $(DEFINES) -DLIN=0 -DIBM=0 -DAPL=1
	CXXFLAGS	:= $(CXXFLAGS) -fvisibility=hidden -mmacosx-version-min=10.4 -Wno-multichar
	CFLAGS		:= $(CFLAGS) -fvisibility=hidden -mmacosx-version-min=10.4 -Wno-multichar
	LDFLAGS		:= $(LDFLAGS) -mmacosx-version-min=10.4 -Z
	MACARCHS	:= -arch i386 -arch ppc
endif
ifdef PLAT_MINGW
	DEFINES		:= $(DEFINES) -DLIN=0 -DIBM=1 -DAPL=0 -DLIL=1 -DBIG=0
	CFLAGS		:= $(CFLAGS) -Wno-multichar
	CXXFLAGS	:= $(CXXFLAGS) -Wno-deprecated -Wno-multichar
	LDFLAGS		:= $(LDFLAGS)
endif


##
# target type specific environment
##################################

ifeq ($(TYPE), LIBDYNAMIC)
	DEFINES		:= $(DEFINES) -DPIC
ifneq ($(PLATFORM), Mingw)
	CFLAGS		:= $(CFLAGS) -fPIC
	CXXFLAGS	:= $(CXXFLAGS) -fPIC
endif
endif


##
# configuration specific environment
####################################

ifeq ($(conf), release_opt)
	CFLAGS		:= $(CFLAGS) -O2 -g
	CXXFLAGS	:= $(CXXFLAGS) -O2 -g
	DEFINES		:= $(DEFINES) -DDEV=0
else ifeq ($(conf), release)
	CFLAGS		:= $(CFLAGS) -O0 -g
	CXXFLAGS	:= $(CXXFLAGS) -O0 -g
	DEFINES		:= $(DEFINES) -DDEV=0
else ifeq ($(conf), debug_opt)
	CFLAGS		:= $(CFLAGS) -O2 -g
	CXXFLAGS	:= $(CXXFLAGS) -O2 -g
	DEFINES		:= $(DEFINES) -DDEV=1
else ifeq ($(conf), debug)
	CFLAGS		:= $(CFLAGS) -O0 -g
	CXXFLAGS	:= $(CXXFLAGS) -O0 -g
	DEFINES		:= $(DEFINES) -DDEV=1
# default to debug configuration
else
	CFLAGS		:= $(CFLAGS) -O0 -g
	CXXFLAGS	:= $(CXXFLAGS) -O0 -g
	DEFINES		:= $(DEFINES) -DDEV=1
endif


##
# determine intermediate filenames
##################################

CXXSOURCES	:= $(filter %.cpp, $(SOURCES))
CCSOURCES	:= $(filter %.c, $(SOURCES))
CCDEPS		:= $(patsubst %.c, $(BUILDDIR)/%$(FORCEREBUILD_SUFFIX)$(MULTI_SUFFIX).dep, $(CCSOURCES))
CCOBJECTS	:= $(patsubst %.c, $(BUILDDIR)/%$(FORCEREBUILD_SUFFIX)$(MULTI_SUFFIX).o, $(CCSOURCES))
CXXDEPS		:= $(patsubst %.cpp, $(BUILDDIR)/%$(FORCEREBUILD_SUFFIX)$(MULTI_SUFFIX).dep, $(CXXSOURCES))
CXXOBJECTS	:= $(patsubst %.cpp, $(BUILDDIR)/%$(FORCEREBUILD_SUFFIX)$(MULTI_SUFFIX).o, $(CXXSOURCES))
RESOURCEOBJ	:= $(patsubst %, $(BUILDDIR)/%$(FORCEREBUILD_SUFFIX)$(MULTI_SUFFIX).ro, $(RESOURCES))
WIN_RESOURCEOBJ	:= $(patsubst %.rc, $(BUILDDIR)/%$(FORCEREBUILD_SUFFIX)$(MULTI_SUFFIX).res, $(WIN_RESOURCES))


##
# build rules
#####################

all: $(REAL_TARGET)

-include $(CCDEPS) $(CXXDEPS)

$(REAL_TARGET): $(CCOBJECTS) $(CXXOBJECTS) $(CCDEPS) $(CXXDEPS) $(RESOURCEOBJ) $(WIN_RESOURCEOBJ)
	@-mkdir -p $(dir $(@))

# static library

ifeq ($(TYPE), LIBSTATIC)
	@$(print_arch) $(subst $(PWD)/, ./, $(abspath $(@)))
ifeq ($(PLATFORM), Darwin)
	@$(AR) -static -o $(@) $(ARFLAGS) $(CCOBJECTS) $(CXXOBJECTS) $(WIN_RESOURCEOBJ) $(RESOURCEOBJ) || $(print_error)
else
	@$(AR) $(ARFLAGS) $(@) $(CCOBJECTS) $(CXXOBJECTS) $(WIN_RESOURCEOBJ) $(RESOURCEOBJ) || $(print_error)
endif
endif

# shared library

ifeq ($(TYPE), LIBDYNAMIC)
	@$(print_so) $(subst $(PWD)/, ./, $(abspath $(@)))
ifeq ($(PLATFORM), Linux)
	@$(LD) $(LDFLAGS) $(LIBPATHS) -rdynamic -shared -Wl,-export-dynamic,-soname,$(notdir $(@)) -o $(@) $(CCOBJECTS) $(CXXOBJECTS) $(WIN_RESOURCEOBJ) $(RESOURCEOBJ) $(LIBS) $(STDLIBS) || $(print_error)
endif
ifeq ($(PLATFORM), Darwin)
	@$(LD) $(MACARCHS) $(LDFLAGS) $(LIBPATHS) -rdynamic -shared -o $(@) $(CCOBJECTS) $(CXXOBJECTS) $(WIN_RESOURCEOBJ) $(RESOURCEOBJ) $(LIBS) $(STDLIBS) || $(print_error)
endif
ifeq ($(PLATFORM), Mingw)
	@$(LD) $(LDFLAGS) $(LIBPATHS) -shared -Wl,-export-dynamic,-soname,$(notdir $(@)) -o $(@) $(CCOBJECTS) $(CXXOBJECTS) $(WIN_RESOURCEOBJ) $(RESOURCEOBJ) $(LIBS) $(STDLIBS) || $(print_error)
endif
endif

# executable

ifeq ($(TYPE), EXECUTABLE)
	@$(print_link) $(subst $(PWD)/, ./, $(abspath $(@)))
	@$(LD) $(MACARCHS) $(LDFLAGS) $(LIBPATHS) -o $(@) $(CCOBJECTS) $(CXXOBJECTS) $(WIN_RESOURCEOBJ) $(RESOURCEOBJ) $(LIBS) $(STDLIBS) || $(print_error)
endif

# debug information

ifneq ($(TYPE), LIBSTATIC)
	@$(OBJCOPY) --only-keep-debug $(@) $(@).debug
	@$(STRIP) -s -x $(@)
	@cd  $(dir $(@)) && $(OBJCOPY) --add-gnu-debuglink=$(notdir $(@)).debug $(notdir $(@)) && cd $(WD)
	@chmod 0644 $(@).debug
endif
#TODO: add Darwin, at least strip binaries
	@$(print_finished)

# basic rules

$(RESOURCEOBJ): $(BUILDDIR)/%$(FORCEREBUILD_SUFFIX)$(MULTI_SUFFIX).ro : %
	@$(print_comp_res) $(subst $(PWD)/, ./, $(abspath $(<)))
	@-mkdir -p $(dir $(@))
	@cd $(dir $(<)) && $(OBJCOPY) -I binary -O $(OBJFORMAT) -B $(BINFORMAT) $(notdir $(<)) $(WD)/$(@) && cd $(WD)

$(WIN_RESOURCEOBJ): $(BUILDDIR)/%$(FORCEREBUILD_SUFFIX)$(MULTI_SUFFIX).res : %.rc
	@$(print_comp_res) $(subst $(PWD)/, ./, $(abspath $(<)))
	@-mkdir -p $(dir $(@))
	@$(WINDRES) $< -O coff -o $(@)

$(CCOBJECTS): $(BUILDDIR)/%$(FORCEREBUILD_SUFFIX)$(MULTI_SUFFIX).o: %.c $(BUILDDIR)/%$(FORCEREBUILD_SUFFIX)$(MULTI_SUFFIX).dep
	@$(print_comp_cc) $(subst $(PWD)/, ./, $(abspath $(<)))
	@-mkdir -p $(dir $(@))
	@$(CC) $(MACARCHS) $(CFLAGS) $(DEFINES) $(INCLUDEPATHS) -c $(<) -o $(@) || $(print_error)

$(CXXOBJECTS): $(BUILDDIR)/%$(FORCEREBUILD_SUFFIX)$(MULTI_SUFFIX).o: %.cpp $(BUILDDIR)/%$(FORCEREBUILD_SUFFIX)$(MULTI_SUFFIX).dep
	@$(print_comp_cxx) $(subst $(PWD)/, ./, $(abspath $(<)))
	@-mkdir -p $(dir $(@))
	@$(CXX) $(MACARCHS) $(CXXFLAGS) $(DEFINES) $(INCLUDEPATHS) -c $(<) -o $(@) || $(print_error)

$(CCDEPS): $(BUILDDIR)/%$(FORCEREBUILD_SUFFIX)$(MULTI_SUFFIX).dep: %.c
	@$(print_dep) $(subst $(PWD)/, ./, $(abspath $(<)))
	@-mkdir -p $(dir $(@))
	@$(CC) $(CFLAGS) $(DEFINES) $(INCLUDEPATHS) -MM $< -MT $(@) -MT $(<:.c=.o) -o $(@) || $(print_error)

$(CXXDEPS): $(BUILDDIR)/%$(FORCEREBUILD_SUFFIX)$(MULTI_SUFFIX).dep: %.cpp
	@$(print_dep) $(subst $(PWD)/, ./, $(abspath $(<)))
	@-mkdir -p $(dir $(@))
	@$(CXX) $(CXXFLAGS) $(DEFINES) $(INCLUDEPATHS) -MM $< -MT $(@) -MT $(<:.cpp=.o) -o $(@) || $(print_error)
