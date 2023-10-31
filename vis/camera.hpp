#ifndef VIS_CAMERA_H
#define VIS_CAMERA_H

#include <linux/videodev2.h> // structs and command constants sent to device
#include <thread>

#include "segmentation.hpp"

class Camera {
private:
    int dev;
    /**
     * Install V4L utilities for gathering information about your system.
     * $ sudo apt install v4l-utils
     *
     * List available formats: $ v4l2-ctl --list-format
     * My laptop (Dell XPS L502X):
     *   [0]: 'YUYV' (YUYV 4:2:2)
	 *   [1]: 'MJPG' (Motion-JPEG, compressed)
     * Display all data: $ v4l2-ctl --list-devices
     */
    v4l2_format imageFormat{};
    v4l2_requestbuffers requestBuffer{0};
    v4l2_buffer queryBuffer{0};
    v4l2_buffer buffer_info{};
    unsigned char *buf;

    std::atomic_bool *on_;
    std::thread record;
    Segmentation *segmentation;

    void Record();

public:
    int exit = 0;

    explicit Camera(std::atomic_bool *on);

    ~Camera();
};

#endif //VIS_CAMERA_H
