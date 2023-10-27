#ifndef VIS_VISUAL_STM_H
#define VIS_VISUAL_STM_H

#include <bit>
#include <cstdint>
#include <list>
#include <string>
#include <unordered_map>
#include <unordered_set>

// Shapes' paths can be saved in 2 ways:      uint16_t, uint32_t
#define SHAPE_POINT_T uint16_t
static int8_t shape_point_bytes = 2;       // 2,        4
static uint8_t shape_point_each_bits = 8;  // 8,        16
static float shape_point_max = 256.0;      // 256.0,    65535.0
// don't make them compiler-level constants, because of their types.

// maximum frames allowed to be present in memory at a time
#define MAX_FRAMES_STORED 10
// forget N frames whenever hit the maximum
#define FORGET_N_FRAMES 1

#pragma clang diagnostic push
#pragma ide diagnostic ignored "Simplify"
static bool littleEndian = std::endian::native == std::endian::little;
#pragma clang diagnostic pop

/** Visual Short-Term Memory */
class VisualSTM {
private:
    const std::string dirOut = "vis/stm/";
    std::string dirShapes = "shapes", dirFrame = "f", dirY = "y", dirU = "u", dirV = "v", dirRt = "r",
            savedStateFile = "saved_state";
    uint16_t nextShapeId = 0;
    // ID of earliest frame which is STILL available in memory
    uint64_t earliestFrameId = 0;
    // total number of frames available in memory
    uint16_t framesStored = 0;
    // IDs of shapes inside current frame
    std::list<uint16_t> shapesInFrame;
    // helper maps for altering 'uint8_t' indexes
    std::unordered_map<uint8_t, std::list<uint16_t>> ym, um, vm;
    // helper maps for altering 'uint16_t' indexes
    std::unordered_map<uint16_t, std::list<uint16_t>> rm;

    /** Forgets N of oldest frames. */
    void Forget();

    /** Iterates on every ID in a Sequence File. */
    void IterateIndex(const char *path, void onEach(VisualSTM *, uint16_t));

    /** Reads an entire Sequence File. */
    static std::list<uint16_t> ReadIndex(const char *path);

    /** Removes an ID from a list (index/sequence file). */
    static void RemoveFromIndex(std::list<uint16_t> *l, uint16_t id);

    /** Save index in non-volatile memory and clear its data from RAM. */
    template<class INT>
    void SaveIndexes(std::unordered_map<INT, std::list<uint16_t>> *indexes, std::string *dir);

public:
    uint64_t nextFrameId = 0;

    VisualSTM();

    /** Inserts a new shape into memory. */
    void Insert(
            uint8_t **m, // average colour
            uint16_t *w, uint16_t *h, // width and height
            uint16_t cx, uint16_t cy, // central points
            std::unordered_set<SHAPE_POINT_T> *path
    );

    /** Anything that needs to be done at the end. */
    void OnFrameFinished();

    /** Saves current state { nextFrameId, nextShapeId, earliestFrameId }
     * Don't save paths as variables in the constructor! */
    void SaveState();
};

#endif //VIS_VISUAL_STM_H
