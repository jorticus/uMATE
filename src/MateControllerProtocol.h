#ifndef MATE_CONTROLLER_PROTOCOL_H
#define MATE_CONTROLLER_PROTOCOL_H

#include "uMate.h"
#include <Serial9b.h>

#define NUM_PORTS (10)

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

/*
    Allows you to control devices attached to a MATE bus.
    Emulates the functionality of a MATE2/MATE3 unit.
*/
class MateControllerProtocol : public MateNetPort
{
public:
    MateControllerProtocol(Stream9b& ser, Stream* debug = nullptr)
        : MateNetPort(ser, debug), timeout(100), devices_scanned(false)
    { }

    void set_timeout(int timeoutMillisec);

    // Scan for all devices attached to the MATE bus (BLOCKING)
    void scan_ports();

    void send_packet(uint8_t port, packet_t* packet);

    bool recv_response(OUT uint8_t* for_command, OUT uint8_t* response, uint8_t response_len);
    bool recv_response_blocking(OUT uint8_t* for_command, OUT uint8_t* response, uint8_t response_len);

    bool recv_response(OUT uint8_t* for_command, OUT response_t* response);
    bool recv_response_blocking(OUT uint8_t* for_command, OUT response_t* response);

    // Increment/Decrement or Enable/Disable a register
    // addr:    The register address
    // port:    The hub port to send to
    // returns: The updated value of the register
    // int16_t increment(uint16_t addr, uint8_t port);
    // int16_t decrement(uint16_t addr, uint8_t port);
    // int16_t disable(uint16_t addr, uint8_t port);
    // int16_t enable(uint16_t addr, uint8_t port);

    // Query a register and retrieve its value (BLOCKING)
    // reg:      The register address
    // param:    Optional parameter
    // port:     The hub port to send to
    // returns:  The register value, or -1 if no response
    int16_t query(uint16_t reg, uint16_t param = 0, uint8_t port = 0);
    int16_t read(uint16_t addr, uint16_t param = 0, uint8_t port = 0);

    // Control something (BLOCKING)
    // reg:      The control address
    // value:    The value to use for controlling (eg. disable/enable)
    // port:     The hub port to send to
    // returns:  true if control was successfully activated
    bool control(uint16_t reg, uint16_t value, uint8_t port = 0);
    bool write(uint16_t addr, uint16_t value, uint8_t port = 0);


    // Scan for a device attached to the specified port
    // May use cached value in scan_ports()
    // port:     The port to scan, 0-10 (root: 0)
    // returns:  The type of device that is attached
    DeviceType scan(uint8_t port = 0);

    // Return the port that a device is attached to.
    // May call scan_ports() and cache the result.
    // dtype:    The device type to look for
    // returns:  The port number, or -1 if not found.
    int8_t find_device(DeviceType dtype);

    // Read a status or log packet
    bool read_status(uint8_t* resp_out, size_t size, uint8_t slot=1, uint8_t port = 0);
    bool read_log(uint8_t* resp_out, size_t size, uint8_t port = 0);

private:
    int timeout;
    DeviceType devices[NUM_PORTS];
    bool devices_scanned;
};

// TODO: Unsure why I can only compile this by including it here...
#include "MateControllerDevice.h"

#endif /* MATE_CONTROLLER_PROTOCOL_H */