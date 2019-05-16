#ifndef MATE_H
#define MATE_H

#include "uMate.h"

typedef struct {
    uint16_t a;
    uint16_t b;
    uint16_t c;
} revision_t;

/*
class MateController
{
public:
    MateController(MateNetPort& bus)
        : _bus(bus)
    { }

    bool begin();

    // Query a register and retrieve its value (BLOCKING)
    // reg:      The register address
    // param:    Optional parameter
    // port:     The hub port to send to
    // returns:  The register value
    uint16_t query(uint16_t reg, uint16_t param = 0, uint8_t port = 0);

    // Control something (BLOCKING)
    // reg:      The control address
    // value:    The value to use for controlling (eg. disable/enable)
    // port:     The hub port to send to
    void control(uint16_t reg, uint16_t value, uint8_t port = 0);

    // Scan for a device attached to the specified port (BLOCKING)
    // port:     The port to scan, 0-10 (root: 0)
    // returns:  The type of device that is attached
    DeviceType scan(uint8_t port = 0);

    // Read the revision from the target device
    revision_t get_revision(uint8_t port = 0);

private:
    MateNetPort& _bus;
};
*/

class MateController : public MateNetPort
{
public:
    MateController(HardwareSerial9b& ser, Stream* debug = nullptr)
        : MateNetPort(ser, debug)
    { }

    void send_packet(uint8_t port, packet_t* packet);
    bool recv_response(OUT uint8_t* for_command, OUT response_t* response);

    // Query a register and retrieve its value (BLOCKING)
    // reg:      The register address
    // param:    Optional parameter
    // port:     The hub port to send to
    // returns:  The register value
    uint16_t query(uint16_t reg, uint16_t param = 0, uint8_t port = 0);

    // Control something (BLOCKING)
    // reg:      The control address
    // value:    The value to use for controlling (eg. disable/enable)
    // port:     The hub port to send to
    void control(uint16_t reg, uint16_t value, uint8_t port = 0);

    // Scan for a device attached to the specified port (BLOCKING)
    // port:     The port to scan, 0-10 (root: 0)
    // returns:  The type of device that is attached
    DeviceType scan(uint8_t port = 0);

    // Read the revision from the target device
    revision_t get_revision(uint8_t port = 0);
};

#endif /* MATE_H */