#include <cstring>
#include <fcntl.h> // O_RDWR (oflag)
#include <sys/ioctl.h>
#include <sys/mman.h> // PROT_READ, PROT_WRITE, MAP_SHARED
#include <unistd.h>

#include "../global.hpp"
#include "camera.hpp"

using namespace std;

Camera::Camera(int *exit) {
    dev = open("/dev/video0", O_RDWR);
    if (dev < 0) {
        perror("Failed to open device");
        *exit = 1;
        return;
    }
    v4l2_capability capability{};
    if (ioctl(dev, VIDIOC_QUERYCAP, &capability) < 0) {
        perror("This device cannot capture frames");
        *exit = 2;
        return;
    }

    // v4l2_format
    imageFormat.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    imageFormat.fmt.pix.width = 640u;
    imageFormat.fmt.pix.height = 480u;
    imageFormat.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV; // V4L2_PIX_FMT_MJPEG -> V4L2_PIX_FMT_YUYV
    imageFormat.fmt.pix.field = V4L2_FIELD_NONE;
    ioctl(dev, VIDIOC_S_FMT, &imageFormat);

    v4l2_requestbuffers requestBuffer{};
    requestBuffer.count = 1; // one request buffer
    requestBuffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    requestBuffer.memory = V4L2_MEMORY_MMAP;
    ioctl(dev, VIDIOC_REQBUFS, &requestBuffer);

    v4l2_buffer queryBuffer{};
    queryBuffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    queryBuffer.memory = V4L2_MEMORY_MMAP;
    queryBuffer.index = 0u;
    ioctl(dev, VIDIOC_QUERYBUF, &queryBuffer);

    buf = (unsigned char *) mmap(
            nullptr, queryBuffer.length, PROT_READ | PROT_WRITE, MAP_SHARED,
            dev, queryBuffer.m.offset);
    memset(buf, 0, queryBuffer.length);

    // v4l2_buffer (must necessarily be separate from `queryBuffer`)
    memset(&buffer_info, 0, sizeof(buffer_info));
    buffer_info.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buffer_info.memory = V4L2_MEMORY_MMAP;
    buffer_info.index = 0u;
    ioctl(dev, VIDIOC_STREAMON, &buffer_info.type);

    // prepare for analysis
    segmentation = new Segmentation(&buf);
    recFuture = recPromise.get_future();
    record = std::thread(&Camera::Record, this);
    record.detach();
}

void Camera::Record() {
    while (on) {
        ioctl(dev, VIDIOC_QBUF, &buffer_info);
        ioctl(dev, VIDIOC_DQBUF, &buffer_info);

        segmentation->bufLength = buffer_info.bytesused;
        segmentation->Process();
    }
#if VISUAL_STM
    // if recording is over, save state of VisualSTM
    segmentation->stm->SaveState();
#endif
    recPromise.set_value();
}

Camera::~Camera() {
    recFuture.wait();
    delete segmentation;
    ioctl(dev, VIDIOC_STREAMOFF, &buffer_info.bytesused);
    close(dev);
}
