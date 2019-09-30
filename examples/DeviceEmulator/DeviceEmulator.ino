// Example that emulates Outback devices.
//
// You can use this to poke values at an Outback MATE controller
// to see how each byte is displayed on the LCD, and how
// the MATE controller interacts with devices.
//
// You can also use this to implement your own Outback-compatible
// interface, though you're quite restricted in what you can 
// do with this protocol...
//

#include <uMate.h>
#include <Serial9b.h>

/*
    The EmulatorDevice represents an emulated device that can respond 
    to commands on the MATE bus.

    You should subclass this with an implementation that returns the correct
    status/query/control responses.
*/
class EmulatorDevice {
public:
    EmulatorDevice(MateDeviceProtocol& bus, DeviceType devType) : 
        bus(bus), devType(devType)
    { }

    void process() {
        uint8_t     dest_port;
        packet_t    packet;

        if (bus.recv_packet(&dest_port, &packet))
        {
            Serial.println("Packet received"); 
            Serial.print("  port:  "); Serial.println(dest_port);
            Serial.print("  type:  "); Serial.print(packet.type, 16);
            switch (packet.type) {
                case PacketType::Increment: Serial.print(" (inc/en)"); break;
                case PacketType::Decrement: Serial.print(" (dec/dis)"); break;
                case PacketType::Write:   Serial.print(" (write)"); break;
                case PacketType::Read:    Serial.print(" (read)"); break;
                case PacketType::Status:  Serial.print(" (status)"); break;
                case PacketType::Log:     Serial.print(" (log)"); break;
            }
            Serial.println();        
            Serial.print("  addr:  "); Serial.println(packet.addr, 16);
            Serial.print("  param: "); Serial.println(packet.param, 16);

            packet_received(dest_port, packet);
        }
    }

    virtual void packet_received(uint8_t dest_port, packet_t& packet) {
        response_t  response;
        response.value = 0x0000;

        switch (packet.type) {
            case PacketType::Read:
                response.value = query(packet.addr);
                // Always respond, even if we can't handle this address
                bus.send_response(0x03, &response);
                break;

            case PacketType::Write:
                control(packet.addr, packet.param);
                // Always respond, even if we didn't do anything
                bus.send_response(0x03, &response);
                break;

            case PacketType::Status:
                status(packet.addr);
                break;

            case PacketType::Log:
                log(packet.param);
                break;
        }
    }

public:
    virtual uint32_t revision() {
        return 0x00030201; // 1.2.3
    }

    virtual uint16_t query(uint16_t addr) {
        switch (addr) {
            // NOTE: I've occasionally seen the upper byte of this return something non-zero (from MX/FX devices).
            // Unsure what it means, so just leave it 00 here and return the device type in the lower byte...
            case 0: return (uint16_t)devType;

            // Revision
            case 2: return (revision() & 0xFF);
            case 3: return ((revision() >> 8) & 0xFF);
            case 4: return ((revision() >> 16) & 0xFF);
        }
    };
    virtual void control(uint16_t addr, uint16_t value) { };
    virtual void status(uint16_t addr) { };
    virtual void log(uint16_t day) { }

protected:
    MateDeviceProtocol& bus;
    DeviceType devType;
};

