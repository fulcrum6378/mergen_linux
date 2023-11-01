# FIXME doesn't work in FreeBSD!
UNAME := $(shell uname)
ifeq ($(UNAME), Linux)
compiler := g++
endif
ifeq ($(UNAME), FreeBSD)
compiler := c++
endif

# FIXME doesn't work in Linux!
UNAME != uname
.if $(UNAME) == Linux
compiler = g++
.elif $(UNAME) == FreeBSD
compiler = c++
.endif

MergenLinux:
	$(compiler) -std=c++20 -lpthread -o build/MergenLinux main.cpp \
	aud/microphone.cpp \
	hpt/touchpad.cpp \
	vis/camera.cpp vis/memory.cpp vis/segmentation.cpp vis/visual_stm.cpp
	#./build/MergenLinux
