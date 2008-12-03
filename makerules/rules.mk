##
# global environment
####################

.PHONY: all

PLATFORM		:= $(shell uname)
ARCHITECTURE	:= $(shell uname -m)
WD				:= $(PWD)

ifeq ($(PLATFORM), Linux)
	ECHOFLAGS	:= -e
else ifeq ($(PLATFORM), Darwin)
	ECHOFLAGS	:=
endif

clearscreen		:= echo $(ECHOFLAGS) "\033[2J\033[H"
print_clean		:= echo $(ECHOFLAGS) "\033[0;31m[ deleting all generated files  ]\033[0m"
print_link		:= echo $(ECHOFLAGS) "\033[0;34m[ linking executable      ]\033[0m: "
print_comp_cc	:= echo $(ECHOFLAGS) "\033[0;32m[ compiling c object      ]\033[0m: "
print_comp_cxx	:= echo $(ECHOFLAGS) "\033[0;32m[ compiling cpp object    ]\033[0m: "
print_comp_res	:= echo $(ECHOFLAGS) "\033[0;32m[ generating resource     ]\033[0m: "
print_arch		:= echo $(ECHOFLAGS) "\033[0;34m[ creating static library ]\033[0m: "
print_so		:= echo $(ECHOFLAGS) "\033[0;34m[ creating shared library ]\033[0m: "
print_dep		:= echo $(ECHOFLAGS) "\033[0;34m[ calculating dependency  ]\033[0m: "
print_finished	:= echo $(ECHOFLAGS) "\033[0;32m finished \o/\033[0m"
print_error		:= (echo $(ECHOFLAGS) "\033[0;31m[ --FAILED-- ] :-(\033[0m" && false)


##
# target specific environment
#############################

include makerules/$(shell basename $(TARGET))

##
# architecture specific environment
###################################

ifeq ($(ARCHITECTURE), i386)
	OBJFORMAT = elf32-i386
	BINFORMAT = i386
else ifeq ($(ARCHITECTURE), i686)
	OBJFORMAT = elf32-i386
	BINFORMAT = i386
else ifeq ($(ARCHITECTURE), x86_64)
	OBJFORMAT = elf64-x86-64
	BINFORMAT = i386:x86-64
endif

##
# platform specific environment
###############################

ifeq ($(PLATFORM), Linux)
	DEFINES		:= $(DEFINES) -DLIN=1 -DIBM=0 -DAPL=0 -DLIL=1 -DBIG=0
else ifeq ($(PLATFORM), Darwin)
	DEFINES		:= $(DEFINES) -DLIN=0 -DIBM=0 -DAPL=1 -DLIL=1 -DBIG=0
endif

##
# configuration specific environment
####################################

# Ben, please check if this is the setup you want. debug information
# is detached anyway, so a '-g' doesn't make a difference in release builds
# (especially in beta versions debuginfo should be shipped with the binary,
# so that the user can submit meaningful stacktrace information)
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
# putting environments together
###############################

CC			:= gcc
CXX			:= g++
LD			:= g++
AR			:= ar
CFLAGS		:= $(CFLAGS)
CXXFLAGS	:= $(CXXFLAGS)
LDFLAGS		:= $(LDFLAGS)
ARFLAGS		:= $(ARFLAGS)
DEFINES		:= $(DEFINES)
CXXSOURCES	:= $(filter %.cpp, $(SOURCES))
CCSOURCES	:= $(filter %.c, $(SOURCES))
CCDEPS		:= $(patsubst %.c, $(BUILDDIR)/%.dep, $(CCSOURCES))
CCOBJECTS	:= $(patsubst %.c, $(BUILDDIR)/%.o, $(CCSOURCES))
CXXDEPS		:= $(patsubst %.cpp, $(BUILDDIR)/%.dep, $(CXXSOURCES))
CXXOBJECTS	:= $(patsubst %.cpp, $(BUILDDIR)/%.o, $(CXXSOURCES))
RESOURCEOBJ	:= $(patsubst %, $(BUILDDIR)/%.ro, $(RESOURCES))



##
# the default targets
#####################

all: $(TARGET)

-include $(CCDEPS) $(CXXDEPS)

$(TARGET):  $(CCOBJECTS) $(CXXOBJECTS) $(CCDEPS) $(CXXDEPS) $(RESOURCEOBJ)
	-mkdir -p $(dir $(@))
ifeq ($(TYPE), LIBSTATIC)
	$(print_arch) $@
	$(AR) $(ARFLAGS) $@ $(CCOBJECTS) $(CXXOBJECTS) $(RESOURCEOBJ) || $(print_error)
else ifeq ($(TYPE), LIBDYNAMIC)
	$(print_so) $@
	$(LD) $(LDFLAGS) $(LIBPATHS) -shared -Wl,-soname,$(notdir $(@)) -o $@ $(CCOBJECTS) $(CXXOBJECTS) $(RESOURCEOBJ) $(LIBS) $(STDLIBS) || $(print_error)
else ifeq ($(TYPE), EXECUTABLE)
	$(print_link) $@
	$(LD) $(LDFLAGS) $(LIBPATHS) -o $@ $(CCOBJECTS) $(CXXOBJECTS) $(RESOURCEOBJ) $(LIBS) $(STDLIBS) || $(print_error)
else
	echo "no target type specified"
	exit 1
endif
ifneq ($(TYPE), LIBSTATIC)
ifeq ($(PLATFORM), Linux)
ifneq ($(conf), debug)
	objcopy --only-keep-debug $@ $(@).debug
	strip -s -x $@
	cd  $(dir $(@)) && objcopy --add-gnu-debuglink=$(notdir $(@)).debug $(notdir $(@)) && cd $(WD)
	chmod 0644 $(@).debug
endif
endif
endif
	$(print_finished)

$(RESOURCEOBJ): $(BUILDDIR)/%.ro : %
	$(print_comp_res) $<
	-mkdir -p $(dir $(@))
	cd  $(dir $(<)) && objcopy -I binary -O $(OBJFORMAT) -B $(BINFORMAT) $(notdir $(<)) $(WD)/$(@) && cd $(WD)

$(CCOBJECTS): $(BUILDDIR)/%.o: %.c $(BUILDDIR)/%.dep
	$(print_comp_cc) $<
	-mkdir -p $(dir $(@))
	$(CC) $(CFLAGS) $(DEFINES) $(INCLUDEPATHS) -c $< -o $@ || $(print_error)

$(CXXOBJECTS): $(BUILDDIR)/%.o: %.cpp $(BUILDDIR)/%.dep
	$(print_comp_cxx) $<
	-mkdir -p $(dir $(@))
	$(CXX) $(CXXFLAGS) $(DEFINES) $(INCLUDEPATHS) -c $< -o $@ || $(print_error)

$(CCDEPS): $(BUILDDIR)/%.dep: %.c
	$(print_dep) $<
	-mkdir -p $(dir $(@))
	$(CC) $(CFLAGS) $(DEFINES) $(INCLUDEPATHS) -MM $< -MT $@ -MT $(<:.c=.o) -o $@ || $(print_error)

$(CXXDEPS): $(BUILDDIR)/%.dep: %.cpp
	$(print_dep) $<
	-mkdir -p $(dir $(@))
	$(CXX) $(CXXFLAGS) $(DEFINES) $(INCLUDEPATHS) -MM $< -MT $@ -MT $(<:.cpp=.o) -o $@ || $(print_error)
