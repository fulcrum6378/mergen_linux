#ifndef VIS_BITMAP_H
#define VIS_BITMAP_H

#include <fstream>
#include <thread>

#define MAX(a, b) ({__typeof__(a) _a = (a); __typeof__(b) _b = (b); _a > _b ? _a : _b; })
#define MIN(a, b) ({__typeof__(a) _a = (a); __typeof__(b) _b = (b); _a < _b ? _a : _b; })

#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"

// related to Windows
struct bmpfile_magic {
    unsigned char magic[2];
};

struct bmpfile_header {
    uint32_t file_size;
    uint16_t creator1;
    uint16_t creator2;
    uint32_t bmp_offset;
};

struct bmpfile_dib_info {
    uint32_t header_size;
    int32_t width;
    int32_t height;
    uint16_t num_planes;
    uint16_t bits_per_pixel;
    uint32_t compression;
    uint32_t bmp_byte_size;
    int32_t hres;
    int32_t vres;
    uint32_t num_colors;
    uint32_t num_important_colors;
};

#pragma clang diagnostic pop

const int kMaxChannelValue = 262143;


/**
 * Yuv2Rgb algorithm is from:
 * https://github.com/tensorflow/tensorflow/blob/5dcfc51118817f27fad5246812d83e5dccdc5f72/
 * tensorflow/tools/android/test/jni/yuv2rgb.cc
 */
void bitmap(char **buf) {
    std::ofstream bmp("../../out.bmp", std::ios::binary);
    int16_t width = 640, height = 480;

    bmpfile_magic magic{'B', 'M'};
    bmp.write((char *) (&magic), sizeof(magic));
    bmpfile_header header = {0};
    header.bmp_offset =
            sizeof(bmpfile_magic) + sizeof(bmpfile_header) + sizeof(bmpfile_dib_info);
    header.file_size = header.bmp_offset + (height * 3 + width % 4) * height;
    bmp.write((char *) (&header), sizeof(header));
    bmpfile_dib_info dib_info = {0};
    dib_info.header_size = sizeof(bmpfile_dib_info);
    dib_info.width = width;
    dib_info.height = height;
    dib_info.num_planes = 1;
    dib_info.bits_per_pixel = 24;
    dib_info.compression = 0;
    dib_info.bmp_byte_size = 0;
    dib_info.hres = 2835;
    dib_info.vres = 2835;
    dib_info.num_colors = 0;
    dib_info.num_important_colors = 0;
    bmp.write((char *) &dib_info, sizeof(dib_info));

    /*AImageCropRect srcRect;
    AImage_getCropRect(image, &srcRect);
    int32_t yStride, uvStride;
    uint8_t *yPixel, *uPixel, *vPixel;
    int32_t yLen, uLen, vLen;
    AImage_getPlaneRowStride(image, 0, &yStride);
    AImage_getPlaneRowStride(image, 1, &uvStride);
    AImage_getPlaneData(image, 0, &yPixel, &yLen);
    AImage_getPlaneData(image, 1, &vPixel, &vLen);
    AImage_getPlaneData(image, 2, &uPixel, &uLen); // IT'S "YVU" NOT YUV!!!
    int32_t uvPixelStride;
    AImage_getPlanePixelStride(image, 1, &uvPixelStride);*/

    for (auto y = (int16_t) (height - 1); y >= 0; y--) {
        /*const uint8_t *pY = yPixel + yStride * (y + srcRect.top) + srcRect.left;

        int32_t uv_row_start = uvStride * ((y + srcRect.top) >> 1);
        const uint8_t *pU = uPixel + uv_row_start + (srcRect.left >> 1);
        const uint8_t *pV = vPixel + uv_row_start + (srcRect.left >> 1);*/

        for (int16_t x = 0; x < width; x++) {
            /*const int32_t uv_offset = (x >> 1) * uvPixelStride;

            int nY = pY[x] - 16;
            int nU = pU[uv_offset] - 128;
            int nV = pV[uv_offset] - 128;
            if (nY < 0) nY = 0;*/
            int i = (y * width) + x;
            int nY = ((*buf)[i] & 0xFF);
            int nU = ((*buf)[(i / 2) * 2] & 0xFF00) >> 8;
            int nV = ((*buf)[(i / 2) * 2 + 1] & 0xFF00) >> 8;

            int nR = (int) (1192 * nY + 1634 * nV);
            int nG = (int) (1192 * nY - 833 * nV - 400 * nU);
            int nB = (int) (1192 * nY + 2066 * nU);

            int maxR = MAX(0, nR), maxG = MAX(0, nG), maxB = MAX(0, nB);
            nR = MIN(kMaxChannelValue, maxR);
            nG = MIN(kMaxChannelValue, maxG);
            nB = MIN(kMaxChannelValue, maxB);

            nR = (nR >> 10) & 0xff;
            nG = (nG >> 10) & 0xff;
            nB = (nB >> 10) & 0xff;

            bmp.put((char) nR);
            bmp.put((char) nG);
            bmp.put((char) nB);
        }
        for (int i = 0; i < width % 4; i++) bmp.put(0);
    }
    bmp.close();
}

#endif //VIS_BITMAP_H
