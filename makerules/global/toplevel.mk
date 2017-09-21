OUTDIR	:= ./build


##
# global environment
####################

.PHONY: all clean linkclean

PLATFORM	:= $(shell uname)
ARCHITECTURE	:= $(shell uname -m)
WD		:= $(PWD)


##
# platform specific environment
###############################

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

##
# architecture specific environment
###################################

ifdef PLAT_LINUX

MOCEXISTS	:= $(shell moc -E /dev/null > /dev/null 2>&1; echo $$?)
MOCQT4EXISTS	:= $(shell moc-qt4 -E /dev/null > /dev/null 2>&1; echo $$?)

ifneq ($(MOCEXISTS), 0)
ifneq ($(MOCQT4EXISTS), 0)
$(error neither 'moc' nor 'moc-qt4' found, install qt4-dev)
endif
endif

ifeq ($(MOCQT4EXISTS), 0)
MOC	:= moc-qt4
endif
ifeq ($(MOCEXISTS), 0)
MOC	:= moc
endif

ifeq ($(ARCHITECTURE), x86_64)
ifeq ($(cross), m32)
	MULTI_SUFFIX	:= 32
	M32_SWITCH	:= -m32
	BARE_LDFLAGS	:= -melf_i386
	OBJFORMAT	:= elf32-i386
	BINFORMAT	:= i386
else
	OBJFORMAT = elf64-x86-64
	BINFORMAT = i386:x86-64
endif #CROSS_32
	ARCH_X86_64	:= Yes
endif #ARCH_X86_64

ifeq ($(ARCHITECTURE), i386)
	OBJFORMAT	:= elf32-i386
	BINFORMAT	:= i386
	ARCH_I386	:= Yes
endif #ARCH_I386

ifeq ($(ARCHITECTURE), i686)
	OBJFORMAT	:= elf32-i386
	BINFORMAT	:= i386
	ARCH_I386	:= Yes
endif #ARCH_I686

endif #PLAT_LINUX

ifdef PLAT_MINGW
ifeq ($(cross), m32)
	MULTI_SUFFIX	:= 32
	M32_SWITCH	:= -m32
	OBJFORMAT 	:= pei-i386
	BINFORMAT 	:= i386
	WINDRES_OPTS	:= -F pe-i386
else
	OBJFORMAT 	:= pei-x86-64
	BINFORMAT 	:= i386
endif #CROSS_32
endif #PLAT_MINGW

ifdef PLAT_DARWIN
ifeq ($(cross), m32)
	MULTI_SUFFIX	:= 64
	M32_SWITCH	:= -m64
endif
endif


##
# intial tool flags
#############################

ifdef PLAT_LINUX
# if someone has a ppc linux machine, please define -DLIL/-DBIG in the code,
# remove them here and use the __ppc__ macro to resolve endianess issues
	DEFINES		:= -DLIN=1 -DIBM=0 -DAPL=0 -DLIL=1 -DBIG=0
	CFLAGS		:= $(M32_SWITCH) -Wno-deprecated-declarations -Wno-multichar -pipe -frounding-math
	CXXFLAGS	:= $(M32_SWITCH) -Wno-deprecated -Wno-deprecated-declarations -Wno-multichar -pipe -frounding-math
	LDFLAGS		:= $(M32_SWITCH) -static-libgcc
	BARE_LDFLAGS	:=
	STRIPFLAGS	:= -s -x
endif
ifdef PLAT_DARWIN
# -DLIL/-DBIG have to be defined in the code itself to support universal builds
	DEFINES		:= -DLIN=0 -DIBM=0 -DAPL=1
	CXXFLAGS	:= $(M32_SWITCH) -mmacosx-version-min=10.6 -Wno-deprecated -Wno-deprecated-declarations -Wno-multichar -frounding-math -fvisibility=hidden
	CFLAGS		:= $(M32_SWITCH) -mmacosx-version-min=10.6 -Wno-deprecated-declarations -Wno-multichar -frounding-math -fvisibility=hidden
	LDFLAGS		:= $(M32_SWITCH) -mmacosx-version-min=10.6
	STRIPFLAGS	:= -x
