Mergen:
	if [ ! -d "build" ]; then mkdir "build"; fi
	g++ -std=c++20 -lpthread -Wpedantic -o build/Mergen main.cpp \
	aud/microphone.cpp aud/speaker.cpp \
	hpt/touchpad.cpp \
	vis/camera.cpp vis/edge_detection.cpp vis/memory.cpp vis/segmentation.cpp vis/visual_stm.cpp
	# build/Mergen
