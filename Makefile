main.o : main.cpp
	g++ -o build/makefile/main.o main.cpp

# AUD
microphone.o : aud/microphone.cpp
	g++ -o build/makefile/aud/microphone.o aud/microphone.cpp

# HPT
touchpad.o : hpt/touchpad.cpp
	g++ -o build/makefile/hpt/touchpad.o hpt/touchpad.cpp

# VIS
camera.o : vis/camera.cpp
	g++ -o build/makefile/vis/camera.o vis/camera.cpp
segmentation.o : vis/segmentation.cpp
	g++ -o build/makefile/vis/segmentation.o vis/segmentation.cpp
