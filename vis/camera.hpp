#ifndef VIS_CAMERA_H
#define VIS_CAMERA_H

#include <thread>

#include "segmentation.hpp"

#ifdef __linux__

#include <linux/videodev2.h>

#else

#include "videodev2.hpp"

#endif

class Camera {
public:
    explicit Camera(int *exit);

    void Stop();

    ~Camera();

private:
    void Record();


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
    v4l2_buffer buffer_info{};
    unsigned char *buf;

    std::thread *record;
    Segmentation *segmentation;

};

#endif //VIS_CAMERA_H
