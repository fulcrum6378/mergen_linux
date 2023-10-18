cmake_minimum_required(VERSION 3.26)
project(MergenLinux)

set(CMAKE_CXX_STANDARD 20)

add_executable(MergenLinux
        main.cpp

        aud/microphone.cpp
        aud/microphone.h

        hpt/touchpad.cpp
        hpt/touchpad.h

        vis/bitmap.h
        vis/camera.cpp
        vis/camera.h
        vis/segmentation.cpp
        vis/segmentation.h
        global.h
)
