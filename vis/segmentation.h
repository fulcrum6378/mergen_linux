#ifndef VIS_SEGMENTATION_H
#define VIS_SEGMENTATION_H

class Segmentation {
private:
    unsigned char **buf_;

public:
    explicit Segmentation(unsigned char **buf);

    void Process(__u32 bytesUsed);

    ~Segmentation();
};

#endif //VIS_SEGMENTATION_H
