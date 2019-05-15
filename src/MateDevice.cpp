#include "MateNetPort.h"
#include "MateDevice.h"

bool MateDevice::recv_packet(OUT uint8_t* port, OUT packet_t* packet)
{
    bool received = recv_data(port, reinterpret_cast<uint8_t*>(packet), sizeof(packet_t));
    if (received) {
        // AVR is little-endian, but protocol is big-endian. Must swap bytes...
        packet->addr = SWAPENDIAN_16(packet->addr);
        packet->param = SWAPENDIAN_16(packet->param);
    }
    return received;
}

void MateDevice::send_response(uint8_t port, response_t* response)
{
    response->value = SWAPENDIAN_16(response->value);

    send_data(port, reinterpret_cast<uint8_t*>(response), sizeof(response_t));
}
