#ifndef VIS_EDGE_DETECTION_H
#define VIS_EDGE_DETECTION_H

#include <cstdint>

#include "global.hpp"

class EdgeDetection {
public:
    explicit EdgeDetection(unsigned char **buf);

    void Process();

    ~EdgeDetection();


    uint32_t bufLength{};

private:
    // buffer of raw image frames in YUYV format
    unsigned char **buf_;
    // multidimensional array of pixels
    uint8_t arr[H][W][3]{};
    // maps pixels to their status of being border or not
    uint8_t statuses[H][W]{};
};

#endif //VIS_EDGE_DETECTION_H