endif
ifdef PLAT_MINGW
	DEFINES		:= -DLIN=0 -DIBM=1 -DAPL=0 -DLIL=1 -DBIG=0 -DBOOST_THREAD_USE_LIB=1
	CFLAGS		:= $(M32_SWITCH) -Wno-deprecated-declarations -Wno-multichar -pipe -frounding-math
	CXXFLAGS	:= $(M32_SWITCH) -Wno-deprecated -Wno-deprecated-declarations -Wno-multichar -pipe -frounding-math
	LDFLAGS		:= $(M32_SWITCH) -static-libgcc
	BARE_LDFLAGS	:=
	STRIPFLAGS	:= -s -x
endif


##
# configuration specific environment
####################################

ifeq ($(conf), release_opt)
	CFLAGS		+= -O2 -fomit-frame-pointer -fstrict-aliasing
	CXXFLAGS	+= -O2 -fomit-frame-pointer -fstrict-aliasing
	DEFINES		+= -DDEV=0 -DNDEBUG
	StripDebug	:= Yes
else ifeq ($(conf), release)
	CFLAGS		+= -O1 
	CXXFLAGS	+= -O1
	DEFINES		+= -DDEV=0 -DNDEBUG
else ifeq ($(conf), debug)
	CFLAGS		+= -O0 -g
	CXXFLAGS	+= -O0 -g
	DEFINES		+= -DDEV=1
else ifeq ($(conf), phone)
	CFLAGS		+= -O1 -g
	CXXFLAGS	+= -O1 -g
	DEFINES		+= -DDEV=1 -DPHONE=1
# default to debug configuration
else
	conf		:= debug
	CFLAGS		+= -O1 -g
	CXXFLAGS	+= -O1 -g
	DEFINES		+= -DDEV=1
endif


##
# helper functions
#############################

print_clean	:= echo "[ RM  ]"
print_link	:= echo "[ LD  ]"
print_comp_cc	:= echo "[ CC  ]"
print_comp_cxx	:= echo "[ CXX ]"
print_comp_res	:= echo "[ RES ]"
print_arch	:= echo "[ AR  ]"
print_so	:= echo "[ SO  ]"
print_dep	:= echo "[ DEP ]"
print_moc	:= echo "[ MOC ]"
print_finished	:= echo "finished."
print_error	:= (echo "[ --FAILED-- ]" && false)


##
# tools
#############################

ifdef PLAT_DARWIN
CC	:= gcc
CXX	:= g++
LD	:= g++
AR	:= libtool
STRIP	:= strip
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

BUILDDIR	:= $(OUTDIR)$(MULTI_SUFFIX)/$(PLATFORM)/$(conf)

ifdef TARGET
include ./makerules/$(TARGET)
include ./makerules/global/paths.mk

ifeq ($(TYPE), EXECUTABLE)
	TYPE_EXECUTABLE := Yes
endif
ifeq ($(TYPE), LIBDYNAMIC)
	TYPE_LIBDYNAMIC := Yes
endif
ifeq ($(TYPE), LIBSTATIC)
	TYPE_LIBSTATIC := Yes
endif

ifndef REAL_TARGET
	REAL_TARGET := $(TARGET)
endif
REAL_TARGET := $(BUILDDIR)/$(REAL_TARGET)

ifdef PLAT_MINGW

ifdef TYPE_EXECUTABLE
REAL_TARGET := $(REAL_TARGET).exe
endif
ifdef TYPE_LIBDYNAMIC
REAL_TARGET := $(REAL_TARGET).p
endif
ifdef TYPE_LIBSTATIC
REAL_TARGET := $(REAL_TARGET).a
endif

else

ifdef TYPE_LIBDYNAMIC
REAL_TARGET := $(REAL_TARGET).p
endif
ifdef TYPE_LIBSTATIC
REAL_TARGET := $(REAL_TARGET).a
endif

