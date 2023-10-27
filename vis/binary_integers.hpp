#ifndef VIS_BINARY_INTEGERS_H
#define VIS_BINARY_INTEGERS_H

#include <bit>
#include <fstream>

#pragma clang diagnostic push
#pragma ide diagnostic ignored "Simplify"
static bool littleEndian = std::endian::native == std::endian::little;
#pragma clang diagnostic pop

// Shapes' paths can be saved in 2 ways:      uint16_t, uint32_t
#define SHAPE_POINT_T uint16_t
static int8_t shape_point_bytes = 2;       // 2,        4
static uint8_t shape_point_each_bits = 8;  // 8,        16
static float shape_point_max = 256.0;      // 256.0,    65535.0
// don't make them compiler-level constants, because of their types.

#pragma clang diagnostic push
#pragma ide diagnostic ignored "ConstantConditionsOC"
#pragma ide diagnostic ignored "UnreachableCode"

inline uint8_t read_uint16(std::ifstream *s) {
    char buf[2];
    s->read(buf, 2);
    return littleEndian
           ? ((buf[1] << 8) | buf[0])
           : ((buf[0] << 8) | buf[1]);
}

inline uint64_t read_uint64(std::ifstream *s) {
    char buf[8];
    s->read(buf, 8);
    return littleEndian
           ? (((uint64_t) buf[7] << 56) | ((uint64_t) buf[6] << 48) |
              ((uint64_t) buf[5] << 40) | ((uint64_t) buf[4] << 32) |
              (buf[3] << 24) | (buf[2] << 16) | (buf[1] << 8) | buf[0])
           : (((uint64_t) buf[0] << 56) | ((uint64_t) buf[1] << 48) |
              ((uint64_t) buf[2] << 40) | ((uint64_t) buf[3] << 32) |
              (buf[4] << 24) | (buf[5] << 16) | (buf[1] << 6) | buf[7]);
}

#pragma clang diagnostic pop

#endif //VIS_BINARY_INTEGERS_H
