#include <cmath>
#include <filesystem>
#include <sys/stat.h>

#include "../global.hpp"
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
        stat(framesPath.c_str(), &sb);
        ifstream fif(framesPath, ios::binary);
        uint64_t fid;
        uint16_t beg, end;
        for (off_t _ = 0; _ < sb.st_size; _ += 12) {
            fid = read_uint64(&fif);
            beg = read_uint16(&fif);
            end = read_uint16(&fif);
            fi[fid] = pair(beg, end);
        }
        fif.close();
        framesStored = fi.size();
    }

    // load file `numbers`: { nextFrameId, firstFrameId, nextShapeId }
    string numbersPath = dirOut + numbersFile;
    if (filesystem::exists(numbersPath)) {
        ifstream ssf(numbersPath, ios::binary);
        nextFrameId = read_uint64(&ssf);
        firstFrameId = read_uint64(&ssf);
        nextShapeId = read_uint16(&ssf);
        firstShapeId = nextShapeId;
        ssf.close();
    }
}

#pragma clang diagnostic push
#pragma ide diagnostic ignored "UnusedParameter" // true negative!

template<class INT>
void VisualSTM::ReadIndices(map<INT, unordered_set<uint16_t>> *indexes, string *dir) {
    struct stat sb{};
    for (const filesystem::directory_entry &ent: filesystem::directory_iterator(
            filesystem::path{*dir})) {
        const char *path = reinterpret_cast<const char *>(ent.path().c_str());
        stat(path, &sb);
        ifstream seq(path, ios::binary);
        unordered_set<uint16_t> l;
        for (off_t _ = 0; _ < sb.st_size; _ += 2)
            l.insert(read_uint16(&seq));
        seq.close();
        (*indexes)[(uint16_t) stoul(ent.path().filename().c_str())] = l;
        remove(path);
    }
}

template<class INT>
void VisualSTM::SaveIndices(map<INT, unordered_set<uint16_t>> *indexes, string *dir) {
    string path;
    for (pair<const INT, unordered_set<uint16_t>> &index: (*indexes)) {
        if (index.second.empty()) continue;
        path = (*dir) + to_string(index.first);
        ofstream sff(path, ios::binary);
        for (uint16_t sid: index.second)
            sff.write((char *) &sid, 2);
        sff.close();
    }
}

#pragma clang diagnostic pop

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

    // update the volatile indices
    yi[(*m)[0]].insert(nextShapeId);
    ui[(*m)[1]].insert(nextShapeId);
    vi[(*m)[2]].insert(nextShapeId);
    ri[r].insert(nextShapeId);

    // increment shape ID
    nextShapeId++;
    if (nextShapeId > 65535) nextShapeId = 0;
}

void VisualSTM::OnFrameFinished() {
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
            string sPath = dirShapes + to_string(sid);
            ifstream shf(sPath, ios::binary);
            shf.read(reinterpret_cast<char *>(&y), 1);
            shf.read(reinterpret_cast<char *>(&u), 1);
            shf.read(reinterpret_cast<char *>(&v), 1);
            r = read_uint16(&shf);
            shf.close();
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
    ofstream fif(dirOut + framesFile, ios::binary);
    for (pair<uint64_t, pair<uint16_t, uint16_t>> f: fi) {
        fif.write((char *) &f.first, 8);
        fif.write((char *) &f.second.first, 2);
        fif.write((char *) &f.second.second, 2);
    }
    fif.close();

    // save file `numbers`
    ofstream ssf(dirOut + numbersFile, ios::binary);
    ssf.write((char *) &nextFrameId, 8);
    ssf.write((char *) &firstFrameId, 8);
    ssf.write((char *) &nextShapeId, 2);
    ssf.close();
}
