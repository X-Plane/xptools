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
ifndef conf
conf	:= debug
endif
BUILDDIR	:= $(OUTDIR)/$(PLATFORM)/$(conf)


##
# architecture specific environment
###################################

ifdef PLAT_LINUX

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
# FIXME: uname -m gives i686, regardless if on win32 or win64
ifdef ARCH_I386
	OBJFORMAT 	:= pei-i386
	BINFORMAT 	:= i386
endif
ifdef ARCH_X86_64
	OBJFORMAT 	:= pei-x86-64
	BINFORMAT 	:= i386
endif
endif #PLAT_MINGW


##
# intial tool flags
#############################

ifdef PLAT_LINUX
# if someone has a ppc linux machine, please define -DLIL/-DBIG in the code,
# remove them here and use the __ppc__ macro to resolve endianess issues
	DEFINES		:= -DLIN=1 -DIBM=0 -DAPL=0 -DLIL=1 -DBIG=0
	CFLAGS		:=  $(M32_SWITCH) -fpie -fvisibility=hidden -Wno-multichar -pipe
	CXXFLAGS	:=  $(M32_SWITCH) -fpie -fvisibility=hidden -fvisibility-inlines-hidden -Wno-deprecated -Wno-multichar -pipe
	LDFLAGS		:=  $(M32_SWITCH) -static-libgcc -Wl,-O1 -rdynamic
	BARE_LDFLAGS	+= -O1
	STRIPFLAGS	:= -s -x
endif
ifdef PLAT_DARWIN
# -DLIL/-DBIG have to be defined in the code itself to support universal builds
	DEFINES		:= -DLIN=0 -DIBM=0 -DAPL=1
	CXXFLAGS	:= -mmacosx-version-min=10.4 -Wno-multichar -frounding-math
	CFLAGS		:= -mmacosx-version-min=10.4 -Wno-multichar -frounding-math
	LDFLAGS		:= -mmacosx-version-min=10.4 -isysroot /Developer/SDKs/MacOSX10.4u.sdk -static-libgcc
	MACARCHS	:= -arch i386 -arch ppc
	STRIPFLAGS	:= -x
endif
ifdef PLAT_MINGW
	DEFINES		:= -DLIN=0 -DIBM=1 -DAPL=0 -DLIL=1 -DBIG=0
	CFLAGS		:= -Wno-multichar
	CXXFLAGS	:= -Wno-deprecated -Wno-multichar
	LDFLAGS		:= -static-libgcc -Wl,-O1
	BARE_LDFLAGS	+= -O1
	STRIPFLAGS	:= -s -x
endif


##
# configuration specific environment
####################################

ifeq ($(conf), release_opt)
	CFLAGS		+= -O2 -fomit-frame-pointer
	CXXFLAGS	+= -O2 -fomit-frame-pointer
	DEFINES		+= -DDEV=0
	StripDebug	:= Yes
else ifeq ($(conf), release)
	CFLAGS		+= -O0 -g
	CXXFLAGS	+= -O0 -g
	DEFINES		+= -DDEV=0
else ifeq ($(conf), debug)
	CFLAGS		+= -O0 -g
	CXXFLAGS	+=  -O0 -g
	DEFINES		+=  -DDEV=1
else ifeq ($(conf), release_test)
	CFLAGS		+= -O2 -fomit-frame-pointer
	CXXFLAGS	+= -O2 -fomit-frame-pointer
	DEFINES		+= -DDEV=1
	StripDebug	:= Yes
# default to debug_opt configuration
else
	CFLAGS		+= -O2 -g
	CXXFLAGS	+= -O2 -g
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
CC	:= gcc-4.2
CXX	:= g++-4.2
LD	:= g++-4.2
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
MOC	:= moc-qt4
endif


##
# target specific environment
#############################

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
REAL_TARGET := $(BUILDDIR)/$(REAL_TARGET)$(MULTI_SUFFIX)

ifdef PLAT_LINUX

ifdef TYPE_LIBDYNAMIC
REAL_TARGET := $(REAL_TARGET).p
endif
ifdef TYPE_LIBSTATIC
REAL_TARGET := $(REAL_TARGET).a
endif

endif #LINUX

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
ALL_OBJECTS	:= $(sort $(COBJECTS) $(CXXOBJECTS) $(RESOURCEOBJ))
ALL_OBJECTFILES	:= $(notdir $(ALL_OBJECTS))

SRC_DIRS	:= $(sort $(dir $(ALL_OBJECTS)))
BUILTINS	:= $(addsuffix builtin$(BIN_SUFFIX).o.$(TARGET), $(SRC_DIRS))
FINALBUILTIN	:= $(BUILDDIR)/obj/builtin$(BIN_SUFFIX).o.$(TARGET).final


##
# build rules
#####################

all: $(REAL_TARGET)

$(REAL_TARGET): $(ALL_OBJECTS)
	@-mkdir -p $(dir $(@))
	@$(print_link) $(subst $(PWD)/, ./, $(abspath $(@)))
ifdef TYPE_EXECUTABLE
	$(LD) $(MACARCHS) $(LIBPATHS) $(LDFLAGS) -o $(@) \
	$(ALL_OBJECTS) $(LIBS) || $(print_error)
endif
ifdef TYPE_LIBDYNAMIC
	@$(LD) $(MACARCHS) $(LDFLAGS) $(LIBPATHS) -shared \
	-Wl,-export-dynamic,-soname,$(notdir $(@)) -o $(@) $(ALL_OBJECTS) \
	$(LIBS) || $(print_error)
endif
ifdef StripDebug
	$(STRIP) $(STRIPFLAGS) $(@)
endif
	@$(print_finished)

$(BUILDDIR)/obj/%$(BIN_SUFFIX).res.o : %
	@$(print_comp_res) $(subst $(PWD)/, ./, $(abspath $(<)))
	@-mkdir -p $(dir $(@))
	@cd $(dir $(<)) && $(OBJCOPY) -I binary -O $(OBJFORMAT) \
	-B $(BINFORMAT) $(notdir $(<)) $(WD)/$(@) && cd $(WD)

$(BUILDDIR)/obj/%$(BIN_SUFFIX).winres.o : %
	@$(print_comp_res) $(subst $(PWD)/, ./, $(abspath $(<)))
	@-mkdir -p $(dir $(@))
	@$(WINDRES) $< -O coff -o $(@)

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
