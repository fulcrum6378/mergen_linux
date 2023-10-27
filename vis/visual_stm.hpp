#ifndef VIS_VISUAL_STM_H
#define VIS_VISUAL_STM_H

#include <array>
#include <map>
#include <unordered_set>

#include "binary_integers.hpp"

// maximum frames allowed to be present in memory at a time
#define MAX_FRAMES_STORED 3
// forget N frames whenever hit the maximum
#define FORGET_N_FRAMES 1

/**
 * Visual Short-Term Memory
 *
 * @see <a href="https://github.com/fulcrum6378/mycv/blob/master/storage/volatile_indices_1.py">
 * Volatile Indices 1</a>
 * @see <a href="https://github.com/fulcrum6378/mycv/blob/master/storage/sequence_files_2.py">
 * Sequence Files 2</a>
 */
class VisualSTM {
private:
    const std::string dirOut = "vis/stm/";
    std::string dirShapes = "shapes", dirY = "y", dirU = "u", dirV = "v", dirR = "r",
            framesFile = "frames", numbersFile = "numbers";
    // ID of earliest frame which is STILL available in memory
    uint64_t firstFrameId = 0;
    // shape ID incrementor | ID of first shape in THIS FRAME
    uint16_t nextShapeId = 0, firstShapeId = 0;
    // total number of frames available in memory
    uint16_t framesStored = 0;
    // frame index (8-bit)
    std::map<uint64_t, std::pair<uint16_t, uint16_t>> fi;
    // 1-bit volatile indices
    std::map<uint8_t, std::unordered_set<uint16_t>> yi, ui, vi;
    // 2-bit volatile indices
    std::map<uint16_t, std::unordered_set<uint16_t>> ri;

    /** Forgets N of oldest frames. */
    void Forget();

    /** Reads all Sequence Files of an aspect and then deletes it. */
    template<class INT>
    static void ReadIndices(std::map<INT, std::unordered_set<uint16_t>> *indexes, std::string *dir);

    /** Save all indices of an aspect as Sequence Files. */
    template<class INT>
    void SaveIndices(std::map<INT, std::unordered_set<uint16_t>> *indexes, std::string *dir);

public:
    // frame ID incrementor
    uint64_t nextFrameId = 0;

    VisualSTM();

    /** Inserts a new shape into memory. */
    void Insert(
            std::array<uint8_t, 3> *m, // average colour
            uint16_t *w, uint16_t *h, // width and height
            uint16_t cx, uint16_t cy, // central points
            std::unordered_set<SHAPE_POINT_T> *path
    );

    /** Anything that needs to be done at the end. */
    void OnFrameFinished();

    /** Saves current state { nextFrameId, firstFrameId, nextShapeId }
     * Don't save paths as variables in the constructor! */
    void SaveState();
};

#endif //VIS_VISUAL_STM_H
