// Example that emulates an Outback MATE device
// The MATE is a master device and allows you to query and monitor other devices on the bus.

#include <uMate.h>
#include <Serial9b.h>

class MxDeviceController : public MateControllerDevice {
public:
    MxDeviceController(MateControllerProtocol& protocol) 
        : MateControllerDevice(protocol, DeviceType::Mx)
    { }
};

class DcDeviceController : public MateControllerDevice {
public:
    DcDeviceController(MateControllerProtocol& protocol) 
        : MateControllerDevice(protocol, DeviceType::FlexNetDc)
    { }
};

MateControllerProtocol mate_bus(Serial9b1, &Serial); // (HardwareSerial9b, Debug Serial)
//MateControllerProtocol mate_bus(Serial9b1);

MxDeviceController mx_device(mate_bus);
DcDeviceController dc_device(mate_bus);

DeviceType devices[10] = { DeviceType::None };

bool devicesFound = false;
bool mx_device_available = false;
bool dc_device_available = false;

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
        mate_bus.scan_ports();

        if (mate_bus.scan(0) != DeviceType::None) {

            for (int i = 0; i < NUM_PORTS; i++) {
                DeviceType dtype = mate_bus.scan(i);
                if (dtype != DeviceType::None) {
                    devices[i] = dtype;
                    Serial.print(i);
                    Serial.print(": ");
                    print_dtype(dtype);
                    Serial.println();
                }
            }

            Serial.println("Done!");
            devicesFound = true;
        } else {
            Serial.println("No MATE devices found");
        }

        delay(1000);

        Serial.println();
        Serial.println("Scanning for MX device...");
        int8_t mx_port = mate_bus.find_device(DeviceType::Mx);
        if (mx_port == -1 || !mx_device.begin(mx_port)) {
            Serial.println("MX device not found!");
            //devicesFound = false;
        } else {
            // TODO: Revision is displayed wrong.
            auto rev = mx_device.get_revision();
            Serial.print("MX device found on port ");
            Serial.println(mx_port);
            Serial.print("Rev: ");
            Serial.print(rev.a); Serial.print(".");
            Serial.print(rev.b); Serial.print(".");
            Serial.print(rev.c);
            Serial.println();
            mx_device_available = true;
        }

        Serial.println();
        Serial.println("Scanning for FLEXnet DC...");
        int8_t dc_port = mate_bus.find_device(DeviceType::FlexNetDc);
        if (dc_port == -1 || !dc_device.begin(dc_port)) {
            Serial.println("DC device not found!");
        } else {
            //auto rev = dc_device.get_revision();
            Serial.print("DC device found on port ");
            Serial.println(dc_port);
            dc_device_available = true;
        }

        if (!mx_device_available || !dc_device_available) {
            devicesFound = false;
        }
    }
    else {
        if (mx_device_available) {
            //mx_device.query();
        }

        if (dc_device_available) {
            //dc_device.query();
        }
    }
}
