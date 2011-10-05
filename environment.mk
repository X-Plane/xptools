ARCHITECTURE	:= $(shell uname -m)
PLATFORM	:= $(shell uname)

SAMPLESRC	:= 'int main(){return 0;}'
EXECFLAGS	:= -o /dev/null - > /dev/null 2>&1
HAVE_MULTILIB := $(shell \
	(echo $(SAMPLESRC) | $(CC) -m32 -xc $(EXECFLAGS)) && \
	(echo $(SAMPLESRC) | $(CXX) -m32 -xc++ $(EXECFLAGS)) && \
	(echo $(SAMPLESRC) | $(CC) -m64 -xc $(EXECFLAGS)) && \
	(echo $(SAMPLESRC) | $(CXX) -m64 -xc++ $(EXECFLAGS)); \
	echo $$?)
ifeq ($(HAVE_MULTILIB), 0)
	MULTILIB	:= Yes
endif

ifneq (, $(findstring MINGW, $(PLATFORM)))
	PLAT_WINNT	:= Yes
endif

ifeq ($(PLATFORM), Linux)
	PLAT_LINUX	:= Yes
endif

ifeq ($(PLATFORM), Darwin)
	PLAT_DARWIN	:= Yes
endif
