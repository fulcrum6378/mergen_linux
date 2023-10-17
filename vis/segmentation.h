#ifndef VIS_SEGMENTATION_H
#define VIS_SEGMENTATION_H

class Segmentation {
private:
    char **buf_;

public:
    explicit Segmentation(char **buf);

    void Process(__u32 bytesUsed);

    ~Segmentation();
};

#endif //VIS_SEGMENTATION_H
