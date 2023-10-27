#ifndef VIS_BINARY_INTEGERS_H
#define VIS_BINARY_INTEGERS_H

// Shapes' paths can be saved in 2 ways:      uint16_t, uint32_t
#define SHAPE_POINT_T uint16_t
static int8_t shape_point_bytes = 2;       // 2,        4
static uint8_t shape_point_each_bits = 8;  // 8,        16
static float shape_point_max = 256.0;      // 256.0,    65535.0
// don't make them compiler-level constants, because of their types.

#endif //VIS_BINARY_INTEGERS_H
