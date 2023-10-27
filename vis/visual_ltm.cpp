#include <chrono>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <sys/stat.h>

#include "../global.hpp"
#include "visual_ltm.hpp"

using namespace std;

VisualSTM::VisualSTM() {
    // create directories if they don't exist and resolves their path variables
    struct stat sb{};
    string root;
    for (string *dir: {&root, &dirShapes, &dirFrame, &dirY, &dirU, &dirV, &dirRt}) {
        string branch = *dir;
        dir->insert(0, dirOut);
        if (!branch.empty()) dir->append("/");
        auto path = (*dir).c_str();
        if (stat(path, &sb) != 0) mkdir(path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    }

    // count frames that are stored in memory
    for ([[maybe_unused]] auto &_:
            filesystem::directory_iterator(filesystem::path{dirFrame}))
        framesStored++;

    // load saved state { nextFrameId, nextShapeId, earliestFrameId }
    string savedStatePath = dirOut + savedStateFile;
    if (filesystem::exists(savedStatePath)) {
        ifstream ssf(savedStatePath, ios::binary);
        char buf[18];
        ssf.read(buf, sizeof(buf));
        nextFrameId = littleEndian
                      ? (((uint64_t) buf[7] << 56) | ((uint64_t) buf[6] << 48) |
                         ((uint64_t) buf[5] << 40) | ((uint64_t) buf[4] << 32) |
                         (buf[3] << 24) | (buf[2] << 16) | (buf[1] << 8) | buf[0])
                      : (((uint64_t) buf[0] << 56) | ((uint64_t) buf[1] << 48) |
                         ((uint64_t) buf[2] << 40) | ((uint64_t) buf[3] << 32) |
                         (buf[4] << 24) | (buf[5] << 16) | (buf[1] << 6) | buf[7]);
        nextShapeId = littleEndian
                      ? ((buf[9] << 8) | buf[8])
                      : ((buf[8] << 8) | buf[9]);
        earliestFrameId = littleEndian
                          ? (((uint64_t) buf[17] << 56) | ((uint64_t) buf[16] << 48) |
                             ((uint64_t) buf[15] << 40) | ((uint64_t) buf[14] << 32) |
                             (buf[13] << 24) | (buf[12] << 16) | (buf[11] << 8) | buf[10])
                          : (((uint64_t) buf[10] << 56) | ((uint64_t) buf[11] << 48) |
                             ((uint64_t) buf[12] << 40) | ((uint64_t) buf[13] << 32) |
                             (buf[14] << 24) | (buf[15] << 16) | (buf[16] << 8) | buf[17]);
        ssf.close();
    }
}

void VisualSTM::Insert(
        uint8_t **m, // average colour
        uint16_t *w, uint16_t *h,  // width and height
        uint16_t cx, uint16_t cy, // central points
        unordered_set<SHAPE_POINT_T> *path
) {
    auto r = (uint16_t) round(((float) *w / (float) *h) * 10.0);

    // write shape file
    ofstream shf(dirShapes + to_string(nextShapeId), ios::binary);
    shf.write((char *) *m, 3); // Mean Colour
    shf.write((char *) &r, 2); // Ratio
    shf.write((char *) &nextFrameId, 8); // Frame ID
    shf.write((char *) w, 2); // Width
    shf.write((char *) h, 2); // Height
    shf.write((char *) &cx, 2); // Centre (X)
    shf.write((char *) &cy, 2); // Centre (Y)
    for (SHAPE_POINT_T p: *path)
        shf.write((char *) &p, shape_point_bytes); // Point {X, Y}
    shf.close();

    // update Y indexes
    ofstream y_f((dirY + to_string((*m)[0])).c_str(), ios::app | ios::binary);
    y_f.write((char *) &nextShapeId, 2);
    y_f.close();

    // update U indexes
    ofstream u_f((dirU + to_string((*m)[1])).c_str(), ios::app | ios::binary);
    u_f.write((char *) &nextShapeId, 2);
    u_f.close();

    // update V indexes
    ofstream v_f((dirV + to_string((*m)[2])).c_str(), ios::app | ios::binary);
    v_f.write((char *) &nextShapeId, 2);
    v_f.close();

    // update Ratio indexes
    ofstream rtf((dirRt + to_string(r)).c_str(), ios::app | ios::binary);
    rtf.write((char *) &nextShapeId, 2);
    rtf.close();
    // FIXME nextShapeId is fine but AFTER being written jumps from 127 to 65408 !!!
    //   while shapesInFrame is written without error!
    // I tried `uint16_t sid = nextShapeId` and `reinterpret_cast<const char*>(&sid)`.

    // save and increment shape ID
    shapesInFrame.push_back(nextShapeId);
    nextShapeId++;
    if (nextShapeId > 65535) nextShapeId = 0;
}

void VisualSTM::OnFrameFinished() {
    // save Frame Index
    ofstream f_f((dirFrame + to_string(nextFrameId)).c_str(), ios::binary);
    for (uint16_t sid: shapesInFrame)
        f_f.write((char *) &sid, 2);
    f_f.close();
    shapesInFrame.clear();

    // check if some frames need to be forgotten
    framesStored++;
    if (framesStored > MAX_FRAMES_STORED) Forget();

    nextFrameId++;
    // if (nextFrameId > 18446744073709552000) nextFrameId = 0;
}

void VisualSTM::Forget() {
    auto t = chrono::system_clock::now();
    for (uint64_t f = earliestFrameId; f < earliestFrameId + FORGET_N_FRAMES; f++) {
        IterateIndex((dirFrame + to_string(f)).c_str(), [](VisualSTM *stm, uint16_t sid) -> void {
            uint8_t y, u, v;
            uint16_t r;

            // open shape file, read its necessary details and then remove it
            string sPath = stm->dirShapes + to_string(sid);
            ifstream shf(sPath, ios::binary);
            shf.read(reinterpret_cast<char *>(&y), 1);
            shf.read(reinterpret_cast<char *>(&u), 1);
            shf.read(reinterpret_cast<char *>(&v), 1);
            char buf[2];
            shf.read(buf, 2);
            r = littleEndian
                ? ((buf[1] << 8) | buf[0])
                : ((buf[0] << 8) | buf[1]);
            shf.close();
            remove(sPath.c_str());

            // read unread indices
            if (!stm->ym.contains(y))
                stm->ym[y] = VisualSTM::ReadIndex((stm->dirY + to_string(y)).c_str());
            if (!stm->um.contains(u))
                stm->um[u] = VisualSTM::ReadIndex((stm->dirU + to_string(u)).c_str());
            if (!stm->vm.contains(v))
                stm->vm[v] = VisualSTM::ReadIndex((stm->dirV + to_string(v)).c_str());
            if (!stm->rm.contains(r))
                stm->rm[r] = VisualSTM::ReadIndex((stm->dirRt + to_string(r)).c_str());

            // remove this shape ID from all indexes
            VisualSTM::RemoveFromIndex(&stm->ym[y], sid);
            VisualSTM::RemoveFromIndex(&stm->um[u], sid);
            VisualSTM::RemoveFromIndex(&stm->vm[v], sid);
            VisualSTM::RemoveFromIndex(&stm->rm[r], sid);
        });
        remove((dirFrame + to_string(f)).c_str()); // don't put it in a variable
    }
    SaveIndexes<uint8_t>(&ym, &dirY);
    SaveIndexes<uint8_t>(&um, &dirU);
    SaveIndexes<uint8_t>(&vm, &dirV);
    SaveIndexes<uint16_t>(&rm, &dirRt);

    earliestFrameId += FORGET_N_FRAMES;
    framesStored -= FORGET_N_FRAMES;
    print("Forgetting time: %lld", chrono::duration_cast<chrono::milliseconds>(
            chrono::system_clock::now() - t).count());
}

void VisualSTM::IterateIndex(const char *path, void onEach(VisualSTM *, uint16_t)) {
    char buf[2];
    struct stat sb{}; // never make it a class member!
    stat(path, &sb);
    ifstream sff(path, ios::binary);
    for (off_t _ = 0; _ < sb.st_size; _ += 2) {
        sff.read(buf, 2);
        onEach(this, littleEndian
                     ? ((buf[1] << 8) | buf[0])
                     : ((buf[0] << 8) | buf[1]));
    }
    sff.close();
}

list<uint16_t> VisualSTM::ReadIndex(const char *path) {
    char buf[2];
    struct stat sb{};
    stat(path, &sb);
    ifstream sff(path, ios::binary);
    list<uint16_t> l;
    for (off_t _ = 0; _ < sb.st_size; _ += 2) {
        sff.read(buf, 2);
        l.push_back(littleEndian
                    ? ((buf[1] << 8) | buf[0])
                    : ((buf[0] << 8) | buf[1]));
    }
    sff.close();
    return l;
}

void VisualSTM::RemoveFromIndex(list<uint16_t> *l, uint16_t id) {
    for (auto sid = begin(*l); sid != end(*l); ++sid)
        if (*sid == id) {
            l->erase(sid);
            return;
        }
    print("Shape %u was not found!", id);
}

#pragma clang diagnostic push
#pragma ide diagnostic ignored "UnusedParameter" // true negative!

template<class INT>
void VisualSTM::SaveIndexes(unordered_map<INT, list<uint16_t>> *indexes, string *dir) {
    for (pair<const INT, list<uint16_t>> &index: (*indexes)) {
        string path = (*dir) + to_string(index.first);
        if (!index.second.empty()) {
            ofstream sff(path, ios::binary);
            for (uint16_t sid: index.second)
                sff.write((char *) &sid, 2);
            sff.close();
        } else remove(path.c_str());
    }
    indexes->clear();
}

#pragma clang diagnostic pop

void VisualSTM::SaveState() {
    ofstream ssf((dirOut + savedStateFile).c_str(), ios::binary);
    ssf.write((char *) &nextFrameId, 8);
    ssf.write((char *) &nextShapeId, 2);
    ssf.write((char *) &earliestFrameId, 8);
    ssf.close();
}
