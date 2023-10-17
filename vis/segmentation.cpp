#include <asm-generic/types.h>
#include <cstring>
#include <fstream>

#include "bitmap.h"
#include "segmentation.h"

using namespace std;

Segmentation::Segmentation(char **buf) : buf_(buf) {}

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

    bitmap(buf_);
}

Segmentation::~Segmentation() = default;