endif #MINGW

endif #TARGET


##
# target type specific environment
##################################

ifdef TYPE_LIBDYNAMIC
	DEFINES		+= -DPIC
ifndef PLAT_MINGW
	CFLAGS		+= -fPIC
	CXXFLAGS	+= -fPIC
endif
ifdef PLAT_DARWIN
	LDFLAGS		+= -undefined dynamic_lookup
endif
endif


##
# determine intermediate filenames
##################################

BIN_SUFFIX	:= $(FORCEREBUILD_SUFFIX)$(MULTI_SUFFIX)

CSOURCES	:= $(filter %.c, $(SOURCES))
CDEPS		:= $(patsubst %.c, $(BUILDDIR)/obj/%$(BIN_SUFFIX).cdep, $(CSOURCES))
COBJECTS	:= $(patsubst %.c, $(BUILDDIR)/obj/%$(BIN_SUFFIX).o, $(CSOURCES))
COBJECTS	:= $(sort $(COBJECTS))

CXXSOURCES	:= $(filter %.cpp, $(SOURCES))
CXXDEPS		:= $(patsubst %.cpp, $(BUILDDIR)/obj/%$(BIN_SUFFIX).cppdep, $(CXXSOURCES))
CXXOBJECTS	:= $(patsubst %.cpp, $(BUILDDIR)/obj/%$(BIN_SUFFIX).o, $(CXXSOURCES))
MOCOBJ		:= $(patsubst %, $(BUILDDIR)/obj/%$(BIN_SUFFIX).moc.o, $(MOCSRC))
CXXOBJECTS	:= $(sort $(MOCOBJ) $(CXXOBJECTS))

RESOURCEOBJ	:= $(patsubst %, $(BUILDDIR)/obj/%$(BIN_SUFFIX).res.o, $(RESOURCES))
RESOURCEOBJ	+= $(patsubst %, $(BUILDDIR)/obj/%$(BIN_SUFFIX).winres.o, $(WIN_RESOURCES))

ALL_DEPS	:= $(sort $(CDEPS) $(CXXDEPS))
ALL_OBJECTS	:= $(sort $(COBJECTS) $(CXXOBJECTS))
ALL_OBJECTFILES	:= $(notdir $(ALL_OBJECTS))

SRC_DIRS	:= $(sort $(dir $(ALL_OBJECTS)))
BUILTINS	:= $(addsuffix builtin$(BIN_SUFFIX).o.$(TARGET), $(SRC_DIRS))
FINALBUILTIN	:= $(BUILDDIR)/obj/builtin$(BIN_SUFFIX).o.$(TARGET).final


##
# build rules
#####################

all: $(REAL_TARGET)

$(REAL_TARGET): $(ALL_OBJECTS) $(RESOURCEOBJ)
	@-mkdir -p $(dir $(@))
	@$(print_link) $(subst $(PWD)/, ./, $(abspath $(@)))

ifdef TYPE_EXECUTABLE
	@$(LD) -o $(@) $(MACARCHS) $(LIBPATHS) $(LDFLAGS) \
	$(ALL_OBJECTS) $(LIBS) $(RESOURCEOBJ) || $(print_error)
endif
ifdef TYPE_LIBDYNAMIC
ifdef PLAT_LINUX
	@$(LD) $(LDFLAGS) $(LIBPATHS) -shared \
	-Wl,-soname,$(notdir $(@)) -o $(@) $(ALL_OBJECTS) \
	$(LIBS) || $(print_error)
else
	@$(LD) $(MACARCHS) $(LDFLAGS) $(LIBPATHS) -shared \
	-o $(@) $(ALL_OBJECTS) \
	$(LIBS) || $(print_error)
endif
endif
ifdef StripDebug
	@$(STRIP) $(STRIPFLAGS) $(@)
