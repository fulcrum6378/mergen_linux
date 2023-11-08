#include <algorithm> // std::sort
#include <chrono>
#include <cmath>
#include <cstring>
#include <filesystem>
#include <set> // RG2
#include <sys/stat.h> // SAVE_BITMAPS

#include "../global.hpp"
#include "bitmap.hpp"
#include "segmentation.hpp"

using namespace std;

Segmentation::Segmentation(unsigned char **buf) : buf_(buf), stm(new VisualSTM()) {

#if SAVE_BITMAPS >= 1 // prepare to save bitmaps if wanted
    struct stat sb{};
    if (stat(dirBitmap.c_str(), &sb) != 0)
        mkdir(dirBitmap.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    else
        for (const auto &entry: std::filesystem::directory_iterator(dirBitmap))
            std::filesystem::remove_all(entry.path());
#endif
}

void Segmentation::Process() {

    // 1. loading; bring separate YUV data into the multidimensional array of pixels `arr`
    auto t0 = chrono::system_clock::now();
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
    bitmap(arr, dirBitmap + to_string(stm->nextFrameId) + ".bmp");
#endif
    auto delta1 = chrono::duration_cast<chrono::milliseconds>(
            chrono::system_clock::now() - t0).count();

    // 2. segmentation
    t0 = chrono::system_clock::now();
    uint32_t nextSeg = 1u;
#if !RG2
    uint16_t thisY = 0u, thisX = 0u;
    int64_t last; // must be signed
    bool foundSthToAnalyse = true;
#pragma clang diagnostic push
#pragma ide diagnostic ignored "ConstantConditionsOC" // false positive
    while (foundSthToAnalyse) {
#pragma clang diagnostic pop
        foundSthToAnalyse = false;
        for (uint16_t y = thisY; y < H; y++) {
            for (uint16_t x = (y == thisY) ? thisX : 0u; x < W; x++)
                if (status[y][x] == 0u) {
                    foundSthToAnalyse = true;
                    thisY = y;
                    thisX = x;
                    break;
                }
            if (foundSthToAnalyse) break;
        }
        if (!foundSthToAnalyse) break;

        Segment seg{nextSeg};
        stack.push_back({thisY, thisX, 0});
        nextSeg++;
        uint16_t y, x, dr;
        while ((last = static_cast<int64_t>(stack.size()) - 1) != -1) {
            y = stack[last][0], x = stack[last][1], dr = stack[last][2];
            if (dr == 0) {
                seg.p.push_back((y << 16) | x);
#if MIN_SEG_SIZE == 1u // add colours in order to compute their mean value later
                seg.ys += arr[y][x][0] * arr[y][x][0];
                seg.us += arr[y][x][1] * arr[y][x][1];
                seg.vs += arr[y][x][2] * arr[y][x][2];
#endif
                status[y][x] = seg.id;
                // left
                stack[last][2]++;
                if (x > 0 && status[y][x - 1] == 0 && CompareColours(&arr[y][x], &arr[y][x - 1u])) {
                    stack.push_back({y, static_cast<uint16_t>(x - 1u), 0u});
                    continue;
                }
            }
            if (dr <= 1) { // top
                stack[last][2]++;
                if (y > 0 && status[y - 1][x] == 0 && CompareColours(&arr[y][x], &arr[y - 1u][x])) {
                    stack.push_back({static_cast<uint16_t>(y - 1u), x, 0u});
                    continue;
                }
            }
            if (dr <= 2) { // right
                stack[last][2]++;
                if (x < (W - 1) && status[y][x + 1] == 0 && CompareColours(&arr[y][x], &arr[y][x + 1u])) {
                    stack.push_back({y, static_cast<uint16_t>(x + 1u), 0u});
                    continue;
                }
            }
            if (dr <= 3) { // bottom
                stack[last][2]++;
                if (y < (H - 1) && status[y + 1][x] == 0 && CompareColours(&arr[y][x], &arr[y + 1u][x])) {
                    stack.push_back({static_cast<uint16_t>(y + 1u), x, 0u});
                    continue;
                }
            }
            stack.pop_back();
        }
        segments.push_back(seg);
    }
#else
    uint32_t nSeg, segmentOfAnyNeighbour = 0u, chosenOne, removal = 1u;
    uint16_t ry, rx, by, bx, ly, lx, ty, tx;
    bool anyQualified;
    set<uint32_t> allowedRegions;
    uint8_t nAllowedRegions = 0u;
    for (uint16_t y = 0u; y < h; y++) {
        for (uint16_t x = 0; x < w; x++) {
            if (status[y][x] != 0) continue;

            // analyse neighbours
            nSeg = status[ry][rx];
            if (x < (w - 1u)) { // right
                ry = y;
                rx = x + 1u;
                if (CompareColours(arr[y][x], arr[ry][rx])) {
                    anyQualified = true;
                    if (nSeg != 0u) allowedRegions.insert(nSeg);
                }
                if (nSeg != 0u && segmentOfAnyNeighbour != 0u) segmentOfAnyNeighbour = nSeg;
            }
            nSeg = status[by][bx] != 0u;
            if (y < (h - 1u)) { // bottom
                by = y + 1u;
                bx = x;
                if (CompareColours(arr[y][x], arr[by][bx])) {
                    anyQualified = true;
                    if (nSeg != 0u) allowedRegions.insert(nSeg);
                }
                if (nSeg != 0u && segmentOfAnyNeighbour != 0u) segmentOfAnyNeighbour = nSeg;
            }
            nSeg = status[ly][lx] != 0u;
            if (x > 0u) { // left
                ly = y;
                lx = x - 1u;
                if (CompareColours(arr[y][x], arr[ly][lx])) {
                    anyQualified = true;
                    if (nSeg != 0u) allowedRegions.insert(nSeg);
                }
                if (nSeg != 0u && segmentOfAnyNeighbour != 0u) segmentOfAnyNeighbour = nSeg;
            }
            nSeg = status[ty][tx] != 0u;
            if (y > 0u) { // top
                ty = y - 1u;
                tx = x;
                if (CompareColours(arr[y][x], arr[ty][tx])) {
                    anyQualified = true;
                    if (nSeg != 0u) allowedRegions.insert(nSeg);
                }
                if (nSeg != 0u && segmentOfAnyNeighbour != 0u) segmentOfAnyNeighbour = nSeg;
            }

            // determine the segment of this pixel
            if (anyQualified) {
                nAllowedRegions = allowedRegions.size();
                if (nAllowedRegions == 0u) {
                    segments[nextSeg] = Segment{nextSeg};
                    status[y][x] = nextSeg;
                } else { // repair the pixels
                    chosenOne = *allowedRegions.begin();
                    for (uint32_t sid: allowedRegions)
                        if (sid != chosenOne) {
                            for (uint32_t changer: segments[sid].p) {
                                segments[chosenOne].p.push_back(changer);
                                status[changer >> 16][changer & 0xFFFF] = chosenOne;
                            }
                            segments.erase(sid);
                        }
                    status[y][x] = chosenOne;
                }
            } else {
                if (segmentOfAnyNeighbour != 0u)
                    status[y][x] = segmentOfAnyNeighbour;
                else {
                    segments[nextSeg] = Segment{nextSeg};
                    status[y][x] = nextSeg;
                }
            }
            segments[nextSeg].p.push_back((y << 16) | x);

            anyQualified = false;
            allowedRegions.clear();
            segmentOfAnyNeighbour = 0u;
            nextSeg++;
        }
    }
#endif //RG2
    auto delta2 = chrono::duration_cast<chrono::milliseconds>(
            chrono::system_clock::now() - t0).count();

    // 3. dissolution
    t0 = chrono::system_clock::now();
#if MIN_SEG_SIZE != 1u && !RG2
    uint32_t absorber_i, size_bef = segments.size(), removal = 1u;
    Segment *absorber;
    for (int32_t seg = static_cast<int32_t>(size_bef) - 1; seg > -1; seg--)
        if (segments[seg].p.size() < MIN_SEG_SIZE) {
            absorber_i = FindPixelOfASegmentToDissolveIn(&segments[seg]);
            if (absorber_i == 0xFFFFFFFF) continue;
            absorber = &segments[status[absorber_i >> 16][absorber_i & 0xFFFF] - 1u];
            for (uint32_t &p: segments[seg].p) {
                absorber->p.push_back(p); // merge()
                status[p >> 16][p & 0xFFFF] = absorber->id;
            }
            swap(segments[seg], segments[size_bef - removal]);
            removal++;
        }
    segments.resize(size_bef - (removal - 1u));
    print("Total segments: %zu / %u", segments.size(), size_bef);
#else
    print("Total segments: %zu", segments.size());
#endif
    auto delta3 = chrono::duration_cast<chrono::milliseconds>(
            chrono::system_clock::now() - t0).count();

    // 4. average colours + detect boundaries
    t0 = chrono::system_clock::now();
    uint32_t l_;
#if MIN_SEG_SIZE != 1u
    array<uint8_t, 3u> *col;
    uint64_t ys, us, vs;
#endif
    bool isFirst;
    uint16_t y, x;
#if !RG2
    for (Segment &seg: segments) {
#else
        for (auto &ss: segments) {
            Segment seg = ss.second;
#endif
        // average colours of each segment
        l_ = seg.p.size();
#if MIN_SEG_SIZE != 1u
        ys = 0u, us = 0u, vs = 0u;
        for (uint32_t p: seg.p) {
            col = reinterpret_cast<array<uint8_t, 3u> *>(&arr[p >> 16][p & 0xFFFF]);
            ys += (*col)[0] * (*col)[0]; // pow(, 2)
            us += (*col)[1] * (*col)[1];
            vs += (*col)[2] * (*col)[2];
        }
        seg.m = {static_cast<uint8_t>(sqrt(ys / l_)),
                 static_cast<uint8_t>(sqrt(us / l_)),
                 static_cast<uint8_t>(sqrt(vs / l_))};
#else
        seg.m = {static_cast<uint8_t>(sqrt(seg.ys / l_)),
                 static_cast<uint8_t>(sqrt(seg.us / l_)),
                 static_cast<uint8_t>(sqrt(seg.vs / l_))};
#endif
        // https://stackoverflow.com/questions/649454/what-is-the-best-way-to-average-two-colors-that-
        // define-a-linear-gradient

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
        seg.w = (seg.max_x + 1u) - seg.min_x;
        seg.h = (seg.max_y + 1u) - seg.min_y;

        // index the Segments by their IDs
        s_index[seg.id] = &seg;
    }
    auto delta4 = chrono::duration_cast<chrono::milliseconds>(
            chrono::system_clock::now() - t0).count();

    // 5. trace border pixels
    t0 = chrono::system_clock::now();
    for (y = 0u; y < H; y++) {
        if (y == 0u || y == H - 1u)
            for (x = 0; x < W; x++)
                SetAsBorder(y, x);
        else
            for (x = 0u; x < W; x++) {
                if (x == 0u or x == W - 1u) {
                    SetAsBorder(y, x);
                    continue;
                }
                if (b_status[y][x] == 1) continue;
                CheckIfBorder(y, x, y, x + 1u); //     eastern
                CheckIfBorder(y, x, y + 1u, x + 1u); // south-eastern
                CheckIfBorder(y, x, y + 1u, x); //     southern
                CheckIfBorder(y, x, y + 1u, x - 1u); // south-western
            }
    }
#if SAVE_BITMAPS == 2
    bitmap(arr, dirBitmap + to_string(stm->nextFrameId) + ".bmp");
#endif
    auto delta5 = chrono::duration_cast<chrono::milliseconds>(
            chrono::system_clock::now() - t0).count();

    // 6. (save in VisualSTM and) track objects and measure their differences
    t0 = chrono::system_clock::now();
#if !RG2
    sort(segments.begin(), segments.end(),
         [](const Segment &a, const Segment &b) { return a.p.size() > b.p.size(); });
    float nearest_dist, dist;
    int32_t best;
    l_ = segments.size();
    for (uint16_t sid = 0u; sid < MAX_SEGS; sid++) {// Segment &seg: segments
        if (sid >= l_) break;
        Segment *seg = &segments[sid];
        seg->ComputeRatioAndCentre();
#if VISUAL_STM
        stm->Insert(seg);
#endif
        if (!prev_segments.empty()) {
            for (uint8_t y_ = seg->m[0] - Y_RADIUS; y_ < seg->m[0] + Y_RADIUS; y_++) {
                auto it = yi.find(y_);
                if (it == yi.end()) continue;
                for (uint16_t i: (*it).second) a_y.insert(i);
            }
            for (uint8_t u_ = seg->m[1] - U_RADIUS; u_ < seg->m[1] + U_RADIUS; u_++) {
                auto it = ui.find(u_);
                if (it == ui.end()) continue;
                for (uint16_t i: (*it).second) a_u.insert(i);
            }
            for (uint8_t v_ = seg->m[2] - V_RADIUS; v_ < seg->m[2] + V_RADIUS; v_++) {
                auto it = vi.find(v_);
                if (it == vi.end()) continue;
                for (uint16_t i: (*it).second) a_v.insert(i);
            }
            for (uint16_t r_ = seg->r - R_RADIUS; r_ < seg->r + R_RADIUS; r_++) {
                auto it = ri.find(r_);
                if (it == ri.end()) continue;
                for (uint16_t i: (*it).second) a_r.insert(i);
            }
            best = -1;
            for (uint16_t can: a_y)
                if (a_u.find(can) != a_u.end() && a_v.find(can) != a_v.end()
                    && a_r.find(can) != a_r.end()) {
                    Segment *prev_seg = &prev_segments[can];
                    dist = static_cast<float>(sqrt(pow(seg->cx - prev_seg->cx, 2) +
                                                   pow(seg->cy - prev_seg->cy, 2)));
                    if (best == -1) { // NOLINT(bugprone-branch-clone)
                        nearest_dist = dist;
                        best = static_cast<int32_t>(can);
                    } else if (dist < nearest_dist) {
                        nearest_dist = dist;
                        best = static_cast<int32_t>(can);
                    } // else {don't set `best` here}
                }
            a_y.clear();
            a_u.clear();
            a_v.clear();
            a_r.clear();
            if (best != -1) {
                Segment *prev_seg = &prev_segments[best];
                diff[sid] = {
                        best, static_cast<int32_t>(nearest_dist),
                        prev_seg->w - seg->w, prev_seg->h - seg->h, prev_seg->r - seg->r,
                        prev_seg->m[0] - seg->m[0], prev_seg->m[1] - seg->m[1],
                        prev_seg->m[2] - seg->m[2],
                };
                /*LOGI("%u->%d : %d, %d, %d, %d, %d, %d, %d", sid, diff[sid][0], diff[sid][1],
                     diff[sid][2], diff[sid][3], diff[sid][4],
                     diff[sid][5], diff[sid][6], diff[sid][7]);*/
            }/* else
                LOGI("Segment %u was lost", sid);*/
        }
        // index segments of the current frame
        _yi[seg->m[0]].insert(sid);
        _ui[seg->m[1]].insert(sid);
        _vi[seg->m[2]].insert(sid);
        _ri[seg->r].insert(sid);
    }
    // replace indexes of the previous frame with the current one
    yi = std::move(_yi);
    ui = std::move(_ui);
    vi = std::move(_vi);
    ri = std::move(_ri);
#if VISUAL_STM
    stm->OnFrameFinished();
#endif
#endif //!RG2
    auto delta6 = chrono::duration_cast<chrono::milliseconds>(
            chrono::system_clock::now() - t0).count();

    // summary: loading + segmentation + dissolution + segment_analysis + tracing + saving
    print("Delta times: %lld + %lld + %lld + %lld + %lld + %lld => %lld",
          delta1, delta2, delta3, delta4, delta5, delta6,
          delta1 + delta2 + delta3 + delta4 + delta5 + delta6);
    print("----------------------------------");

#if VISUAL_STM
    // if recording is over, save state of VisualSTM
    if (!on) stm->SaveState();
#endif

    // clear data and unlock the frame
    memset(status, 0, sizeof(status));
    memset(b_status, 0, sizeof(b_status));
    s_index.clear();
    prev_segments = std::move(segments);
    diff.clear();
}

bool Segmentation::CompareColours(uint8_t (*a)[3], uint8_t (*b)[3]) {
    return abs((*a)[0] - (*b)[0]) <= 4 &&
           abs((*a)[1] - (*b)[1]) <= 4 &&
           abs((*a)[2] - (*b)[2]) <= 4;
}

uint32_t Segmentation::FindPixelOfASegmentToDissolveIn(Segment *seg) {
    uint32_t cor = seg->p.front();
    uint16_t a = cor >> 16, b = cor & 0xFFFF;
    if (a > 0u)
        return ((a - 1u) << 16) | b;
    if (b > 0u)
        return (a << 16) | (b - 1u);
    cor = seg->p.back();
    a = cor >> 16, b = cor & 0xFFFF;
    if (a < H - 1u)
        return ((a + 1u) << 16) | b;
    if (b < W - 1u)
        return (a << 16) | (b + 1u);
    return 0xFFFFFFFF;
}

void Segmentation::CheckIfBorder(uint16_t y1, uint16_t x1, uint16_t y2, uint16_t x2) {
    if (status[y1][x1] != status[y2][x2]) {
        SetAsBorder(y1, x1);
        SetAsBorder(y2, x2);
    }
}

void Segmentation::SetAsBorder(uint16_t y, uint16_t x) {
    b_status[y][x] |= 1u;
#if SAVE_BITMAPS == 2
    arr[y][x][0] = 76u, arr[y][x][1] = 84u, arr[y][x][2] = 255u;
#endif
    Segment *seg = s_index[status[y][x]];
    seg->border.insert(
            (static_cast<SHAPE_POINT_T>((SHAPE_POINT_MAX / static_cast<float>(seg->h)) *
                                        static_cast<float>(y - seg->min_y)) // fractional Y
                    << SHAPE_POINT_EACH_BITS) |
            static_cast<SHAPE_POINT_T>((SHAPE_POINT_MAX / static_cast<float>(seg->w)) *
                                       static_cast<float>(x - seg->min_x))  // fractional X
    ); // they get reversed in while writing to a file
}

Segmentation::~Segmentation() {
    delete stm;
}
