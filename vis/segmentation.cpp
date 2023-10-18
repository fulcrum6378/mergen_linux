#include <chrono>
#include <cstring>

#include "../global.h"
#include "bitmap.h"
#include "segmentation.h"

using namespace std;

Segmentation::Segmentation(std::atomic_bool *on, unsigned char **buf, VisualSTM *stm) :
        on_(on), buf_(buf), stm(stm) {}

#pragma clang diagnostic push
#pragma ide diagnostic ignored "ConstantConditionsOC"
#pragma ide diagnostic ignored "UnreachableCode"

void Segmentation::Process() { // never use `sizeof(*buf_)`

    // 1. loading; bring separate YUV data into the multidimensional array of pixels `arr`
    auto t0 = chrono::system_clock::now();
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
    //bitmap(arr, "../" + to_string(frame) + ".bmp");
    auto delta1 = chrono::duration_cast<chrono::milliseconds>(
            chrono::system_clock::now() - t0).count();

    // 2. segmentation
    t0 = chrono::system_clock::now();
    uint16_t thisY = 0, thisX = 0;
    int64_t last; // must be signed
    uint32_t nextSeg = 1;
    bool foundSthToAnalyse = true;
    while (foundSthToAnalyse) {
        foundSthToAnalyse = false;
        for (uint16_t y = thisY; y < h; y++) {
            for (uint16_t x = (y == thisY) ? thisX : 0; x < w; x++)
                if (status[y][x] == 0) {
                    foundSthToAnalyse = true;
                    thisY = y;
                    thisX = x;
                    break;
                }
            if (foundSthToAnalyse) break;
        }
        if (!foundSthToAnalyse) break;

        Segment seg{nextSeg};
        stack.push_back(new uint16_t[3]{thisY, thisX, 0});
        nextSeg++;
        uint16_t y, x, dr;
        while ((last = ((int64_t) stack.size()) - 1) != -1) {
            y = stack[last][0], x = stack[last][1], dr = stack[last][2];
            if (dr == 0) {
                seg.p.push_back((y << 16) | x);
                status[y][x] = seg.id;
                // left
                stack[last][2]++;
                if (x > 0 && status[y][x - 1] == 0 && CompareColours(arr[y][x], arr[y][x - 1])) {
                    stack.push_back(new uint16_t[3]{y, static_cast<uint16_t>(x - 1), 0});
                    continue;
                }
            }
            if (dr <= 1) { // top
                stack[last][2]++;
                if (y > 0 && status[y - 1][x] == 0 && CompareColours(arr[y][x], arr[y - 1][x])) {
                    stack.push_back(new uint16_t[3]{static_cast<uint16_t>(y - 1), x, 0});
                    continue;
                }
            }
            if (dr <= 2) { // right
                stack[last][2]++;
                if (x < (w - 1) && status[y][x + 1] == 0 &&
                    CompareColours(arr[y][x], arr[y][x + 1])) {
                    stack.push_back(new uint16_t[3]{y, static_cast<uint16_t>(x + 1), 0});
                    continue;
                }
            }
            if (dr <= 3) { // bottom
                stack[last][2]++;
                if (y < (h - 1) && status[y + 1][x] == 0 &&
                    CompareColours(arr[y][x], arr[y + 1][x])) {
                    stack.push_back(new uint16_t[3]{static_cast<uint16_t>(y + 1), x, 0});
                    continue;
                }
            }
            stack.pop_back();
        }
        segments.push_back(seg);
    }
    auto delta2 = chrono::duration_cast<chrono::milliseconds>(
            chrono::system_clock::now() - t0).count();

    // 3. dissolution
    t0 = chrono::system_clock::now();
    if (min_seg_size > 1) {
        uint32_t absorber_i, size_bef = segments.size(), removal = 1;
        Segment *absorber;
        for (int32_t seg = ((int32_t) size_bef) - 1; seg > -1; seg--)
            if (segments[seg].p.size() < min_seg_size) {
                absorber_i = FindPixelOfASegmentToDissolveIn(&segments[seg]);
                if (absorber_i == 0xFFFFFFFF) continue;
                absorber = &segments[status[absorber_i >> 16][absorber_i & 0xFFFF] - 1];
                for (uint32_t &p: segments[seg].p) {
                    absorber->p.push_back(p); // merge()
                    status[p >> 16][p & 0xFFFF] = absorber->id;
                }
                swap(segments[seg], segments[size_bef - removal]);
                removal++;
            }
        segments.resize(size_bef - (removal - 1));
        print("Total segments: %zu / %u", segments.size(), size_bef);
    } else
        print("Total segments: %zu", segments.size());
    auto delta3 = chrono::duration_cast<chrono::milliseconds>(
            chrono::system_clock::now() - t0).count();

    // 4. average colours + detect boundaries
    t0 = chrono::system_clock::now();
    uint32_t l_;
    unsigned char *col;
    uint64_t aa, bb, cc;
    bool isFirst;
    uint16_t y, x;
    for (Segment &seg: segments) {
        // average colours of each segment
        aa = 0, bb = 0, cc = 0;
        for (uint32_t p: seg.p) {
            col = arr[p >> 16][p & 0xFFFF];
            aa += col[0];
            bb += col[1];
            cc += col[2];
        }
        l_ = seg.p.size();
        seg.m = new uint8_t[3]{static_cast<uint8_t>(aa / l_),
                               static_cast<uint8_t>(bb / l_),
                               static_cast<uint8_t>(cc / l_)};

        // detect boundaries (min_y, min_x, max_y, max_x)
        isFirst = true;
        for (uint32_t &p: seg.p) {
            y = p >> 16;
            x = p & 0xFFFF;
            if (isFirst) {
                seg.min_y = y;
                seg.min_x = x;
                seg.max_y = y;
                seg.max_x = x;
                isFirst = false;
            } else {
                if (y < seg.min_y) seg.min_y = y;
                if (x < seg.min_x) seg.min_x = x;
                if (y > seg.max_y) seg.max_y = y;
                if (x > seg.max_x) seg.max_x = x;
            }
        }
        seg.w = (seg.max_x + 1) - seg.min_x;
        seg.h = (seg.max_y + 1) - seg.min_y;

        // index the Segments by their IDs
        s_index[seg.id] = &seg;
    }
    auto delta4 = chrono::duration_cast<chrono::milliseconds>(
            chrono::system_clock::now() - t0).count();

    // 5. trace border pixels
    t0 = chrono::system_clock::now();
    for (y = 0; y < h; y++) {
        if (y == 0 || y == h - 1)
            for (x = 0; x < w; x++)
                SetAsBorder(y, x);
        else
            for (x = 0; x < w; x++) {
                if (x == 0 or x == w - 1) {
                    SetAsBorder(y, x);
                    continue;
                }
                if (((b_status[y][x] >> 24) & 0xFF) == 1) continue;
                CheckIfBorder(y, x, y, x + 1); //     eastern
                CheckIfBorder(y, x, y + 1, x + 1); // south-eastern
                CheckIfBorder(y, x, y + 1, x); //     southern
                CheckIfBorder(y, x, y + 1, x - 1); // south-western
            }
    }
    auto delta5 = chrono::duration_cast<chrono::milliseconds>(
            chrono::system_clock::now() - t0).count();

    // 6. store the segments
    t0 = chrono::system_clock::now();
    sort(segments.begin(), segments.end(),
         [](const Segment &a, const Segment &b) { return a.p.size() > b.p.size(); });
    l_ = segments.size();
    for (uint16_t seg = 0; seg < max_segs; seg++) {// Segment &seg: segments
        if (seg >= l_) break;
        stm->Insert(&segments[seg].m, &segments[seg].w, &segments[seg].h,
                    (segments[seg].min_x + segments[seg].max_x + 1) / 2, // central point X
                    (segments[seg].min_y + segments[seg].max_y + 1) / 2, // central point Y
                    &segments[seg].border);
    }
    stm->OnFrameFinished();
    auto delta6 = chrono::duration_cast<chrono::milliseconds>(
            chrono::system_clock::now() - t0).count();

    // summary: loading + segmentation + dissolution + segment_analysis + tracing + saving
    print("Delta times: %lld + %lld + %lld + %lld + %lld + %lld => %lld",
          delta1, delta2, delta3, delta4, delta5, delta6,
          delta1 + delta2 + delta3 + delta4 + delta5 + delta6);
    print("----------------------------------");

    // if recording is over, save state of VisualSTM
    if (!*on_) stm->SaveState();

    // clear data and unlock the frame
    memset(status, 0, sizeof(status));
    memset(b_status, 0, sizeof(b_status));
    s_index.clear();
    segments.clear();
}

