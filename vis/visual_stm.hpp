#ifndef VIS_VISUAL_STM_H
#define VIS_VISUAL_STM_H

#include <array>
#include <map>
#include <unordered_set>

#include "segment.hpp"

// maximum frames allowed to be present in memory at a time
#define MAX_FRAMES_STORED 3u
// forget N frames whenever hit the maximum
#define FORGET_N_FRAMES 1u

/**
 * Visual Short-Term Memory
 * deprecated and currently used only for debugging.
 *
 * @see <a href="https://github.com/fulcrum6378/mycv/blob/master/storage/volatile_indices_1.py">
 * Volatile Indices 1</a>
 * @see <a href="https://github.com/fulcrum6378/mycv/blob/master/storage/sequence_files_2.py">
 * Sequence Files 2</a>
 */
class VisualSTM {
public:
    VisualSTM();

    /** Inserts a new shape into memory. */
    void Insert(Segment *seg);

    /** Anything that needs to be done at the end. */
    void OnFrameFinished();

    /** Saves current state { nextFrameId, firstFrameId, nextShapeId }
     * Don't save paths as variables in the constructor! */
    void SaveState();


    // frame ID incrementer
    uint64_t nextFrameId = 0ull;
private:
    /** Forgets N of oldest frames. */
    void Forget();

    /** Reads all Sequence Files of an aspect and then deletes it. */
    template<class INT>
    static void ReadIndices(std::map<INT, std::unordered_set<uint16_t>> *indexes, std::string *dir);

    /** Save all indices of an aspect as Sequence Files. */
    template<class INT>
    void SaveIndices(std::map<INT, std::unordered_set<uint16_t>> *indexes, std::string *dir);


    const std::string dirOut = "vis/stm/";
    std::string dirShapes = "shapes", dirY = "y", dirU = "u", dirV = "v", dirR = "r",
            framesFile = "frames", numbersFile = "numbers";
    // ID of earliest frame which is STILL available in memory
    uint64_t firstFrameId = 0ull;
    // shape ID incrementer | ID of first shape in THIS FRAME
    uint16_t nextShapeId = 0u, firstShapeId = 0u;
    // total number of frames available in memory
    uint16_t framesStored = 0u;
    // frame index (8-bit)
    std::map<uint64_t, std::pair<uint16_t, uint16_t>> fi;
    // 8-bit volatile indices
    std::map<uint8_t, std::unordered_set<uint16_t>> yi, ui, vi;
    // 16-bit volatile indices
    std::map<uint16_t, std::unordered_set<uint16_t>> ri;
};

#endif //VIS_VISUAL_STM_H
