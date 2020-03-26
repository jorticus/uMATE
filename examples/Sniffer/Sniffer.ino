// Example that can intercept traffic between two Outback devices
//
// Serial0: PC connection (log output)
// Serial1: FX/MX/DC Device (Bus B)
// Serial2: MATE (Bus A)

//#include <uMate.h>
#include <Serial9b.h>

#define BUS_A Serial9b2 // MATE
#define BUS_B Serial9b1 // Device

void setup() {
    Serial.begin(115200);

    Serial9b1.begin(9600);
    Serial9b2.begin(9600);
}

typedef enum {
    Idle,
    BusA,
    BusB
} bus_t;

void loop() {
    
    static bus_t bus = Idle;
    char msg[16];
    
    //uint8_t port;
    uint8_t buffer[32];

    // MATE -> Device
    if (BUS_A.available()) {
        uint16_t b = BUS_A.read9b();

        if ((bus != BusA) || (b & BIT9)) {
            Serial.print("\nA: ");
        }
        bus = BusA;

        snprintf(msg, sizeof(msg), "%.2x ", b); // 9-bit data
        Serial.print(msg);

        BUS_B.write9b(b);
    }

    // Device -> MATE
    if (BUS_B.available()) {
        uint16_t b = BUS_B.read9b();

        if ((bus != BusB) || (b & BIT9)) {
            Serial.print("\nB: ");
        }
        bus = BusB;

        snprintf(msg, sizeof(msg), "%.2x ", b); // 9-bit data
        Serial.print(msg);

        BUS_A.write9b(b);
    }
}