#pragma clang diagnostic pop

/**
 * - `abs()` is much more efficient than `256 - static_cast<uint8_t>(a - b)`!
 * - There's no need for `static_cast<int16_t>`.
 * - `https://stackoverflow.com/questions/649454/what-is-the-best-way-to-average-two-colors-
 * that-define-a-linear-gradient` doesn't make a (big) difference.
 * - Geometric mean didn't work correctly (0,0,0).
 */
bool Segmentation::CompareColours(unsigned char a[3], unsigned char b[3]) {
    return abs((int16_t) a[0] - (int16_t) b[0]) <= 4 &&
           abs((int16_t) a[1] - (int16_t) b[1]) <= 4 &&
           abs((int16_t) a[2] - (int16_t) b[2]) <= 4;
}

#pragma clang diagnostic push
#pragma ide diagnostic ignored "UnreachableCallsOfFunction"

uint32_t Segmentation::FindPixelOfASegmentToDissolveIn(Segment *seg) const {
    uint32_t cor = seg->p.front();
    uint16_t a = cor >> 16, b = cor & 0xFFFF;
    if (a > 0)
        return ((a - 1) << 16) | b;
    if (b > 0)
        return (a << 16) | (b - 1);
    cor = seg->p.back();
    a = cor >> 16, b = cor & 0xFFFF;
    if (a < h - 1)
        return ((a + 1) << 16) | b;
    if (b < w - 1)
        return (a << 16) | (b + 1);
    return 0xFFFFFFFF;
}

#pragma clang diagnostic pop

void Segmentation::CheckIfBorder(uint16_t y1, uint16_t x1, uint16_t y2, uint16_t x2) {
    if (status[y1][x1] != status[y2][x2]) {
        SetAsBorder(y1, x1);
        SetAsBorder(y2, x2);
    }
}

void Segmentation::SetAsBorder(uint16_t y, uint16_t x) {
    b_status[y][x] |= 1 << 24;
    Segment *seg = s_index[status[y][x]];
    seg->border.insert(
            (static_cast<shape_point_t>((shape_point_max / (float) seg->w) *
                                        (float) (x - seg->min_x)) // fractional X
                    << shape_point_each_bits) |
            static_cast<shape_point_t>((shape_point_max / (float) seg->h) *
                                       (float) (y - seg->min_y))  // fractional Y
    );
}

Segmentation::~Segmentation() = default;
