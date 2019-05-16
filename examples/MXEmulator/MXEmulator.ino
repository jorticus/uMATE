// Example that emulates an Outback MX charger device
//
// You can use this to poke values at an Outback MATE controller
// to see how each byte is displayed on the LCD, and how
// the MATE controller interacts with devices.
//
// You can also use this to implement your own MX-compatible
// charger interface, though you're quite restricted in what
// you can do with this protocol...
//

#include <uMate.h>
#include <Serial9b.h>

MateDevice mate_bus(Serial9b1, &Serial); // (HardwareSerial9b, Debug Serial)

uint16_t query(uint16_t addr);
bool control(packet_t& packet);
void status(packet_t& packet);
void log(packet_t& packet);

void setup() {
    Serial.begin(9600);
    mate_bus.begin();
    pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
    uint8_t dest_port;
    packet_t packet;
    response_t response;

    if (mate_bus.recv_packet(&dest_port, &packet))
    {
        digitalWrite(LED_BUILTIN, HIGH);

        Serial.println("Packet received"); 
        Serial.print("  port:  "); Serial.println(dest_port);
        Serial.print("  type:  "); Serial.print(packet.type, 16);
        switch (packet.type) {
            case PacketType::Control: Serial.print(" (control)"); break;
            case PacketType::Query:   Serial.print(" (query)"); break;
            case PacketType::Status:  Serial.print(" (status)"); break;
            case PacketType::Log:     Serial.print(" (log)"); break;
        }
        Serial.println();        
        Serial.print("  addr:  "); Serial.println(packet.addr, 16);
        Serial.print("  param: "); Serial.println(packet.param, 16);

        response.value = 0x0000;

        switch (packet.type) {
            case PacketType::Query:
                response.value = query(packet.addr);
                // Always respond, even if we can't handle this address
                mate_bus.send_response(0x03, &response);
                break;

            case PacketType::Control:
                control(packet);
                // Always respond, even if we didn't do anything
                mate_bus.send_response(0x03, &response);
                break;

            case PacketType::Status:
                status(packet);
                break;

            case PacketType::Log:
                log(packet);
                break;
        }

        digitalWrite(LED_BUILTIN, LOW);
    }
}

uint16_t query(uint16_t addr)
{
    switch (addr) {
        // Scan
        case 0:
            Serial.println("SCAN QUERY");
            // NOTE: I've occasionally seen the upper byte of this return something non-zero (from MX/FX devices).
            // Unsure what it means, so just leave it 00 here and return the device type in the lower byte...
            return (uint16_t)DeviceType::Mx;

        // Revision (1.2.3)
        case 2: return 1;
        case 3: return 2;
        case 4: return 3;

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

    return 0;
}

bool control(packet_t& packet)
{
    return false;
}

void status(packet_t& packet)
{
    uint8_t status_data[13] = {
        0x81, 0x80, 0x82, 0x00, 0x00, 0x3F, 0x02, 0x01, 
        0xF0, 0x03, 0xE7, 0x27, 0x0F 
    };
    mate_bus.send_data(0, status_data, sizeof(status_data));
}

void log(packet_t& packet)
{
    uint8_t day = packet.param;
    uint8_t log_data[] = { 
        0x02, 0xFF, 0x17, 0x01, 0x16, 0x3C, 0x00, 0x01, 
        0x01, 0x40, 0x00, 0x10, 0x10, day
    };
    mate_bus.send_data(0, log_data, sizeof(log_data));
}
