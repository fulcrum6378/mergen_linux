#include <cstring>
#include <filesystem>
#include <fstream>
#include <sys/stat.h>

#include "visual_stm.hpp"

using namespace std;

VisualSTM::VisualSTM() {
    // create directories if they don't exist and resolves their path variables
    struct stat sb{};
    string root;
    for (string *dir: {&root, &dirShapes, &dirY, &dirU, &dirV, &dirR}) {
        string branch = *dir;
        dir->insert(0, dirOut);
        if (!branch.empty()) dir->append("/");
        auto path = (*dir).c_str();
        if (stat(path, &sb) != 0) mkdir(path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    }

    // load volatile indices
    ReadIndices(&yi, &dirY);
    ReadIndices(&ui, &dirU);
    ReadIndices(&vi, &dirV);
    ReadIndices(&ri, &dirR);

    // load file `frames` (frame index {frame ID, beginning shape ID, ending shape ID})
    string framesPath = dirOut + framesFile;
    if (filesystem::exists(framesPath)) {
        uint64_t fid;
        uint16_t beg, end;
        stat(framesPath.c_str(), &sb);
        char buf[sb.st_size];
        ifstream fif(framesPath, ios::binary);
        fif.read(buf, sb.st_size);
        fif.close();
        for (off_t off = 0; off < sb.st_size; off += 12) {
            memcpy(&fid, &buf[off], 8);
            memcpy(&beg, &buf[off + 8], 2);
            memcpy(&end, &buf[off + 10], 2);
            fi[fid] = pair(beg, end);
        }
        framesStored = fi.size();
    }

    // load file `numbers`: { nextFrameId, firstFrameId, nextShapeId }
    string numbersPath = dirOut + numbersFile;
    if (filesystem::exists(numbersPath)) {
        char buf[18];
        ifstream ssf(numbersPath, ios::binary);
        ssf.read(buf, sizeof(buf));
        ssf.close();
        memcpy(&nextFrameId, &buf[0], 8);
        memcpy(&firstFrameId, &buf[8], 8);
        memcpy(&nextShapeId, &buf[16], 2);
        firstShapeId = nextShapeId;
    }
}

template<class INT>
void VisualSTM::ReadIndices(map<INT, unordered_set<uint16_t>> *indexes, string *dir) {
    struct stat sb{};
    for (const filesystem::directory_entry &ent: filesystem::directory_iterator(
            filesystem::path{*dir})) {
        const char *path = reinterpret_cast<const char *>(ent.path().c_str());
        stat(path, &sb);
        char buf[sb.st_size];
        ifstream seq(path, ios::binary);
        seq.read(buf, sb.st_size);
        seq.close();
        unordered_set<uint16_t> l;
        uint16_t i;
        for (off_t off = 0; off < sb.st_size; off += 2) {
            memcpy(&i, &buf[off], 2);
            l.insert(i);
        }
        (*indexes)[static_cast<uint16_t>(stoul(ent.path().filename().c_str()))] = l;
        remove(path);
    }
}

#pragma clang diagnostic push
#pragma ide diagnostic ignored "UnusedParameter" // false positive

template<class INT>
void VisualSTM::SaveIndices(map<INT, unordered_set<uint16_t>> *indexes, string *dir) {
    string path;
    uint32_t off;
    for (pair<const INT, unordered_set<uint16_t>> &index: (*indexes)) {
        if (index.second.empty()) continue;
        char buf[index.second.size() * 2];
        off = 0;
        for (uint16_t sid: index.second) {
            memcpy(&buf[off], &sid, 2);
            off += 2;
        }
        path = (*dir) + to_string(index.first);
        ofstream sff(path, ios::binary);
        sff.write(buf, sizeof(buf));
        sff.close();
    }
}

#pragma clang diagnostic pop

[[maybe_unused]] void VisualSTM::Insert(Segment *seg) {
    // put data in a buffer
    uint64_t off = 21;
    char buf[off + (shape_point_bytes * seg->border.size())];
    memcpy(&buf[0], &seg->m, 3); // Mean Colour
    memcpy(&buf[3], &seg->r, 2); // Ratio
    memcpy(&buf[5], &nextFrameId, 8); // Frame ID
    memcpy(&buf[13], &seg->w, 2); // Width
    memcpy(&buf[15], &seg->h, 2); // Height
    memcpy(&buf[17], &seg->cx, 2); // Centre (X)
    memcpy(&buf[19], &seg->cy, 2); // Centre (Y)
    for (SHAPE_POINT_T p: seg->border) {
        memcpy(&buf[off], &p, shape_point_bytes); // Point {X, Y}
        off += shape_point_bytes;
    }

    // write buffer to shape file
    ofstream shf(dirShapes + to_string(nextShapeId), ios::binary);
    shf.write(buf, static_cast<streamsize>(sizeof(buf)));
    shf.close();

    // update the volatile indices
    yi[seg->m[0]].insert(nextShapeId);
    ui[seg->m[1]].insert(nextShapeId);
    vi[seg->m[2]].insert(nextShapeId);
    ri[seg->r].insert(nextShapeId);

    // increment shape ID
    nextShapeId++;
    if (nextShapeId > 65535) nextShapeId = 0;
}

[[maybe_unused]] void VisualSTM::OnFrameFinished() {
    // index this frame
    fi[nextFrameId] = pair(firstShapeId, nextShapeId);
    firstShapeId = nextShapeId;

    // check if some frames need to be forgotten
    framesStored++;
    if (framesStored > MAX_FRAMES_STORED) Forget();

    nextFrameId++;
    // if (nextFrameId > 18446744073709552000) nextFrameId = 0;
}

void VisualSTM::Forget() {
    pair<uint16_t, uint16_t> *range;
    for (uint64_t f = firstFrameId; f < firstFrameId + FORGET_N_FRAMES; f++) {
        range = &fi[f];
        for (uint16_t sid = range->first; sid < range->second; sid++) {
            uint8_t y, u, v;
            uint16_t r;

            // open shape file, read its necessary details and then remove it
            char b_s[5];
            string sPath = dirShapes + to_string(sid);
            ifstream shf(sPath, ios::binary);
            shf.read(b_s, sizeof(b_s));
            shf.close();
            memcpy(&y, &b_s[0], 1);
            memcpy(&u, &b_s[1], 1);
            memcpy(&v, &b_s[2], 1);
            memcpy(&r, &b_s[3], 2);
            remove(sPath.c_str());

            // remove this shape ID from all indexes
            yi[y].erase(sid);
            ui[u].erase(sid);
            vi[v].erase(sid);
            ri[r].erase(sid);
        }
        fi.erase(f);
    }

    firstFrameId += FORGET_N_FRAMES;
    framesStored -= FORGET_N_FRAMES;
}

void VisualSTM::SaveState() {
    // save volatile indices
    SaveIndices<uint8_t>(&yi, &dirY);
    SaveIndices<uint8_t>(&ui, &dirU);
    SaveIndices<uint8_t>(&vi, &dirV);
    SaveIndices<uint16_t>(&ri, &dirR);

    // save file `frames`
    char b_f[fi.size() * 12];
    uint32_t off = 0;
    for (pair<uint64_t, pair<uint16_t, uint16_t>> f: fi) {
        memcpy(&b_f[off], &f.first, 8);
        off += 8;
        memcpy(&b_f[off], &f.second.first, 2);
        off += 2;
        memcpy(&b_f[off], &f.second.second, 2);
        off += 2;
    }
    ofstream fif(dirOut + framesFile, ios::binary);
    fif.write(b_f, static_cast<int32_t>(sizeof(b_f)));
    fif.close();

    // save file `numbers`
    char b_n[18];
    memcpy(&b_n[0], &nextFrameId, 8);
    memcpy(&b_n[8], &firstFrameId, 8);
    memcpy(&b_n[16], &nextShapeId, 2);
    ofstream ssf(dirOut + numbersFile, ios::binary);
    ssf.write(b_n, sizeof(b_n));
    ssf.close();
}
