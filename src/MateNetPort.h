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
    Mx = 3
    // Mate?
    // Flexnet?
} ;

enum PacketType {
    Query = 2,
    Control = 3,
    Status = 4,
    Log = 22
} ;

typedef __attribute__((packed)) struct {
    uint8_t type;
    uint16_t addr;
    uint16_t param;
} packet_t;

// typedef struct {
//     packet_t packet;
//     uint16_t checksum;
// } packet_frame_t;

typedef __attribute__((packed)) struct {
    //uint8_t reserved;
    uint16_t value;
} response_t;

// template<typename T>
// struct frame_s {
//     uint8_t     port;
//     T           payload;
//     uint16_t    checksum;
// };
// typedef frame_s<packet_t> packet_frame_t;
// typedef frame_s<response_t> response_frame_t;

#define MAX_PACKET_LEN (32)

class MateNetPort
{
public:
    MateNetPort(HardwareSerial9b& ser, Stream* debug = nullptr);
    void begin();

    bool available();

    // Query a register and retrieve its value (BLOCKING)
    // reg:      The register address
    // param:    Optional parameter
    // port:     The hub port to send to
    // returns:  The register value
    //uint16_t query(uint16_t reg, uint16_t param = 0, uint8_t port = 0);

    // Control something (BLOCKING)
    // reg:      The control address
    // value:    The value to use for controlling (eg. disable/enable)
    // port:     The hub port to send to
    //void control(uint16_t reg, uint16_t value, uint8_t port = 0);

    // Scan for a device attached to the specified port (BLOCKING)
    // port:     The port to scan, 0-10 (root: 0)
    // returns:  The type of device that is attached
    //DeviceType scan(uint8_t port = 0);

//protected:
    void send_data(uint8_t port, uint8_t* data, uint8_t len);
    bool recv_data(OUT uint8_t* port, OUT uint8_t* data, uint8_t len);
    
    // MATE -> DEVICE
    //void send_packet(uint8_t port, packet_t* packet);
    //bool recv_response(OUT uint8_t* port, OUT response_t* response);
    //bool recv_response(uint8_t* buffer, uint8_t buffer_size, OUT uint8_t* port, OUT uint8_t* recv_bytes);

    // DEVICE -> MATE
    //bool recv_packet(OUT uint8_t* port, OUT packet_t* packet);
    //void send_response(uint8_t port, response_t* response);

protected:
    Stream* debug;

    // Calculate the checksum of some raw data
    // The checksum is a simple 16-bit sum over all the bytes in the packet,
    // including the 9-bit start-of-packet byte (though the 9th bit is not counted)
    uint16_t calc_checksum(uint8_t* data, uint8_t len);

    //bool recv_start_of_packet(uint8_t* sof_byte);

private:
    HardwareSerial9b& ser;

    uint8_t rx_buffer[MAX_PACKET_LEN];
    uint8_t rx_idx;
};


#endif /* MATENETPORT_H */