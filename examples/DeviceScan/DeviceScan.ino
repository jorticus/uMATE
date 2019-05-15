// Example that emulates an Outback MATE device
// The MATE is a master device and allows you to query and monitor other devices on the bus.

#include <uMate.h>
#include <Serial9b.h>

HardwareSerial9b ser(Serial1);
MateNetPort mate_bus(ser); // (HardwareSerial9b, Debug Serial)
MateController mate(mate_bus);

DeviceType devices[10] = { DeviceType::None };

void setup() {
    Serial.begin(9600);

    mate_bus.begin();
    mate.begin();

    Serial.println("Scanning for MATE devices...");
    auto root_dtype = mate.scan(0);
    if (root_dtype != DeviceType::None) {
        devices[0] = root_dtype;
        Serial.print("0: ");
        Serial.print(root_dtype);
        Serial.println();

        if (root_dtype == DeviceType::Hub) {
            for (int i = 1; i < 10; i++) {
                auto dtype = mate.scan(i);
                if (dtype != DeviceType::None) {
                    devices[i] = dtype;
                    Serial.print(i);
                    Serial.print(": ");
                    Serial.print(dtype);
                    Serial.println();
                }
            }
        }

    } else {
        Serial.println("No MATE devices found");
    }
}

void loop() {
    // TODO: Read device revision
}
