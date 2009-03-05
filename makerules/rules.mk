##
# global environment
####################

.PHONY: all

PLATFORM		:= $(shell uname)
ARCHITECTURE	:= $(shell uname -m)
WD				:= $(PWD)
ifneq (, $(findstring MINGW, $(PLATFORM)))
	PLATFORM	:= Mingw
endif
ifeq ($(cross), mingw64)
	MULTI_PREFIX	:= 64_
	MULTI_SUFFIX	:= 64
	CROSSPREFIX		:= x86_64-pc-mingw32-
	ARCHITECTURE	:= x86_64
endif
ifeq ($(cross), m32)
	MULTI_PREFIX	:= 32_
	MULTI_SUFFIX	:= 32
	M32_SWITCH		:= -m32
endif

ifeq ($(PLATFORM), Darwin)
	ECHOFLAGS	:=
else
	ECHOFLAGS	:= -e
endif

ifdef NOTCOLORED
print_clean		:= echo "[ deleting all generated files  ]"
print_link		:= echo "[ linking executable      ]: "
print_comp_cc	:= echo "[ compiling c object      ]: "
print_comp_cxx	:= echo "[ compiling cpp object    ]: "
print_comp_res	:= echo "[ generating resource     ]: "
print_arch		:= echo "[ creating static library ]: "
print_so		:= echo "[ creating shared library ]: "
print_dep		:= echo "[ calculating dependency  ]: "
print_finished	:= echo " finished."
print_error		:= (echo "[ --FAILED-- ]" && false)
else
clearscreen		:= echo $(ECHOFLAGS) "\033[2J\033[H"
print_clean		:= echo $(ECHOFLAGS) "\033[0;31m[ deleting all generated files  ]\033[0m"
print_link		:= echo $(ECHOFLAGS) "\033[0;34m[ linking executable      ]\033[0m: "
print_comp_cc	:= echo $(ECHOFLAGS) "\033[0;32m[ compiling c object      ]\033[0m: "
print_comp_cxx	:= echo $(ECHOFLAGS) "\033[0;32m[ compiling cpp object    ]\033[0m: "
print_comp_res	:= echo $(ECHOFLAGS) "\033[0;32m[ generating resource     ]\033[0m: "
print_arch		:= echo $(ECHOFLAGS) "\033[0;34m[ creating static library ]\033[0m: "
print_so		:= echo $(ECHOFLAGS) "\033[0;34m[ creating shared library ]\033[0m: "
print_dep		:= echo $(ECHOFLAGS) "\033[0;34m[ calculating dependency  ]\033[0m: "
print_finished	:= echo $(ECHOFLAGS) "\033[0;32m finished.\033[0m"
print_error		:= (echo $(ECHOFLAGS) "\033[0;31m[ --FAILED-- ]\033[0m" && false)
endif


##
# tools
#############################

ifeq ($(PLATFORM), Darwin)
CC			:= gcc-4.2
CXX			:= g++-4.2
LD			:= g++-4.2
AR			:= libtool
else
CC			:= $(CROSSPREFIX)gcc
CXX			:= $(CROSSPREFIX)g++
LD			:= $(CROSSPREFIX)g++
AR			:= $(CROSSPREFIX)ar
OBJCOPY		:= $(CROSSPREFIX)objcopy
STRIP		:= $(CROSSPREFIX)strip
endif


##
# target specific environment
#############################

include makerules/$(shell basename $(TARGET))
REAL_TARGET := $(TARGET)$(MULTI_SUFFIX)


##
# architecture specific environment
###################################

ifeq ($(ARCHITECTURE), i386)
	OBJFORMAT = elf32-i386
	BINFORMAT = i386
endif
ifeq ($(ARCHITECTURE), i686)
	OBJFORMAT = elf32-i386
	BINFORMAT = i386
endif
ifeq ($(ARCHITECTURE), x86_64)
	OBJFORMAT = elf64-x86-64
	BINFORMAT = i386:x86-64
endif
ifeq ($(PLATFORM), Mingw)
# uname -m gives i686, regardless if on win32 or win64
ifeq ($(ARCHITECTURE), i686)
	OBJFORMAT = pei-i386
	BINFORMAT = i386
