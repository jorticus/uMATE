#include "MxController.h"


typedef struct {
    uint8_t a; // raw_ah(part1)
    int8_t b;  // pv_current = b+128
    int8_t c;  // bat_current = c+128
    int8_t d;  // raw_kwh(part1)
    uint8_t e; // raw_ah(part2)
    uint8_t f; // ???
    uint8_t status;
    uint8_t errors;
    uint8_t j; // raw_kwh(part2)
    uint16_t bat_voltage;
    uint16_t pv_voltage;
} mx_status_packed_t;


