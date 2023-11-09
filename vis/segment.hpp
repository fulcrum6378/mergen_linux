#ifndef VIS_SEGMENT_H
#define VIS_SEGMENT_H

#include <array>
#include <cmath>
#include <list>
#include <unordered_set>

// minimum allowed number of pixels for a segment to contain (only 1 or higher)
#define MIN_SEG_SIZE 5u

// Shapes' paths can be saved as:     8-bit   or 16-bit
#define SHAPE_POINT_T uint32_t     // uint16_t   uint32_t
#define SHAPE_POINT_BYTES 4u       // 2u,        4u
#define SHAPE_POINT_EACH_BITS 16u  // 8u,        16u
#define SHAPE_POINT_MAX 65535.0f   // 256.0f,    65535.0f

struct Segment {
    // starting from 1
    uint32_t id;
    // pixel coordinates
    std::list<uint32_t> p;
#if MIN_SEG_SIZE == 1u
    // sum of colours
    uint64_t ys, us, vs;
#endif
    // average colour
    std::array<uint8_t, 3u> m;
    // boundaries, dimensions, their ratio and coordinates of central point
    uint16_t min_y, min_x, max_y, max_x, w, h, r, cx, cy;
    // border pixels
    std::unordered_set<SHAPE_POINT_T> border;

    void ComputeRatioAndCentre() {
        r = static_cast<uint16_t>(std::round((static_cast<float>(w) / static_cast<float>(h)) * 10.0f));
        cx = (min_x + max_x + 1u) / 2u;
        cy = (min_y + max_y + 1u) / 2u;
    }
};

#endif //VIS_SEGMENT_H
