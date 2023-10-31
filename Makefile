MergenLinux:
	g++ -std=c++20 -o build/MergenLinux main.cpp \
	aud/microphone.cpp \
	hpt/touchpad.cpp \
	vis/camera.cpp vis/memory.cpp vis/segmentation.cpp vis/visual_stm.cpp
	#./build/MergenLinux
