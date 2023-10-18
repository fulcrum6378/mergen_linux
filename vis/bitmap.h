#ifndef VIS_BITMAP_H
#define VIS_BITMAP_H

#include <fstream>

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

void bitmap(unsigned char arr[480][640][3], const std::string& path) {
    std::ofstream bmp(path, std::ios::binary);
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

    for (auto yy = (int16_t) (height - 1); yy >= 0; yy--) {
        for (int16_t xx = 0; xx < width; xx++) {
            double y = static_cast<double>(arr[yy][xx][0]),
                    u = static_cast<double>(arr[yy][xx][1]),
                    v = static_cast<double>(arr[yy][xx][2]),
                    r, g, b;

            r = y + 1.4065 * (v - 128);                 //r0
            g = y - 0.3455 * (u - 128) - 0.7169 * (v - 128); //g0
            b = y + 1.1790 * (u - 128);                 //b0

            if (r < 0) r = 0; else if (r > 255) r = 255;
            if (g < 0) g = 0; else if (g > 255) g = 255;
            if (b < 0) b = 0; else if (b > 255) b = 255;

            bmp.put((char) b);
            bmp.put((char) g);
            bmp.put((char) r);
        }
        for (int i = 0; i < width % 4; i++) bmp.put(0);
    }
    bmp.close();
}

#endif //VIS_BITMAP_H
