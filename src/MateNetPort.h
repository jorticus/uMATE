#ifndef MATENETPORT_H
#define MATENETPORT_H

#include <stdint.h>
#include <Arduino.h>
#include <Serial9b.h>

#ifndef OUT
#define OUT
#endif

enum DeviceType {
    None = 0,
    Hub = 1,
    Fx = 2,
    Mx = 3,
    FlexNetDc = 4,

    MaxDevices
};

enum PacketType {
    Decrement = 0,
    Increment = 1,
    Read = 2,
    Write = 3,
    Status = 4,
    Log = 22
};

typedef struct {
    uint8_t type;
    uint16_t addr;
    uint16_t param;
} packet_t __attribute__((packed));

// typedef struct {
//     packet_t packet;
//     uint16_t checksum;
// } packet_frame_t;

typedef struct {
    //uint8_t reserved;
    uint16_t value;
} response_t __attribute__((packed));

// template<typename T>
// struct frame_s {
//     uint8_t     port;
//     T           payload;
//     uint16_t    checksum;
// };
// typedef frame_s<packet_t> packet_frame_t;
// typedef frame_s<response_t> response_frame_t;

#define MAX_PACKET_LEN (32)

enum CommsStatus {
    Success = 0,
    NoData,
    Timeout,
    NoStartOfPacketFound,
    BufferOverrun,
    InsufficientData,
    BadChecksum
};

/*
    Implements the MATE framing layer.
    There are no restrictions on what kind of packet you can send/receive,
    so you will probably want to use a MateDeviceProtocol or MateControllerProtocol on top of this.
*/
class MateNetPort
{
public:
    MateNetPort(HardwareSerial9b& ser, Stream* debug = nullptr);
    void begin();

    bool available();

    void send_data(uint8_t port, uint8_t* data, uint8_t len);
    CommsStatus recv_data(OUT uint8_t* port, OUT uint8_t* data, OUT uint8_t* len);

protected:
    Stream* debug;

    // Calculate the checksum of some raw data
    // The checksum is a simple 16-bit sum over all the bytes in the packet,
    // including the 9-bit start-of-packet byte (though the 9th bit is not counted)
    uint16_t calc_checksum(uint8_t* data, uint8_t len);

private:
    HardwareSerial9b& ser;

    uint8_t rx_buffer[MAX_PACKET_LEN];
    uint8_t rx_idx;
};


#endif /* MATENETPORT_H */