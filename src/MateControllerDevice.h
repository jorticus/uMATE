#ifndef MATE_CONTROLLER_DEVICE_H
#define MATE_CONTROLLER_DEVICE_H

class MateControllerDevice {
public:
    MateControllerDevice(
        MateControllerProtocol& protocol, 
        DeviceType m_dtype
    ) : 
        protocol(protocol),
        m_dtype(m_dtype),
        m_port(-1),
        m_isOpen(false)
    { }

    // Open a device on the specified m_port.
    // returns false if the device is not present on that m_port.
    bool begin(uint8_t port) {
        this->m_port = port;
        m_isOpen = (protocol.scan(m_port) == m_dtype);
        return m_isOpen;
    }

    // Query a register and retrieve its value (BLOCKING)
    // reg:      The register address
    // param:    Optional parameter
    // returns:  The register value
    uint16_t query(uint16_t reg, uint16_t param = 0) {
        return protocol.query(reg, param, this->m_port);
    }

    // Control something (BLOCKING)
    // reg:      The control address
    // value:    The value to use for controlling (eg. disable/enable)
    void control(uint16_t reg, uint16_t value) {
        protocol.control(reg, value, this->m_port);
    }

    // Read the revision from the target device
    revision_t get_revision() {
        if (m_dtype == DeviceType::Fx) {
            // FX devices use an alternate method for retrieving revision
            revision_t rev = {0};
            rev.c = query(0x0001);
            return rev;
        }
        else {
            return protocol.get_revision(this->m_port);
        }
    }

    DeviceType deviceType() const {
        return m_dtype;
    }
    int8_t port() const {
        return m_port;
    }
    bool isConnected() const {
        return m_isOpen;
    }

protected:
    MateControllerProtocol& protocol;
    DeviceType m_dtype;
    int8_t m_port;
    bool m_isOpen;
};

#endif /* MATE_CONTROLLER_DEVICE_H */