#ifndef VIS_CAMERA_H
#define VIS_CAMERA_H

#include <future>
#include <linux/videodev2.h>
#include <thread>

// image processing method: 1 => Segmentation, 2 => EdgeDetection.
#define VIS_METHOD 1

#if VIS_METHOD == 1

#include "segmentation.hpp"

#elif VIS_METHOD == 2

#include "edge_detection.hpp"

#endif

class Camera {
public:
    explicit Camera(int *exit);

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
    v4l2_format imageFormat{
            V4L2_BUF_TYPE_VIDEO_CAPTURE, {W, H, V4L2_PIX_FMT_YUYV, V4L2_FIELD_NONE}
    };
    v4l2_buffer buf_info{
            .index = 0u,
            .type = V4L2_BUF_TYPE_VIDEO_CAPTURE,
            .field = V4L2_FIELD_NONE,
            .memory = V4L2_MEMORY_MMAP
    };
    unsigned char *buf;

    std::thread record;
    std::promise<void> recPromise;
    std::future<void> recFuture;
#if VIS_METHOD == 1
    Segmentation *segmentation;
#elif VIS_METHOD == 2
    EdgeDetection *edgeDetection;
#endif
};

#endif //VIS_CAMERA_H