/*
    Outback MX Charge Controller emulator device

    This just responds with some statically-defined values that should help
    you figure out how the MATE interprets specific responses.
*/
class MxEmulatorDevice : public EmulatorDevice {
public:
    MxEmulatorDevice(MateDeviceProtocol& bus) 
        : EmulatorDevice(bus, DeviceType::Mx)
    { }

protected:
    virtual uint16_t query(uint16_t addr) {
        switch (addr) {
            ///// STATUS/CC/METER /////
            // Charger Watts (1)
            case 0x016A: return 44;
            // Charger kWh (1/10)
            case 0x01EA: return 55;
            // Charger amps DC (-128..127, 0Amps=128)
            case 0x01C7: return 128; // 0 amps
            // Battery voltage (1/10)
            case 0x0008: return 77;
            // Panel voltage (1)
            case 0x01C6: return 88;
            
            ///// STATUS/CC/MODE /////
            // Mode (0..4)
            case 0x01C8: return 4; // Equalize
            // Aux Relay Mode / Aux Relay State
            case 0x01C9:
                // bit 7: relay state
                // bit 6..0: relay mode
                //   0: Float
                //   1: Diversion: Relay
                //   2: Diversion: Solid St
                //   3: Low batt disconnect
                //   4: Remote
                //   5: Vent fan
                //   6: PV Trigger
                //   7: Error output
                //   8: Night light
                return 0x86; // Relay on, PV trigger

            ///// STATUS/CC/STAT /////
            // Maximum battery (1/10)
            case 0x000F: return 111;
            // VOC (1/10)
            case 0x0010: return 112;
            // Max VOC (1/10)
            case 0x0012: return 133;
            // Total kWH DC (1)
            case 0x0013: return 144;
            // Total kAH (1/10)
            case 0x0014: return 155;
            // Max wattage (1)
            case 0x0015: return 166;

            ///// STATUS/CC/SETPT /////
            // Absorb (1/10)
            case 0x0170: return 177;
            // Float (1/10)
            case 0x0172: return 188;
        }

        // Handle devicetype/revision
        return EmulatorDevice::query(addr);
    }

    virtual void control(uint16_t addr, uint16_t value)
    { 
        // TODO
    }

    virtual void status(uint16_t addr) {
        uint8_t status_data[13] = {
            0x81, 0x80, 0x82, 0x00, 0x00, 0x3F, 0x02, 0x01, 
            0xF0, 0x03, 0xE7, 0x27, 0x0F 
        };
        bus.send_data(0, status_data, sizeof(status_data));
    }

    virtual void log(uint16_t day) {
        uint8_t log_data[] = { 
            0x02, 0xFF, 0x17, 0x01, 0x16, 0x3C, 0x00, 0x01, 
            0x01, 0x40, 0x00, 0x10, 0x10, (uint8_t)day
        };
        bus.send_data(0, log_data, sizeof(log_data));
    }
};

/*
    Emulates an FX inverter, outputting dummy data so 
    we can see how the MATE unit responds
*/
class FxEmulatorDevice : public EmulatorDevice {
public:
    FxEmulatorDevice(MateDeviceProtocol& bus) 
        : EmulatorDevice(bus, DeviceType::Fx)
    { }

protected:
    virtual uint16_t query(uint16_t addr) {
        switch (addr) {
            ///// STATUS/FX/MODE /////
            // Inverter control
            case 0x003D: return 1; // 0: OFF, 1: Search, 2: ON
            // AC In control
            case 0x003A: return 1; // 0: Drop, 1: Use
            // Charge control
            case 0x003C: return 2; // 0: Off, 1: Auto, 2: On
            // Aux control
            case 0x005A: return 1; // 0: Off, 1: Auto, 2: On
            // EQ enabled
            case 0x0038: return 0; // TODO: This doesn't seem to change the value

            ///// STATUS/FX/METER /////
            // Output voltage (VAC) (*2)
            case 0x002D: return 11; // 22V
            // Input voltage (VAC) (*2)
            case 0x002C: return 22;  // 44V
            // Inverter current (AAC) (/2)
            case 0x006D: return 33;  // 16.5A
            // Charger current (AAC) (/2)
            case 0x006A: return 44;  // 22.0A
            // Input current (AAC) (/2)
            case 0x006C: return 55;  // 27.5A
            // Sell current (AAC) (/2)
            case 0x006B: return 66;  // 33.0A

            ////////// STATUS/FX/BATT //////////
            // Battery actual (VDC)
            case 0x0019: return 77;
            // Battery temp compensated (VDC)
            case 0x0016: return 88;
            // Absorb setpoint (VDC)
            case 0x000B: return 99;
            // Absorb time remaining (Hours)
            case 0x0070: return 111;
            // Float setpoint (VDC)
            case 0x000A: return 122;
            // Float time remaining (Hours)
            case 0x006E: return 133;
            // Re-float setpoint (VDC)
            case 0x000D: return 144;
            // Equalize setpoint (VDC)
            case 0x000C: return 155;
            // Equalize time remaining (Hours)
            case 0x0071: return 166;
            // Battery temp (Not in degree C/F)
            case 0x0032: return 177;
            // Warnings > Airtemp (0-255)
            case 0x0033: return 200;
            // Warnings > FET temp
            case 0x0034: return 211;
            // Warnings > Cap temp
            case 0x0035: return 222;

            ////////// STATUS/FX //////////
            // Errors
            case 0x0039: return 0xFF;
            // Warnings
            case 0x0059: return 0xFF;

            // Disconn
            case 0x0084: return 0xFF;
            // Sell
            case 0x008F: return 0xFF;  // Stop sell reason
        }

        // Handle devicetype/revision
        return EmulatorDevice::query(addr);
    }

