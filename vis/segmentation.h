#ifndef VIS_SEGMENTATION_H
#define VIS_SEGMENTATION_H

#include <array>
#include <atomic>
#include <list>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "visual_ltm.hpp"

// height of an image frame
#define H 480
// width of an image frame
#define W 640
// minimum allowed number of pixels for a segment to contain
#define MIN_SEG_SIZE 8
// maximum allowed segments to be stored in the short-term memory
#define MAX_SEGS 10
// 0=>no, 1=>yes, 2=>yes with border highlights
#define SAVE_BITMAPS 2
// enable method "Region Growing 2" in favour of the 4th
#define RG2 false

struct Segment {
    // starting from 1
    uint32_t id;
    // pixel coordinates
    std::list<uint32_t> p;
    // average colour
    uint8_t *m;
    // boundaries and dimensions
    uint16_t min_y, min_x, max_y, max_x, w, h;
    // border pixels
    std::unordered_set<SHAPE_POINT_T> border;
};

class Segmentation {
private:
    std::atomic_bool *on_;
    unsigned char **buf_;

    // multidimensional array of pixels
    uint8_t arr[H][W][3]{};
    // maps pixels to their Segment IDs
    uint32_t status[H][W]{};
    // maps pixels to their status of being border or not
    uint8_t b_status[H][W]{};
    // a vector containing all Segments
#if !RG2
    std::vector<Segment> segments;
    // simulates recursive programming (vector is always better for it than list!)
    std::vector<std::array<uint16_t, 3>> stack;
#else
    std::unordered_map<uint32_t, Segment> segments;
#endif
    // maps IDs of Segments to their pointers
    std::unordered_map<uint32_t, Segment *> s_index;
    // visual short-term memory (output)
    VisualSTM *stm;

    static bool CompareColours(uint8_t (*a)[3], uint8_t (*b)[3]);

    static uint32_t FindPixelOfASegmentToDissolveIn(Segment *seg);

    // Checks if this pixel is in border.
    void CheckIfBorder(uint16_t y1, uint16_t x1, uint16_t y2, uint16_t x2);

    // Recognises this pixel as border.
    void SetAsBorder(uint16_t y, uint16_t x);

public:
    uint32_t bufLength{};

    explicit Segmentation(std::atomic_bool *on, unsigned char **buf, VisualSTM *stm);

    void Process();

    ~Segmentation();
};

#endif //VIS_SEGMENTATION_H
