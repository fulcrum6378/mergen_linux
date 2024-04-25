#ifndef VIS_BITMAP_H
#define VIS_BITMAP_H

#include <filesystem>
#include <fstream>
#include <sys/stat.h>

#include "global.hpp"

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
    int32_t h_res;
    int32_t v_res;
    uint32_t num_colors;
    uint32_t num_important_colors;
};

#pragma clang diagnostic pop

static const std::string dirBitmap("vis/bmp/");
static uint32_t frameId = 1u;

class Bitmap {
public:
    [[maybe_unused]] static void createDir() {
        struct stat sb{};
        if (stat(dirBitmap.c_str(), &sb) != 0)
            mkdir(dirBitmap.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
        else
            for (const auto &entry: std::filesystem::directory_iterator(dirBitmap))
                std::filesystem::remove_all(entry.path());
    }

    [[maybe_unused]] static void save(unsigned char arr[H][W][3]) {
        std::ofstream bmp(dirBitmap + std::to_string(frameId) + ".bmp", std::ios::binary);
        frameId++;

        bmpfile_magic magic{'B', 'M'};
        bmp.write((char *) (&magic), sizeof(magic));
        bmpfile_header header = {0};
        header.bmp_offset =
                sizeof(bmpfile_magic) + sizeof(bmpfile_header) + sizeof(bmpfile_dib_info);
        header.file_size = header.bmp_offset + (H * 3 + W % 4) * H;
        bmp.write((char *) (&header), sizeof(header));
        bmpfile_dib_info dib_info = {0};
        dib_info.header_size = sizeof(bmpfile_dib_info);
        dib_info.width = W;
        dib_info.height = H;
        dib_info.num_planes = 1u;
        dib_info.bits_per_pixel = 24u;
        dib_info.compression = 0u;
        dib_info.bmp_byte_size = 0u;
        dib_info.h_res = 2835;
        dib_info.v_res = 2835;
        dib_info.num_colors = 0u;
        dib_info.num_important_colors = 0u;
        bmp.write((char *) &dib_info, sizeof(dib_info));

        for (auto yy = (int16_t) (H - 1); yy >= 0; yy--) {
            for (int16_t xx = 0; xx < W; xx++) {
                double y = static_cast<double>(arr[yy][xx][0]),
                        u = static_cast<double>(arr[yy][xx][1]),
                        v = static_cast<double>(arr[yy][xx][2]),
                        r, g, b;

                r = y + 1.4065 * (v - 128.0);                        //r0
                g = y - 0.3455 * (u - 128.0) - 0.7169 * (v - 128.0); //g0
                b = y + 1.1790 * (u - 128.0);                        //b0

                if (r < 0.0) r = 0.0; else if (r > 255.0) r = 255.0;
                if (g < 0.0) g = 0.0; else if (g > 255.0) g = 255.0;
                if (b < 0.0) b = 0.0; else if (b > 255.0) b = 255.0;

                bmp.put(static_cast<char>(b));
                bmp.put(static_cast<char>(g));
                bmp.put(static_cast<char>(r));
            }
            for (int i = 0; i < (W % 4); i++) bmp.put(0); // both 'ide diagnostic' items are only for this line!
        }
        bmp.close();
    }
};

#endif //VIS_BITMAP_H
