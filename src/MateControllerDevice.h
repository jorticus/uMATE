#ifndef MATE_CONTROLLER_DEVICE_H
#define MATE_CONTROLLER_DEVICE_H

class MateControllerDevice {
public:
    MateControllerDevice(
        MateControllerProtocol& protocol, 
        DeviceType dtype
    ) : 
        protocol(protocol),
        dtype(dtype),
        port(-1),
        is_open(false)
    { }

    // Open a device on the specified port.
    // returns false if the device is not present on that port.
    bool begin(uint8_t port) {
        this->port = port;
        is_open = (protocol.scan(port) == dtype);
        return is_open;
    }

    // Query a register and retrieve its value (BLOCKING)
    // reg:      The register address
    // param:    Optional parameter
    // returns:  The register value
    uint16_t query(uint16_t reg, uint16_t param = 0) {
        return protocol.query(reg, param, this->port);
    }

    // Control something (BLOCKING)
    // reg:      The control address
    // value:    The value to use for controlling (eg. disable/enable)
    void control(uint16_t reg, uint16_t value) {
        protocol.control(reg, value, this->port);
    }

    // Read the revision from the target device
    revision_t get_revision() {
        if (dtype == DeviceType::Fx) {
            // FX devices use an alternate method for retrieving revision
            revision_t rev = {0};
            rev.c = query(0x0001);
            return rev;
        }
        else {
            return protocol.get_revision(this->port);
        }
    }

    DeviceType deviceType() const {
        return dtype;
    }

protected:
    MateControllerProtocol& protocol;
    DeviceType dtype;
    int8_t port;
    bool is_open;
};

#endif /* MATE_CONTROLLER_DEVICE_H */