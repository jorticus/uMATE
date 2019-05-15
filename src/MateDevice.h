#ifndef MATE_DEVICE_H
#define MATE_DEVICE_H

#include "uMate.h"

class MateDevice : public MateNetPort
{
public:
    MateDevice(HardwareSerial9b& ser, Stream* debug = nullptr)
        : MateNetPort(ser, debug)
    { }

    // Receive a packet. Must be called periodically
    bool recv_packet(OUT uint8_t* port, OUT packet_t* packet);

    // Send a response to a received packet
    void send_response(uint8_t port, response_t* response);
};


#endif /* MATE_DEVICE_H */