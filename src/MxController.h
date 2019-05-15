#ifndef MX_CONTROLLER_H
#define MX_CONTROLLER_H

#include <stdint.h>

typedef struct {
    int8_t pv_current;
    int8_t bat_current;
    uint16_t raw_ah;
    uint16_t raw_kwh;
    uint8_t status;
    uint8_t errors;
    uint16_t bat_voltage;
    uint16_t pv_voltage;
} mx_status_t;

typedef struct {
    int day;
    int amp_hours;
    int kilowatt_hours;
    int volts_peak;
    int amps_peak;
    int kilowatts_peak;
    int bat_min;
    int bat_max;
    int absorb_time;
    int float_time;
} mx_logpage_t;

// #if (sizeof(mx_status_t) != 12)
// #error "Invalid struct size"
// #endif

#endif /* MX_CONTROLLER_H */