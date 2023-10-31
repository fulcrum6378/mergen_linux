#ifndef VIS_MEMORY_H
#define VIS_MEMORY_H

#include <list>
#include <unordered_map>
#include <unordered_set>

/**
 * Visual Memory
 *
 * @see <a href="https://github.com/fulcrum6378/mycv/blob/master/storage/sequence_files_2.py">
 * Sequence Files 2</a>
 */
class [[maybe_unused]] VisMemory {
private:
    const std::string dirOut = "vis/mem/";
    std::string dirShapes = "shapes", dirY = "y", dirU = "u", dirV = "v", dirR = "r";
    // ID of earliest frame which is STILL available in memory
    uint64_t firstFrameId = 0;
    // shape ID incrementer
    uint16_t nextShapeId = 0;
    // helper maps for altering 'uint8_t' indexes
    std::unordered_map<uint8_t, std::list<uint16_t>> ym, um, vm;
    // helper maps for altering 'uint16_t' indexes
    std::unordered_map<uint16_t, std::list<uint16_t>> rm;

    /** Forgets N of oldest frames. */
    [[maybe_unused]] void Forget();

    /** Iterates on every ID in a Sequence File. */
    void IterateIndex(const char *path, void onEach(VisMemory *, uint16_t));

    /** Reads an entire Sequence File. */
    static std::list<uint16_t> ReadIndex(const char *path);

    /** Removes an ID from a list (index/sequence file). */
    static void RemoveFromIndex(std::list<uint16_t> *l, uint16_t id);

    /** Save an index in non-volatile memory and clear its data from RAM. */
    template<class INT>
    void SaveIndexes(std::unordered_map<INT, std::list<uint16_t>> *indexes, std::string *dir);

public:
    // frame ID incrementer
    uint64_t nextFrameId = 0;

    VisMemory();

    /** Inserts a new shape into memory. */
    [[maybe_unused]] void Insert(
            uint8_t **m, // average colour
            uint16_t *w, uint16_t *h, // width and height
            uint16_t cx, uint16_t cy, // central point
            std::unordered_set<uint16_t> *path
    );
};

#endif //VIS_MEMORY_H
