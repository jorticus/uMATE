// Example that emulates an Outback MX charger device

#include <uMate.h>
#include <Serial9b.h>

MateDevice mate_bus(Serial9b1, &Serial); // (HardwareSerial9b, Debug Serial)

bool query(uint16_t addr, OUT uint16_t& value);
bool control(packet_t& packet);
void status(packet_t& packet);
void log(packet_t& packet);

void setup() {
    Serial.begin(9600);
    mate_bus.begin();
}

void loop() {
    uint8_t dest_port;
    packet_t packet;
    response_t response;

    if (mate_bus.recv_packet(&dest_port, &packet))
    {
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
                query(packet.addr, OUT &response.value);
                // Always respond, even if we didn't do anything
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

    }
}

bool query(uint16_t addr, OUT uint16_t* value)
{
    switch (addr) {
        // Scan
        case 0x0000:
            Serial.println("SCAN QUERY");
            // NOTE: I've occasionally seen the upper byte of this return something non-zero (from MX/FX devices).
            // Unsure what it means, so just leave it 00 here...
            *value = (uint16_t)DeviceType::Mx;
            return true;

        // Revision (2.3.4)
        case 0x0002:
            *value = 1;
            return true;
        case 0x0003:
            *value = 2;
            return true;
        case 0x0004:
            *value = 3;
            return true;
    }

    return false;
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
