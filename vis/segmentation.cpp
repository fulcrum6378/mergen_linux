#include <asm-generic/types.h>
#include <cstring>
#include <fstream>
#include <iostream>

#include "bitmap.h"
#include "segmentation.h"

using namespace std;

Segmentation::Segmentation(unsigned char **buf) : buf_(buf) {}

void Segmentation::Process(__u32 bytesUsed) {
    /*ofstream outFile("../../test.jpg", ios::binary);
    __u32 bufPos = 0, outFileMemBlockSize = 0, remainingBufferSize = bytesUsed;
    char *outFileMemBlock;
    while (remainingBufferSize > 0) {
        bufPos += outFileMemBlockSize;
        outFileMemBlockSize = 1024;
        outFileMemBlock = new char[sizeof(char) * outFileMemBlockSize];
        memcpy(outFileMemBlock, *buf_ + bufPos, outFileMemBlockSize);
        outFile.write(outFileMemBlock, outFileMemBlockSize);
        if (outFileMemBlockSize > remainingBufferSize)
            outFileMemBlockSize = remainingBufferSize;
        remainingBufferSize -= outFileMemBlockSize;
        delete outFileMemBlock;
    }
    outFile.close();*/

    int16_t width = 640, height = 480;
    char arr[480][640][3];

    for (int i = 0, j = 0; i < sizeof(arr) && j < bytesUsed; i += 6, j += 4) { // don't call `sizeof(*buf_)`
        double y, v, u, r, g, b;

        y = static_cast<double>((*buf_)[j + 0]); //y0
        u = static_cast<double>((*buf_)[j + 1]); //u0
        v = static_cast<double>((*buf_)[j + 3]); //v0

        r = y + 1.4065 * (v - 128);                 //r0
        g = y - 0.3455 * (u - 128) - 0.7169 * (v - 128); //g0
        b = y + 1.1790 * (u - 128);                 //b0

        if (r < 0) r = 0; else if (r > 255) r = 255;
        if (g < 0) g = 0; else if (g > 255) g = 255;
        if (b < 0) b = 0; else if (b > 255) b = 255;

        arr[j / width][j % width][0] = (char) r;
        arr[j / width][j % width][1] = (char) g;
        arr[j / width][j % width][2] = (char) b;

        std::cout << (i / 3) / height << " x " << i % height << std::endl;

        //second pixel
        u = static_cast<double>((*buf_)[j + 1]); //u0
        y = static_cast<double>((*buf_)[j + 2]); //y1
        v = static_cast<double>((*buf_)[j + 3]); //v0

        r = y + 1.4065 * (v - 128);                 //r1
        g = y - 0.3455 * (u - 128) - 0.7169 * (v - 128); //g1
        b = y + 1.1790 * (u - 128);                 //b1

        if (r < 0) r = 0; else if (r > 255) r = 255;
        if (g < 0) g = 0; else if (g > 255) g = 255;
        if (b < 0) b = 0; else if (b > 255) b = 255;

        arr[j / width][(j % width) + 1][0] = (char) r;
        arr[j / width][(j % width) + 1][1] = (char) g;
        arr[j / width][(j % width) + 1][2] = (char) b;
    }

    bitmap(buf_, arr);
}

Segmentation::~Segmentation() = default;
