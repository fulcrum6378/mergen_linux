.if !exists(./build)
MAKE_BUILD_DIR = mkdir build
.endif

MergenLinux:
	${MAKE_BUILD_DIR}
	c++ -std=c++20 -lpthread -o build/MergenLinux main.cpp \
	aud/microphone.cpp aud/speaker.cpp \
	hpt/touchpad.cpp \
	vis/camera.cpp vis/memory.cpp vis/segmentation.cpp vis/visual_stm.cpp
	echo "Now run: $ build/MergenLinux"
