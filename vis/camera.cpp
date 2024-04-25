#include <cstring>
#include <cerrno> // ioctl
#include <fcntl.h> // O_RDWR (oflag)
#include <iostream> // perror
#include <sys/ioctl.h>
#include <sys/mman.h> // PROT_READ, PROT_WRITE, MAP_SHARED
#include <unistd.h> // close

#include "../global.hpp"
#include "camera.hpp"

using namespace std;

Camera::Camera(int *exit) {
    // open first camera device and check its capabilities
    if ((dev = open("/dev/video0", O_RDWR)) == -1) {
        perror("Failed to open camera");
        *exit = 1;
        return;
    }
    v4l2_capability capability{};
    if (ioctl(dev, VIDIOC_QUERYCAP, &capability) == -1) {
        perror("This camera cannot capture frames");
        *exit = 2;
        return;
    }

    // set image format for camera
    if (ioctl(dev, VIDIOC_S_FMT, &imageFormat) == -1) {
        print("Camera could not set format: %d", errno);
        *exit = 3;
        return;
    }

    // allocate a buffer
    v4l2_requestbuffers reqBuf{1u, buf_info.type, buf_info.memory};
    if (ioctl(dev, VIDIOC_REQBUFS, &reqBuf) == -1) {
        print("Couldn't request buffer from camera: %d", errno);
        *exit = 4;
        return;
    }

    // retrieve data on the allocated buffer, then map and clean the buffer
    if (ioctl(dev, VIDIOC_QUERYBUF, &buf_info) == -1) {
        print("Camera didn't return the buffer information: %d", errno);
        *exit = 5;
        return;
    }
    buf = (unsigned char *) mmap(
            nullptr, buf_info.length, PROT_READ | PROT_WRITE, MAP_SHARED,
            dev, buf_info.m.offset);
    memset(buf, 0, buf_info.length);

    // start streaming
    if (ioctl(dev, VIDIOC_STREAMON, &buf_info.type) == -1) {
        print("Camera couldn't start streaming: %d", errno);
        *exit = 6;
        return;
    }

    // prepare for analysis and start recording
#if VIS_METHOD == 1
    segmentation = new Segmentation(&buf);
#elif VIS_METHOD == 2
    edgeDetection = new EdgeDetection(&buf);
#endif
    recFuture = recPromise.get_future();
    record = std::thread(&Camera::Record, this);
    record.detach();
}

void Camera::Record() {
    while (on) {
        ioctl(dev, VIDIOC_QBUF, &buf_info);
        ioctl(dev, VIDIOC_DQBUF, &buf_info);

#if VIS_METHOD == 1
        segmentation->bufLength = buf_info.bytesused;
        segmentation->Process();
#elif VIS_METHOD == 2
        edgeDetection->bufLength = buf_info.bytesused;
        edgeDetection->Process();
#endif
    }
#if VIS_METHOD == 1 && VISUAL_STM
    // if recording is over, save state of VisualSTM
    segmentation->stm->SaveState();
#endif
    recPromise.set_value();
}

Camera::~Camera() {
    recFuture.wait();
#if VIS_METHOD == 1
    delete segmentation;
#elif VIS_METHOD == 2
    delete edgeDetection;
#endif
    ioctl(dev, VIDIOC_STREAMOFF, &buf_info.bytesused);
    close(dev);
}
