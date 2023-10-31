#ifndef VIS_SEGMENT_H
#define VIS_SEGMENT_H

#include <array>
#include <cmath>
#include <list>
#include <unordered_set>

// minimum allowed number of pixels for a segment to contain (only 1 or higher)
#define MIN_SEG_SIZE 5

// Shapes' paths can be saved in 2 ways:      8-bit     16-bit
#define SHAPE_POINT_T uint32_t             // uint16_t  uint32_t
static int8_t shape_point_bytes = 4;       // 2,        4
static uint8_t shape_point_each_bits = 16; // 8,        16
static float shape_point_max = 65535.0;    // 256.0,    65535.0
// don't make them compiler-level constants, because of their types.

struct Segment {
    // starting from 1
    uint32_t id;
    // pixel coordinates
    std::list<uint32_t> p;
#if MIN_SEG_SIZE == 1
    // sum of colours
    uint64_t ys, us, vs;
#endif
    // average colour
    std::array<uint8_t, 3> m;
    // boundaries, dimensions, their ratio and coordinates of central point
    uint16_t min_y, min_x, max_y, max_x, w, h, r, cx, cy;
    // border pixels
    std::unordered_set<SHAPE_POINT_T> border;

    void ComputeRatioAndCentre() {
        r = static_cast<uint16_t>(round((static_cast<float>(w) / static_cast<float>(h)) * 10.0));
        cx = (min_x + max_x + 1) / 2;
        cy = (min_y + max_y + 1) / 2;
    }
};

#endif //VIS_SEGMENT_H
