// Example that bridges a MATEnet bus to a host computer via USB
// This would be connected in place of an Outback MATE

#define DEVICE_ID (2)

#define PJON_INCLUDE_TSA true // ThroughSerialAsync
#define PJON_PACKET_MAX_LENGTH 100

enum Target {
    Device = 0x0A,
    Mate   = 0x0B
} ;

#include <uMate.h>
#include <Serial9b.h>
#include <PJON.h>

// MATEnet bus is connected to Hardware Serial 1 (9-bit communication)
MateControllerProtocol mate_bus_a(Serial9b1);

#ifdef HAVE_HWSERIAL2
MateControllerProtocol mate_bus_b(Serial9b2);
#endif

PJON<ThroughSerialAsync> pjon_bus(DEVICE_ID);

#define MAX_BUFFER_LEN  (100)
#define MATE_PACKET_LEN (sizeof(packet_t) + 1)
#define MATE_RESP_LEN   (sizeof(response_t) + 1)

void setup() {
    // This is the USB-CDC serial port, so baud doesn't actually matter
    Serial.begin(1000000);

    pinMode(LED_BUILTIN, OUTPUT);

    mate_bus_a.begin();
#ifdef HAVE_HWSERIAL2
    mate_bus_b.begin();
#endif

    pjon_bus.set_receiver(receiver_function);
    pjon_bus.strategy.set_serial(&Serial);
    pjon_bus.begin();
}

void loop() {
    static uint16_t nexttick = 0;
    uint8_t byte0;
    uint8_t buffer_len = MAX_BUFFER_LEN-2; //MATE_RESP_LEN-1;
    uint8_t buffer[MAX_BUFFER_LEN];
    
    uint8_t target_id = 0; // Broadcast

    // Packed representation of MATE response
    // struct {
    //     uint8_t     cmd;
    //     response_t  resp;
    // } resp_data  __attribute__ ((packed));

    // Handle PJON protocol stack
    uint16_t status = pjon_bus.receive();
    pjon_bus.update();

    if (status == PJON_NAK) {
        pjon_bus.send_packet(0, "N", 1);
    }

    // Incoming data from Outback Device
    if (mate_bus_a.available()) {
        auto err = mate_bus_a.recv_data(&byte0, &buffer[1], &buffer_len);
        if (err == CommsStatus::Success) {
            buffer[0] = byte0;

            // Forward to USB Serial
            pjon_bus.send_packet(
                target_id,
                buffer,
                buffer_len+1
            );
        } else {
            if (err != CommsStatus::NoData) {
                buffer[0] = (uint8_t)err;
                pjon_bus.send_packet(
                    target_id,
                    buffer, 1
                );
            }
        }
    }

#ifdef HAVE_HWSERIAL2
    // Incoming data from Outback MATE
    //buffer_len = 50;
    if (mate_bus_b.available()) {
        auto err = mate_bus_b.recv_data(&byte0, &buffer[1], &buffer_len);
        if (err == CommsStatus::Success) {
            buffer[0] = byte0;

            // Forward to USB Serial
            uint8_t target_id = 0; // Broadcast
            pjon_bus.send_packet(
                target_id,
                buffer,
                buffer_len+1
            );
        } else {
            if (err != CommsStatus::NoData) {
                buffer[0] = (uint8_t)err;
                pjon_bus.send_packet(
                    target_id,
                    buffer, 1
                );
            }
        }
    }
#endif

    //pjon_bus.send_packet(0, "123456789012345678901234567890", 30);

    //uint8_t data[] = {0x0b, 0x03, 0x89, 0x84, 0x89, 0x00, 0x55, 0x3f, 0x02, 0x00, 0x15, 0x00, 0xfe, 0x02, 0xa8};
    //pjon_bus.send_packet(0, data, sizeof(data));
}

void receiver_function(uint8_t* payload, uint16_t length, const PJON_Packet_Info &packet_info) {
    // Forward the packet into the MATEnet bus
    if (length >= 3) {
        uint8_t target      = payload[0];
        uint8_t byte0       = payload[1];
        length -= 2;
        uint8_t *packet     = &payload[2];

        //pjon_bus.reply("R", 1); // ERROR

        // Send a command to an attached Outback device
        if (target == Target::Device) {
            mate_bus_a.send_data(byte0, packet, length);
            return;
        }
#ifdef HAVE_HWSERIAL2
        // Send a response to an attached Outback MATE
        else if (target == Target::Mate) {
            mate_bus_b.send_data(byte0, packet, length);
            return;
        }
#endif
    }

    pjon_bus.reply("E", 1); // ERROR
}
