#include <chrono>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <sys/stat.h>

#include "../global.hpp"
#include "visual_ltm.hpp"

using namespace std;

VisualLTM::VisualLTM() {
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

    // load saved state { nextFrameId, firstFrameId, nextShapeId }
    string savedStatePath = dirOut + savedStateFile;
    if (filesystem::exists(savedStatePath)) {
        ifstream ssf(savedStatePath, ios::binary);
        nextFrameId = read_uint64(&ssf);
        firstFrameId = read_uint64(&ssf);
        nextShapeId = read_uint16(&ssf);
        ssf.close();
    }
}

[[maybe_unused]] void VisualLTM::Insert(
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
    // FIX-ME nextShapeId is fine but AFTER being written jumps from 127 to 65408 !!!
    //   while shapesInFrame is written without error!
    // I tried `uint16_t sid = nextShapeId` and `reinterpret_cast<const char*>(&sid)`.

    // save and increment shape ID
    shapesInFrame.push_back(nextShapeId);
    nextShapeId++;
    if (nextShapeId > 65535) nextShapeId = 0;
}

[[maybe_unused]] void VisualLTM::OnFrameFinished() {
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

void VisualLTM::Forget() {
    auto t = chrono::system_clock::now();
    for (uint64_t f = firstFrameId; f < firstFrameId + FORGET_N_FRAMES; f++) {
        IterateIndex((dirFrame + to_string(f)).c_str(), [](VisualLTM *stm, uint16_t sid) -> void {
            uint8_t y, u, v;
            uint16_t r;

            // open shape file, read its necessary details and then remove it
            string sPath = stm->dirShapes + to_string(sid);
            ifstream shf(sPath, ios::binary);
            shf.read(reinterpret_cast<char *>(&y), 1);
            shf.read(reinterpret_cast<char *>(&u), 1);
            shf.read(reinterpret_cast<char *>(&v), 1);
            r = read_uint16(&shf);
            shf.close();
            remove(sPath.c_str());

            // read unread indices
            if (!stm->ym.contains(y))
                stm->ym[y] = VisualLTM::ReadIndex((stm->dirY + to_string(y)).c_str());
            if (!stm->um.contains(u))
                stm->um[u] = VisualLTM::ReadIndex((stm->dirU + to_string(u)).c_str());
            if (!stm->vm.contains(v))
                stm->vm[v] = VisualLTM::ReadIndex((stm->dirV + to_string(v)).c_str());
            if (!stm->rm.contains(r))
                stm->rm[r] = VisualLTM::ReadIndex((stm->dirRt + to_string(r)).c_str());

            // remove this shape ID from all indexes
            VisualLTM::RemoveFromIndex(&stm->ym[y], sid);
            VisualLTM::RemoveFromIndex(&stm->um[u], sid);
            VisualLTM::RemoveFromIndex(&stm->vm[v], sid);
            VisualLTM::RemoveFromIndex(&stm->rm[r], sid);
        });
        remove((dirFrame + to_string(f)).c_str()); // don't put it in a variable
    }
    SaveIndexes<uint8_t>(&ym, &dirY);
    SaveIndexes<uint8_t>(&um, &dirU);
    SaveIndexes<uint8_t>(&vm, &dirV);
    SaveIndexes<uint16_t>(&rm, &dirRt);

    firstFrameId += FORGET_N_FRAMES;
    framesStored -= FORGET_N_FRAMES;
    print("Forgetting time: %lld", chrono::duration_cast<chrono::milliseconds>(
            chrono::system_clock::now() - t).count());
}

void VisualLTM::IterateIndex(const char *path, void onEach(VisualLTM *, uint16_t)) {
    struct stat sb{}; // never make it a class member!
    stat(path, &sb);
    ifstream sff(path, ios::binary);
    for (off_t _ = 0; _ < sb.st_size; _ += 2)
        onEach(this, read_uint16(&sff));
    sff.close();
}

list<uint16_t> VisualLTM::ReadIndex(const char *path) {
    struct stat sb{};
    stat(path, &sb);
    ifstream sff(path, ios::binary);
    list<uint16_t> l;
    for (off_t _ = 0; _ < sb.st_size; _ += 2)
        l.push_back(read_uint16(&sff));
    sff.close();
    return l;
}

void VisualLTM::RemoveFromIndex(list<uint16_t> *l, uint16_t id) {
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
void VisualLTM::SaveIndexes(unordered_map<INT, list<uint16_t>> *indexes, string *dir) {
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

[[maybe_unused]] void VisualLTM::SaveState() {
    ofstream ssf((dirOut + savedStateFile).c_str(), ios::binary);
    ssf.write((char *) &nextFrameId, 8);
    ssf.write((char *) &firstFrameId, 8);
    ssf.write((char *) &nextShapeId, 2);
    ssf.close();
}
