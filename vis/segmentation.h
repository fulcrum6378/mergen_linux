#ifndef VIS_SEGMENTATION_H
#define VIS_SEGMENTATION_H

#include <atomic>
#include <list>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "visual_stm.h"

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
    std::unordered_set<shape_point_t> border;
};

class Segmentation {
private:
    std::atomic_bool *on_;
    unsigned char **buf_;

    // width, height
    const uint16_t h = 480, w = 640;
    // minimum allowed number of pixels for a segment to contain
    const uint32_t min_seg_size = 1;
    // maximum allowed segments to be stored in the short-term memory
    const uint16_t max_segs = 10;

    // multidimensional array of pixels + last byte indicates whether it is a border pixel or not.
    unsigned char arr[480][640][3]{}; // four bytes: b(1=true,0=false), Y, U, V
    // maps pixels to their Segment IDs
    uint32_t status[480][640]{};
    // maps pixels to their Segment IDs
    uint8_t b_status[480][640]{};
    // a vector containing all Segments
    std::vector<Segment> segments;
    // simulates recursive programming (vector is always better for it than list!)
    std::vector<uint16_t *> stack;
    // maps IDs of Segments to their pointers
    std::unordered_map<uint32_t, Segment *> s_index;
    // visual short-term memory (output)
    VisualSTM *stm;

    static bool CompareColours(unsigned char a[3], unsigned char b[3]);

    uint32_t FindPixelOfASegmentToDissolveIn(Segment *seg) const;

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
