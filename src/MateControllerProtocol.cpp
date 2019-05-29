
#include "MateControllerProtocol.h"

/*
bool MateControllerProtocol::begin()
{
    // auto dtype = _bus.scan(port);
    // if (dtype == DeviceType::Mate) {
    //     this->_port = port;
    //     return true;
    // }
    //return false; // Device not found on this port

    return true;
}
*/

void MateControllerProtocol::set_timeout(int timeoutMillisec)
{
    this->timeout = timeoutMillisec;
}

void MateControllerProtocol::send_packet(uint8_t port, packet_t* packet)
{
    if (packet == nullptr)
        return;

    // AVR is little-endian, but protocol is big-endian. Must swap bytes...
    packet->addr = SWAPENDIAN_16(packet->addr);
    packet->param = SWAPENDIAN_16(packet->param);

    send_data(port, reinterpret_cast<uint8_t*>(packet), sizeof(packet_t));
}


bool MateControllerProtocol::recv_response(OUT uint8_t* for_command, OUT response_t* response)
{
    if (for_command == nullptr || response == nullptr)
        return false;

    bool received = recv_data(for_command, reinterpret_cast<uint8_t*>(response), sizeof(response_t));
    if (received) {
        // port is actually the command we're responding to, plus an error flag in bit7
        uint8_t c = *for_command;
        if (c & 0x80) {
            if (debug) { 
                debug->print("Invalid command: ");
                debug->println(c & 0x7F, 16);
            }
            return false; // Invalid command
        }

        response->value = SWAPENDIAN_16(response->value);
    }
    return received;
}

bool MateControllerProtocol::recv_response_blocking(OUT uint8_t* for_command, OUT response_t* response)
{
    for (int i = 0; i < this->timeout; i++) {
        if (recv_response(for_command, response)) {
            return true;
        }
        delay(1);
    }
    //if (debug) { debug->println("RX timeout"); }
    return false;
}

int16_t MateControllerProtocol::query(uint16_t reg, uint16_t param, uint8_t port)
{
    packet_t packet;
    //packet.port = port;
    packet.type = PacketType::Query;
    packet.addr = reg;
    packet.param = param;

    send_packet(port, &packet);

    // Wait for response, with timeout (BLOCKING)
    uint8_t for_command;
    response_t response;
    if (recv_response_blocking(&for_command, &response)) {
        return response.value;
    }

    return -1;
}

bool MateControllerProtocol::control(uint16_t reg, uint16_t value, uint8_t port)
{
    packet_t packet;
    packet.type = PacketType::Control;
    packet.addr = reg;
    packet.param = value;
    
    send_packet(port, &packet);

    // Does this send a response?
    return true;
}


DeviceType MateControllerProtocol::scan(uint8_t port)
{
    int16_t value = query(0x00, 0, port);
    if (value >= 0 && value < DeviceType::MaxDevices) {
        return (DeviceType)value;
    }
    return DeviceType::None;
}


revision_t MateControllerProtocol::get_revision(uint8_t port)
{
    int16_t value = 0;
    revision_t rev;
    if ((value = query(2, 0, port)) >= 0)
        rev.a = value;
    if ((value = query(3, 0, port)) >= 0)
        rev.b = value;
    if ((value = query(4, 0, port)) >= 0)
        rev.c = value;
    return rev;
}