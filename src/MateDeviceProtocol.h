#ifndef MATE_DEVICE_PROTOCOL_H
#define MATE_DEVICE_PROTOCOL_H

#include "uMate.h"

/*
    Allows you to communicate with the MATE bus as a device.
    useful for emulating devices (Hub, MX, FX, etc)
*/
class MateDeviceProtocol : public MateNetPort
{
public:
    MateDeviceProtocol(Stream9b& ser, Stream* debug = nullptr)
        : MateNetPort(ser, debug)
    { }

    // Receive a packet. Must be called periodically
    bool recv_packet(OUT uint8_t* port, OUT packet_t* packet);

    // Send a response to a received packet
    // type should match the type of command that this is responding to
    void send_response(PacketType type, response_t* response);
};


#endif /* MATE_DEVICE_PROTOCOL_H */