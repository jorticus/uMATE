
#include "MateController.h"

/*
bool MateController::begin()
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

void MateController::send_packet(uint8_t port, packet_t* packet)
{
    send_data(port, reinterpret_cast<uint8_t*>(&packet), sizeof(packet_t));
}


bool MateController::recv_response(OUT uint8_t* port, OUT response_t* response)
{
    return recv_data(port, reinterpret_cast<uint8_t*>(response), sizeof(response_t));
}

uint16_t MateController::query(uint16_t reg, uint16_t param, uint8_t port)
{
    packet_t packet;
    //packet.port = port;
    packet.type = PacketType::Query;
    packet.addr = reg;
    packet.param = param;

    send_packet(port, &packet);

    // Wait for response (BLOCKING)
    uint8_t recv_port;
    response_t response;
    while (!recv_response(&recv_port, &response))
        continue; // TODO: Timeout

    return response.value;
}

void MateController::control(uint16_t reg, uint16_t value, uint8_t port)
{
    packet_t packet;
    packet.type = PacketType::Control;
    packet.addr = reg;
    packet.param = value;
    
    send_packet(port, &packet);

    // Does this send a response?
}


DeviceType MateController::scan(uint8_t port)
{
    uint16_t value = query(0x00, 0, port);
    
    return (DeviceType)value;
}


revision_t MateController::get_revision(uint8_t port)
{
    revision_t rev;
    query(0x0002, 0, port);
}