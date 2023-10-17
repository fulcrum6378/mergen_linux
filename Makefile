main.o : main.cpp
	g++ -o build/makefile/main.o main.cpp

microphone.o : aud/microphone.cpp
	g++ -o build/makefile/aud/microphone.o aud/microphone.cpp

camera.o : vis/camera.cpp
	g++ -o build/makefile/vis/camera.o vis/camera.cpp
