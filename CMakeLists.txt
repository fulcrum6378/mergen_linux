cmake_minimum_required(VERSION 3.26)
project(MergenLinux)

set(CMAKE_CXX_STANDARD 20)

add_executable(MergenLinux
        main.cpp
        global.hpp

        aud/microphone.cpp
        aud/microphone.hpp

        hpt/touchpad.cpp
        hpt/touchpad.hpp

        vis/bitmap.hpp
        vis/camera.cpp
        vis/camera.hpp
        vis/memory.cpp
        vis/memory.hpp
        vis/segment.hpp
        vis/segmentation.cpp
        vis/segmentation.hpp
        vis/visual_stm.cpp
        vis/visual_stm.hpp
)
