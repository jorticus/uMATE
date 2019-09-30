// Example that bridges a MATEnet bus to a host computer via USB
// This would be connected in place of an Outback MATE

#define DEVICE_ID (2)

#define PJON_INCLUDE_TSA true // ThroughSerialAsync

#include <uMate.h>
#include <Serial9b.h>
#include <PJON.h>

// MATEnet bus is connected to Hardware Serial 1 (9-bit communication)
MateControllerProtocol mate_bus(Serial9b1);

PJON<ThroughSerialAsync> pjon_bus(DEVICE_ID);

#define MATE_PACKET_LEN (sizeof(packet_t) + 1)
#define MATE_RESP_LEN   (sizeof(response_t) + 1)

void setup() {
    // This is the USB-CDC serial port, so baud doesn't actually matter
    Serial.begin(9600);

    pinMode(LED_BUILTIN, OUTPUT);

    mate_bus.begin();

    pjon_bus.set_receiver(receiver_function);
    pjon_bus.strategy.set_serial(&Serial);
    pjon_bus.begin();
}

void loop() {
    static uint16_t nexttick = 0;
    uint8_t port;
    uint8_t buffer[MATE_PACKET_LEN];

    // Packed representation of MATE response
    struct {
        uint8_t     cmd;
        response_t  resp;
    } resp_data  __attribute__ ((packed));

    // Handle PJON protocol stack
    pjon_bus.receive();
    pjon_bus.update();

    // Incoming data from MATEnet bus
    if (mate_bus.available() && mate_bus.recv_data(&port, &buffer[1], MATE_PACKET_LEN-1)) {
        buffer[0] = port;

        // Forward to USB Serial
        uint8_t target_id = 0; // Broadcast
        pjon_bus.send_packet(
            target_id,
            buffer,
            sizeof(buffer)
        );
    }
}

void receiver_function(uint8_t* payload, uint16_t length, const PJON_Packet_Info &packet_info) {
    // Forward the packet into the MATEnet bus
    if (length >= 2) {
        uint8_t port        = payload[0];
        uint8_t *packet     = &payload[1];
        mate_bus.send_data(port, packet, length-1);
    }
}