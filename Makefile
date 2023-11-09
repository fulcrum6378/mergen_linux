MergenLinux:
	# TODO mkdir build if not exists...
	c++ -std=c++20 -lpthread -o build/MergenLinux main.cpp \
	aud/microphone.cpp aud/speaker.cpp \
	hpt/touchpad.cpp \
	vis/camera.cpp vis/memory.cpp vis/segmentation.cpp vis/visual_stm.cpp
	# build/MergenLinux
