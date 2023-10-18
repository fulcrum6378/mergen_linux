#ifndef VIS_SEGMENTATION_H
#define VIS_SEGMENTATION_H

class Segmentation {
private:
    unsigned char **buf_;

public:
    uint32_t bufLength{};

    explicit Segmentation(unsigned char **buf);

    void Process();

    ~Segmentation();
};

#endif //VIS_SEGMENTATION_H