    virtual void status(uint16_t addr) {
        uint8_t status_data[13] = {
            0x02, 0x28, 0x0A, 0x02, 0x01, 0x0A, 0x00, 0x64, 
            0x00, 0x00, 0xDC, 0x14, 0x0A
        };
        bus.send_data(0, status_data, sizeof(status_data));
    }
};

class FlexNetDcDevice : public EmulatorDevice {
public:
    FlexNetDcDevice(MateDeviceProtocol& bus) 
        : EmulatorDevice(bus, DeviceType::FlexNetDc)
    { }

protected:
    virtual uint16_t query(uint16_t addr) {
        // TODO

        // Handle devicetype/revision
        return EmulatorDevice::query(addr);
    }

    virtual void status(uint16_t addr) {
        // There are two status packets that form the flexnetDC status
        switch (addr) {
            case 1: {
                uint8_t status_data[13] = {
                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
                    0x00, 0x00, 0x00, 0x00, 0x00
                };
                bus.send_data(0, status_data, sizeof(status_data));
                break;
            }

            case 2: {
                uint8_t status_data[13] = {
                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
                    0x00, 0x00, 0x00, 0x00, 0x00
                };
                bus.send_data(0, status_data, sizeof(status_data));
                break;
            }
        }
    }
};

/*
    This emulates an Outback hub with 0-10 devices attached to it.

    Use attach_device() to attach a device to the hub.
    Only call process() on the hub, not the attached devices.
*/
class HubEmulatorDevice : public EmulatorDevice {
public:
    HubEmulatorDevice(MateDeviceProtocol& bus) 
        : EmulatorDevice(bus, DeviceType::Hub)
    {
        for (int i = 0; i < 10; i++)
            devices[0] = nullptr;
     }

    void attach_device(uint8_t port, EmulatorDevice& device) {
        if (port <= 10) {
            devices[port] = &device;
        }
    }

protected:
    virtual void packet_received(uint8_t dest_port, packet_t& packet) {
        if (dest_port == 0) {
            // Port0 is the hub device itself
            EmulatorDevice::packet_received(dest_port, packet);
        } else if (dest_port <= 10) {
            // Port1-10 are child devices
            devices[dest_port]->packet_received(0, packet);
        }
    }

protected:
    EmulatorDevice* devices[10];
};


MateDeviceProtocolProtocol mate_bus(Serial9b1, &Serial); // (HardwareSerial9b, Debug Serial)
MxEmulatorDevice mx_device(mate_bus);
FxEmulatorDevice fx_device(mate_bus);
FlexNetDcDevice flexnet_device(mate_bus);
HubEmulatorDevice hub(mate_bus);

void setup() {
    Serial.begin(9600);
    mate_bus.begin();
    pinMode(LED_BUILTIN, OUTPUT);

    hub.attach_device(1, mx_device);
    hub.attach_device(2, fx_device);
    hub.attach_device(3, flexnet_device);
}

void loop() {
    delay(100);

    RXLED1;
    hub.process();
    RXLED0;
}