endif
#ifdef PLAT_MINGW
#	@-cp libs/local$(MULTI_SUFFIX)/lib/*.dll $(dir $(@))
#endif
	@$(print_finished)

$(BUILDDIR)/obj/%$(BIN_SUFFIX).res.o : %
	@$(print_comp_res) $(subst $(PWD)/, ./, $(abspath $(<)))
	@-mkdir -p $(dir $(@))
	@cd $(dir $(<)) && $(OBJCOPY) -I binary -O $(OBJFORMAT) \
	-B $(BINFORMAT) $(notdir $(<)) $(WD)/$(@) && cd $(WD)

$(BUILDDIR)/obj/%$(BIN_SUFFIX).winres.o : %
	@$(print_comp_res) $(subst $(PWD)/, ./, $(abspath $(<)))
	@-mkdir -p $(dir $(@))
	@$(WINDRES) $(WINDRES_OPTS) -O coff -i $(<) -o $(@)

$(BUILDDIR)/obj/%$(BIN_SUFFIX).moc.o: %
	@$(print_moc) $(subst $(PWD)/, ./, $(abspath $(<)))
	@-mkdir -p $(dir $(@))
	@$(MOC) $(DEFINES) $(INCLUDEPATHS) -o$(@:.o=.cpp) $<
	@$(CXX) $(MACARCHS) $(CXXFLAGS) $(DEFINES) $(INCLUDEPATHS) \
	-c $(@:.o=.cpp) -o $(@) || $(print_error)

$(BUILDDIR)/obj/%$(BIN_SUFFIX).o: %.c
	@$(print_comp_cc) $(subst $(PWD)/, ./, $(abspath $(<)))
	@-mkdir -p $(dir $(@))
	@$(CC) $(MACARCHS) $(CFLAGS) $(DEFINES) $(INCLUDEPATHS) \
	-c $(<) -o $(@) || $(print_error)
	@$(CC) $(CFLAGS) $(DEFINES) $(INCLUDEPATHS) $(CPPFLAGS) \
	-MM -MT $(@) -MT $(@:.o=.cdep) -o $(@:.o=.cdep) $(<) \
	|| $(print_error)

$(BUILDDIR)/obj/%$(BIN_SUFFIX).o: %.cpp
	@$(print_comp_cxx) $(subst $(PWD)/, ./, $(abspath $(<)))
	@-mkdir -p $(dir $(@))
	@$(CXX) $(MACARCHS) $(CXXFLAGS) $(DEFINES) $(INCLUDEPATHS) \
	-c $(<) -o $(@) || $(print_error)
	@$(CXX) $(CXXFLAGS) $(DEFINES) $(INCLUDEPATHS) $(CPPFLAGS) \
	-MM -MT $(@) -MT $(@:.o=.cppdep) -o $(@:.o=.cppdep) $(<) \
	|| $(print_error)

$(FINALBUILTIN): $(BUILTINS)
	@$(print_link) $(subst $(PWD)/, ./, $(abspath $(@)))
	@-mkdir -p $(dir $(@))
	@ld $(MACARCHS) $(BARE_LDFLAGS) -Ur -o $(@) $(BUILTINS)

.SECONDEXPANSION:
$(BUILTINS): $$(filter $$(addprefix ./$$(dir $$(@)), $$(ALL_OBJECTFILES)), $$(ALL_OBJECTS))
	@$(print_link) $(subst $(PWD)/, ./, $(abspath $(@)))
	@-mkdir -p $(dir $(@))
	@ld $(MACARCHS) $(BARE_LDFLAGS) -r -o $(@) \
	$(filter $(addprefix ./$(dir $(@)), $(ALL_OBJECTFILES)), $(ALL_OBJECTS))

linkclean:
	@echo "removing builtins and binaries"
	@-rm -rf $(BUILTINS) $(FINALBUILTIN) $(REAL_TARGET)

clean:
	@echo "cleaning tree, removing $(OUTDIR)"
	@-rm -rf $(OUTDIR)

-include $(ALL_DEPS)