endif
ifeq ($(ARCHITECTURE), x86_64)
	OBJFORMAT = pei-x86-64
	BINFORMAT = i386
endif
endif


##
# platform specific environment
###############################

ifeq ($(PLATFORM), Linux)
	DEFINES		:= $(DEFINES) -DLIN=1 -DIBM=0 -DAPL=0 -DLIL=1 -DBIG=0
	CFLAGS		:= $(CFLAGS) $(M32_SWITCH) -fvisibility=hidden
	CXXFLAGS	:= $(CXXFLAGS) $(M32_SWITCH) -fvisibility=hidden -Wno-deprecated
	LDFLAGS		:= $(LDFLAGS) $(M32_SWITCH) -rdynamic
endif
ifeq ($(PLATFORM), Darwin)
	DEFINES		:= $(DEFINES) -DLIN=0 -DIBM=0 -DAPL=1 -DLIL=1 -DBIG=0
	CXXFLAGS	:= $(CXXFLAGS) -fvisibility=hidden -mmacosx-version-min=10.4
	CFLAGS		:= $(CFLAGS) -fvisibility=hidden -mmacosx-version-min=10.4
	LDFLAGS		:= $(LDFLAGS) -mmacosx-version-min=10.4
endif
ifeq ($(PLATFORM), Mingw)
	DEFINES		:= $(DEFINES) -DLIN=0 -DIBM=1 -DAPL=0 -DLIL=1 -DBIG=0
	CFLAGS		:= $(CFLAGS)
	CXXFLAGS	:= $(CXXFLAGS)
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
CCDEPS		:= $(patsubst %.c, $(BUILDDIR)/$(MULTI_PREFIX)%$(FORCEREBUILD_SUFFIX).dep, $(CCSOURCES))
CCOBJECTS	:= $(patsubst %.c, $(BUILDDIR)/$(MULTI_PREFIX)%$(FORCEREBUILD_SUFFIX).o, $(CCSOURCES))
CXXDEPS		:= $(patsubst %.cpp, $(BUILDDIR)/$(MULTI_PREFIX)%$(FORCEREBUILD_SUFFIX).dep, $(CXXSOURCES))
CXXOBJECTS	:= $(patsubst %.cpp, $(BUILDDIR)/$(MULTI_PREFIX)%$(FORCEREBUILD_SUFFIX).o, $(CXXSOURCES))
RESOURCEOBJ	:= $(patsubst %, $(BUILDDIR)/$(MULTI_PREFIX)%$(FORCEREBUILD_SUFFIX).ro, $(RESOURCES))



##
# build rules
#####################

all: $(REAL_TARGET)

-include $(CCDEPS) $(CXXDEPS)

$(REAL_TARGET):  $(CCOBJECTS) $(CXXOBJECTS) $(CCDEPS) $(CXXDEPS) $(RESOURCEOBJ)
	-mkdir -p $(dir $(@))

# static library

ifeq ($(TYPE), LIBSTATIC)
	$(print_arch) $@
ifeq ($(PLATFORM), Darwin)
	$(AR) -static -o $@ $(ARFLAGS) $(CCOBJECTS) $(CXXOBJECTS) $(RESOURCEOBJ) || $(print_error)
else
	$(AR) $(ARFLAGS) $@ $(CCOBJECTS) $(CXXOBJECTS) $(RESOURCEOBJ) || $(print_error)
endif
endif

# shared library

ifeq ($(TYPE), LIBDYNAMIC)
	$(print_so) $@
ifeq ($(PLATFORM), Linux)
	$(LD) $(LDFLAGS) $(LIBPATHS) -rdynamic -shared -Wl,-export-dynamic,-soname,$(notdir $(@)) -o $@ $(CCOBJECTS) $(CXXOBJECTS) $(RESOURCEOBJ) $(LIBS) $(STDLIBS) || $(print_error)
