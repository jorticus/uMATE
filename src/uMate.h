#ifndef UMATE_H
#define UMATE_H

#ifndef OUT
#define OUT
#endif

#define SWAPENDIAN_16(x) (uint16_t)((((x) & 0xFF00) >> 8) | (((x) & 0x00FF) << 8))
#define SWAPENDIAN_32(x)            (uint32_t)((((x) & 0xFF000000UL) >> 24UL) | (((x) & 0x00FF0000UL) >> 8UL) | \
                                               (((x) & 0x0000FF00UL) << 8UL)  | (((x) & 0x000000FFUL) << 24UL))

#define NUM_MATE_PORTS (10)
#define MATENET_BAUD 9600

#include "MateNetPort.h"
#include "MateControllerProtocol.h"
//#include "MateControllerDevice.h" // TODO: This doesn't compile here...
#include "MateDeviceProtocol.h"


#endif /* UMATE_H */