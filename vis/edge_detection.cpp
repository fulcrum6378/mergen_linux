#include <chrono>

#include "bitmap.hpp"
#include "edge_detection.hpp"

using namespace std;

EdgeDetection::EdgeDetection(unsigned char **buf) : buf_(buf) {

#if SAVE_BITMAPS >= 1
    Bitmap::createDir();
#endif
}

void EdgeDetection::Process() {

    // 1. loading; bring separate YUV data into the multidimensional array of pixels `arr`
    auto checkPoint = chrono::system_clock::now();
    int off = 0, ww = static_cast<int>(W);
    for (int j = 0; j < bufLength; j += 4) {
        int yy = (off / 3) / ww, xx = (off / 3) % ww;
        arr[yy][xx][0] = (*buf_)[j + 0]; // first pixel
        arr[yy][xx][1] = (*buf_)[j + 1];
        arr[yy][xx][2] = (*buf_)[j + 3];
        arr[yy][xx + 1][0] = (*buf_)[j + 2]; // second pixel
        arr[yy][xx + 1][1] = (*buf_)[j + 1];
        arr[yy][xx + 1][2] = (*buf_)[j + 3];
        off += 6;
    }
#if SAVE_BITMAPS == 1
    Bitmap::save(arr);
#endif
    auto delta1 = chrono::duration_cast<chrono::milliseconds>(
            chrono::system_clock::now() - checkPoint).count();

    // summary: loading +
    /*print("EdgeDetection: %lld + %lld + %lld + %lld + %lld + %lld => %lld",
          delta1, delta2, delta3, delta4, delta5, delta6,
          delta1 + delta2 + delta3 + delta4 + delta5 + delta6);*/
}

EdgeDetection::~EdgeDetection() {
}
