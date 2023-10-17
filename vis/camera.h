#ifndef VIS_CAMERA_H
#define VIS_CAMERA_H

#include <linux/videodev2.h>

#include "segmentation.h"

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
    v4l2_buffer buffer_info{}; // FIXME WTF
    unsigned char *buf;

    Segmentation *segmentation;

    void Capture();

public:
    int exit = 0;

    Camera();

    ~Camera();
};

#endif //VIS_CAMERA_H
