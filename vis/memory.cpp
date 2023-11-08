#include <chrono>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <sys/stat.h>

#include "../global.hpp"
#include "segment.hpp"
#include "memory.hpp"

using namespace std;

VisMemory::VisMemory() {
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
}

[[maybe_unused]] void VisMemory::Insert(
        uint8_t **m, // average colour
        uint16_t *w, uint16_t *h,  // width and height
        uint16_t cx, uint16_t cy, // central points
        unordered_set<uint16_t> *path
) {
    auto r = static_cast<uint16_t>(round((static_cast<float>(*w) / static_cast<float>(*h)) * 10.0));

    // put data in a buffer
    uint64_t off = 21u;
    char buf[off + (SHAPE_POINT_BYTES * (*path).size())];
    memcpy(&buf[0], m, 3u); // Mean Colour
    memcpy(&buf[3], &r, 2u); // Ratio
    memcpy(&buf[5], &nextFrameId, 8u); // Frame ID
    memcpy(&buf[13], w, 2u); // Width
    memcpy(&buf[15], h, 2u); // Height
    memcpy(&buf[17], &cx, 2u); // Centre (X)
    memcpy(&buf[19], &cy, 2u); // Centre (Y)
    for (uint16_t p: *path) {
        memcpy(&buf[off], &p, SHAPE_POINT_BYTES); // Point {X, Y}
        off += SHAPE_POINT_BYTES;
    }

    // write buffer to shape file
    ofstream shf(dirShapes + to_string(nextShapeId), ios::binary);
    shf.write(buf, static_cast<streamsize>(sizeof(buf)));
    shf.close();

    // update Y indexes
    ofstream y_f((dirY + to_string((*m)[0])).c_str(), ios::app | ios::binary);
    y_f.write((char *) &nextShapeId, 2u);
    y_f.close();

    // update U indexes
    ofstream u_f((dirU + to_string((*m)[1])).c_str(), ios::app | ios::binary);
    u_f.write((char *) &nextShapeId, 2u);
    u_f.close();

    // update V indexes
    ofstream v_f((dirV + to_string((*m)[2])).c_str(), ios::app | ios::binary);
    v_f.write((char *) &nextShapeId, 2u);
    v_f.close();

    // update Ratio indexes
    ofstream rtf((dirR + to_string(r)).c_str(), ios::app | ios::binary);
    rtf.write((char *) &nextShapeId, 2u);
    rtf.close();
    // FIX-ME nextShapeId is fine but AFTER being written jumps from 127 to 65408 !!!
    //   while shapesInFrame is written without error!
    // I tried `uint16_t sid = nextShapeId` and `reinterpret_cast<const char*>(&sid)`.
}

[[maybe_unused]] void VisMemory::Forget() {
    auto t = chrono::system_clock::now();
    for (uint64_t f = firstFrameId; f < firstFrameId + 1u; f++) {
        IterateIndex((/*dirFrame + */to_string(f)).c_str(), [](VisMemory *ltm, uint16_t sid
        ) -> void {
            uint8_t y, u, v;
            uint16_t r;

            // open shape file, read its necessary details and then remove it
            char b_s[5];
            string sPath = ltm->dirShapes + to_string(sid);
            ifstream shf(sPath, ios::binary);
            shf.read(b_s, sizeof(b_s));
            shf.close();
            memcpy(&y, &b_s[0], 1u);
            memcpy(&u, &b_s[1], 1u);
            memcpy(&v, &b_s[2], 1u);
            memcpy(&r, &b_s[3], 2u);
            remove(sPath.c_str());

            // read unread indices
            if (!ltm->ym.contains(y))
                ltm->ym[y] = VisMemory::ReadIndex((ltm->dirY + to_string(y)).c_str());
            if (!ltm->um.contains(u))
                ltm->um[u] = VisMemory::ReadIndex((ltm->dirU + to_string(u)).c_str());
            if (!ltm->vm.contains(v))
                ltm->vm[v] = VisMemory::ReadIndex((ltm->dirV + to_string(v)).c_str());
            if (!ltm->rm.contains(r))
                ltm->rm[r] = VisMemory::ReadIndex((ltm->dirR + to_string(r)).c_str());

            // remove this shape ID from all indexes
            VisMemory::RemoveFromIndex(&ltm->ym[y], sid);
            VisMemory::RemoveFromIndex(&ltm->um[u], sid);
            VisMemory::RemoveFromIndex(&ltm->vm[v], sid);
            VisMemory::RemoveFromIndex(&ltm->rm[r], sid);
        });
        remove((/*dirFrame + */to_string(f)).c_str()); // don't put it in a variable
    }
    SaveIndexes<uint8_t>(&ym, &dirY);
    SaveIndexes<uint8_t>(&um, &dirU);
    SaveIndexes<uint8_t>(&vm, &dirV);
    SaveIndexes<uint16_t>(&rm, &dirR);

    firstFrameId += 1u;
    print("Forgetting time: %lld", chrono::duration_cast<chrono::milliseconds>(
            chrono::system_clock::now() - t).count());
}

void VisMemory::IterateIndex(const char *path, void onEach(VisMemory *, uint16_t)) {
    struct stat sb{}; // never make it a class member!
    stat(path, &sb);
    char buf[sb.st_size];
    ifstream sff(path, ios::binary);
    sff.read(buf, sb.st_size);
    sff.close();
    uint16_t i;
    for (off_t off = 0; off < sb.st_size; off += 2) {
        memcpy(&i, &buf[off], 2u);
        onEach(this, i);
    }
}

list<uint16_t> VisMemory::ReadIndex(const char *path) {
    struct stat sb{};
    stat(path, &sb);
    char buf[sb.st_size];
    ifstream sff(path, ios::binary);
    sff.read(buf, sb.st_size);
    sff.close();
    list<uint16_t> l;
    uint16_t i;
    for (off_t off = 0; off < sb.st_size; off += 2) {
        memcpy(&i, &buf[off], 2u);
        l.push_back(i);
    }
    return l;
}

void VisMemory::RemoveFromIndex(list<uint16_t> *l, uint16_t id) {
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
void VisMemory::SaveIndexes(unordered_map<INT, list<uint16_t>> *indexes, string *dir) {
    string path;
    uint32_t off;
    for (pair<const INT, list<uint16_t>> &index: (*indexes)) {
        if (!index.second.empty()) {
            char buf[index.second.size() * 2u];
            off = 0u;
            for (uint16_t sid: index.second) {
                memcpy(&buf[off], &sid, 2u);
                off += 2u;
            }
            path = (*dir) + to_string(index.first);
            ofstream sff(path, ios::binary);
            sff.write(buf, sizeof(buf));
            sff.close();
        } else remove(path.c_str());
    }
    indexes->clear();
}

#pragma clang diagnostic pop
