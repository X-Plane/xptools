ARCHITECTURE	:= $(shell uname -m)
PLATFORM	:= $(shell uname)

ifneq (, $(findstring MINGW, $(PLATFORM)))
	PLAT_WINNT	:= Yes
endif

ifeq ($(PLATFORM), Linux)
	PLAT_LINUX	:= Yes
endif

ifeq ($(PLATFORM), Darwin)
	PLAT_DARWIN	:= Yes
endif

SAMPLESRC	:= 'int main(){return 0;}'

ifdef PLAT_WINNT
EXECFLAGS	:= -c -o /dev/null - > /dev/null 2>&1
else
EXECFLAGS	:= -o /dev/null - > /dev/null 2>&1
endif

HAVE_MULTILIB := $(shell \
	(echo $(SAMPLESRC) | $(CC) -m32 -xc $(EXECFLAGS)) && \
	(echo $(SAMPLESRC) | $(CXX) -m32 -xc++ $(EXECFLAGS)) && \
	(echo $(SAMPLESRC) | $(CC) -m64 -xc $(EXECFLAGS)) && \
	(echo $(SAMPLESRC) | $(CXX) -m64 -xc++ $(EXECFLAGS)); \
	echo $$?)
ifeq ($(HAVE_MULTILIB), 0)
	MULTILIB	:= Yes
endif
