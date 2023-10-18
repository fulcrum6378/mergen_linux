#include "bitmap.h"
#include "segmentation.h"

using namespace std;

static int frame = 0;

Segmentation::Segmentation(unsigned char **buf) : buf_(buf) {}

void Segmentation::Process() { // never use `sizeof(*buf_)`
    unsigned char arr[480][640][3];
    int i = 0;
    for (int j = 0; j < bufLength; j += 4) {
        int yy = (i / 3) / 640, xx = (i / 3) % 640;

        // first pixel
        arr[yy][xx][0] = (*buf_)[j + 0];
        arr[yy][xx][1] = (*buf_)[j + 1];
        arr[yy][xx][2] = (*buf_)[j + 3];

        //second pixel
        arr[yy][xx + 1][0] = (*buf_)[j + 2];
        arr[yy][xx + 1][1] = (*buf_)[j + 1];
        arr[yy][xx + 1][2] = (*buf_)[j + 3];

        i += 6;
    }

    bitmap(arr, "../" + to_string(frame) + ".bmp");
    frame++;
}

Segmentation::~Segmentation() = default;