endif
ifeq ($(PLATFORM), Darwin)
	$(LD) $(MACARCHS) $(LDFLAGS) $(LIBPATHS) -rdynamic -shared -o $@ $(CCOBJECTS) $(CXXOBJECTS) $(RESOURCEOBJ) $(LIBS) $(STDLIBS) || $(print_error)
endif
ifeq ($(PLATFORM), Mingw)
	$(LD) $(LDFLAGS) $(LIBPATHS) -shared -Wl,-export-dynamic,-soname,$(notdir $(@)) -o $@ $(CCOBJECTS) $(CXXOBJECTS) $(RESOURCEOBJ) $(LIBS) $(STDLIBS) || $(print_error)
endif
endif

# executable

ifeq ($(TYPE), EXECUTABLE)
	$(print_link) $@
	$(LD) $(LDFLAGS) $(LIBPATHS) -o $@ $(CCOBJECTS) $(CXXOBJECTS) $(RESOURCEOBJ) $(LIBS) $(STDLIBS) || $(print_error)
endif

# debug information

ifneq ($(TYPE), LIBSTATIC)
ifeq ($(conf), release)
ifeq ($(PLATFORM), Linux)
	$(OBJCOPY) --only-keep-debug $@ $(@).debug
	$(STRIP) -s -x $@
	cd  $(dir $(@)) && $(OBJCOPY) --add-gnu-debuglink=$(notdir $(@)).debug $(notdir $(@)) && cd $(WD)
	chmod 0644 $(@).debug
endif
ifeq ($(PLATFORM), Mingw)
	$(OBJCOPY) --only-keep-debug $@.exe $(@).debug
	$(STRIP) -s -x $@.exe
	cd  $(dir $(@)) && $(OBJCOPY) --add-gnu-debuglink=$(notdir $(@)).debug $(notdir $(@)).exe && cd $(WD)
	chmod 0644 $(@).debug
endif
endif
endif
	$(print_finished)

# basic rules

$(RESOURCEOBJ): $(BUILDDIR)/$(MULTI_PREFIX)%$(FORCEREBUILD_SUFFIX).ro : %
	$(print_comp_res) $<
	-mkdir -p $(dir $(@))
	cd  $(dir $(<)) && $(OBJCOPY) -I binary -O $(OBJFORMAT) -B $(BINFORMAT) $(notdir $(<)) $(WD)/$(@) && cd $(WD)

$(CCOBJECTS): $(BUILDDIR)/$(MULTI_PREFIX)%$(FORCEREBUILD_SUFFIX).o: %.c $(BUILDDIR)/$(MULTI_PREFIX)%$(FORCEREBUILD_SUFFIX).dep
	$(print_comp_cc) $<
	-mkdir -p $(dir $(@))
	$(CC) $(MACARCHS) $(CFLAGS) $(DEFINES) $(INCLUDEPATHS) -c $< -o $@ || $(print_error)

$(CXXOBJECTS): $(BUILDDIR)/$(MULTI_PREFIX)%$(FORCEREBUILD_SUFFIX).o: %.cpp $(BUILDDIR)/$(MULTI_PREFIX)%$(FORCEREBUILD_SUFFIX).dep
	$(print_comp_cxx) $<
	-mkdir -p $(dir $(@))
	$(CXX) $(MACARCHS) $(CXXFLAGS) $(DEFINES) $(INCLUDEPATHS) -c $< -o $@ || $(print_error)

$(CCDEPS): $(BUILDDIR)/$(MULTI_PREFIX)%$(FORCEREBUILD_SUFFIX).dep: %.c
	$(print_dep) $<
	-mkdir -p $(dir $(@))
	$(CC) $(CFLAGS) $(DEFINES) $(INCLUDEPATHS) -MM $< -MT $@ -MT $(<:.c=.o) -o $@ || $(print_error)

$(CXXDEPS): $(BUILDDIR)/$(MULTI_PREFIX)%$(FORCEREBUILD_SUFFIX).dep: %.cpp
	$(print_dep) $<
	-mkdir -p $(dir $(@))
	$(CXX) $(CXXFLAGS) $(DEFINES) $(INCLUDEPATHS) -MM $< -MT $@ -MT $(<:.cpp=.o) -o $@ || $(print_error)
