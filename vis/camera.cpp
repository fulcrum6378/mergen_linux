#include <csignal>
#include <cstring>
#include <fcntl.h>
#include <fstream>
#include <string>
#include <sys/ioctl.h>
#include <sys/mman.h>

#include "camera.h"

using namespace std;

Camera::Camera() {
    dev = open("/dev/video0", O_RDWR);
    if (dev < 0) {
        perror("Failed to open device!");
        return;
    }
    v4l2_capability capability{};
    if (ioctl(dev, VIDIOC_QUERYCAP, &capability) < 0) {
        perror("The device cannot capture frame");
        return;
    }

    // v4l2_format
    imageFormat.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    imageFormat.fmt.pix.width = 640;
    imageFormat.fmt.pix.height = 480;
    imageFormat.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV; // V4L2_PIX_FMT_MJPEG -> V4L2_PIX_FMT_YUYV
    imageFormat.fmt.pix.field = V4L2_FIELD_NONE;
    ioctl(dev, VIDIOC_S_FMT, &imageFormat);

    // v4l2_requestbuffers
    requestBuffer.count = 1; // one request buffer
    requestBuffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    requestBuffer.memory = V4L2_MEMORY_MMAP;
    ioctl(dev, VIDIOC_REQBUFS, &requestBuffer);

    queryBuffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    queryBuffer.memory = V4L2_MEMORY_MMAP;
    queryBuffer.index = 0;
    ioctl(dev, VIDIOC_QUERYBUF, &queryBuffer);

    arr = (char *) mmap(
            nullptr, queryBuffer.length, PROT_READ | PROT_WRITE, MAP_SHARED,
            dev, queryBuffer.m.offset);
    memset(arr, 0, queryBuffer.length);

    memset(&buffer_info, 0, sizeof(buffer_info));
    buffer_info.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buffer_info.memory = V4L2_MEMORY_MMAP;
    buffer_info.index = 0;
    ioctl(dev, VIDIOC_STREAMON, &buffer_info.type);
}

void Camera::Capture() {
    ioctl(dev, VIDIOC_QBUF, &buffer_info);
    ioctl(dev, VIDIOC_DQBUF, &buffer_info);

    ofstream outFile;
    outFile.open("../../test.yuv", ios::binary | ios::app);
    __u32 bufPos = 0, outFileMemBlockSize = 0, remainingBufferSize = buffer_info.bytesused;
    char *outFileMemBlock;
    while (remainingBufferSize > 0) {
        bufPos += outFileMemBlockSize;
        outFileMemBlockSize = 1024;
        outFileMemBlock = new char[sizeof(char) * outFileMemBlockSize];
        memcpy(outFileMemBlock, arr + bufPos, outFileMemBlockSize);
        outFile.write(outFileMemBlock, outFileMemBlockSize);
        if (outFileMemBlockSize > remainingBufferSize)
            outFileMemBlockSize = remainingBufferSize;
        remainingBufferSize -= outFileMemBlockSize;

        delete outFileMemBlock;
    }
    outFile.close();
}

Camera::~Camera() {
    ioctl(dev, VIDIOC_STREAMOFF, &buffer_info.type);
    close(dev);
}
