#ifndef MATE_CONTROLLER_DEVICE_H
#define MATE_CONTROLLER_DEVICE_H

#include <time.h>

enum class CommonRegisters : uint16_t {
    DeviceId = 0,
    Revision = 0x0001,
    RevA = 0x0002,
    RevB = 0x0003,
    RevC = 0x0004,

    GetBatteryTemperature = 0x4000,
    SetBatteryTemperature = 0x4001,
    Time = 0x4004,
    Date = 0x4005
};

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
    uint16_t read(uint16_t reg, uint16_t param = 0) {
        return query(reg, param);
    }

    // Control something (BLOCKING)
    // reg:      The control address
    // value:    The value to use for controlling (eg. disable/enable)
    void control(uint16_t reg, uint16_t value) {
        protocol.control(reg, value, this->m_port);
    }
    void write(uint16_t reg, uint16_t value) {
        control(reg, value);
    }

    // Read the revision from the target device
    revision_t get_revision() {
        if (m_dtype == DeviceType::Fx) {
            // FX devices use an alternate method for retrieving revision
            revision_t rev = {0};
            rev.c = query((uint16_t)CommonRegisters::Revision);
            return rev;
        }
        else {
            revision_t rev;
            rev.a = query((uint16_t)CommonRegisters::RevA, 0);
            rev.b = query((uint16_t)CommonRegisters::RevB, 0);
            rev.c = query((uint16_t)CommonRegisters::RevC, 0);
            return rev;
        }
    }

    bool read_status(uint8_t* resp_out, size_t size, uint8_t slot=1) {
        return protocol.read_status(resp_out, size, slot, this->m_port);
    }
    bool read_log(uint8_t* resp_out, size_t size) {
        return protocol.read_log(resp_out, size, this->m_port);
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

public: // Properties
    uint16_t get_battery_temperature() {
        return read((uint16_t)CommonRegisters::GetBatteryTemperature);
    }

    void update_battery_temperature(uint16_t temp) {
        write((uint16_t)CommonRegisters::SetBatteryTemperature, temp);
    }

    void update_time(struct tm * ts)
    {
        uint16_t time = (
            ((ts->tm_hour & 0x1F) << 11) |
            ((ts->tm_min & 0x3F) << 5) |
            ((ts->tm_sec & 0x1F) >> 1)
        );
        uint16_t date = (
            (((ts->tm_year-2000) & 0x7F) << 9) |
            ((ts->tm_mon & 0x0F) << 5) |
            (ts->tm_mday & 0x1F)
        );
        write((uint16_t)CommonRegisters::Time, time);
        write((uint16_t)CommonRegisters::Date, date);
    }

protected:
    MateControllerProtocol& protocol;
    DeviceType m_dtype;
    int8_t m_port;
    bool m_isOpen;
};

#endif /* MATE_CONTROLLER_DEVICE_H */