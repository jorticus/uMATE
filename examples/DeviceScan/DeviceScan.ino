// Example that emulates an Outback MATE device
// The MATE is a master device and allows you to query and monitor other devices on the bus.

#include <uMate.h>
#include <Serial9b.h>

//MateControllerProtocol mate_bus(Serial9b1, &Serial); // (HardwareSerial9b, Debug Serial)
MateControllerProtocol mate_bus(Serial9b1);

DeviceType devices[10] = { DeviceType::None };

bool devicesFound = false;

const char* dtypes[] = {
    "None",
    "Hub",
    "FX",
    "MX",
    "FlexNetDC"
};

void print_dtype(DeviceType dtype) {
    if (dtype < DeviceType::MaxDevices) {
        Serial.print(dtypes[dtype]);
        Serial.print(" ");
    } else {
        Serial.print(dtype);
    }
}

void setup() {
    Serial.begin(9600);

    mate_bus.begin();

    delay(4000);
    Serial.println("Ready!");
}

void loop() {
    // TODO: Read device revision
    if (!devicesFound) {
        Serial.println("Scanning for MATE devices...");
        auto root_dtype = mate_bus.scan(0);
        if (root_dtype != DeviceType::None) {
            devices[0] = root_dtype;
            Serial.print("0: ");
            print_dtype(root_dtype);
            Serial.println();

            if (root_dtype == DeviceType::Hub) {
                for (int i = 1; i < 10; i++) {
                    auto dtype = mate_bus.scan(i);
                    if (dtype != DeviceType::None) {
                        devices[i] = dtype;
                        Serial.print(i);
                        Serial.print(": ");
                        print_dtype(dtype);
                        Serial.println();
                    }
                }
            }

            Serial.println("Done!");
            devicesFound = true;
        } else {
            Serial.println("No MATE devices found");
        }

        delay(1000);
    }
    else {
        // TODO: Query MX status
    }
}
