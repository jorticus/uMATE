#include "uMate.h"

#define MATENET_BAUD 9600
#define RX_TIMEOUT 100 // ms

MateNetPort::MateNetPort(HardwareSerial9b& ser, Stream* debug)
  : ser(ser), debug(debug)
{
    
}

void MateNetPort::begin()
{
    ser.begin(MATENET_BAUD);
}

void MateNetPort::send_data(uint8_t port, uint8_t* data, uint8_t len)
{
    if (data == nullptr || len == 0)
        return;

    // First byte has bit 9 set (port)
    ser.write9b(port | BIT9);

    // Data payload
    for (int i = 0; i < len; i++) {
        ser.write9b(data[i]);
    }

    // Data checksum
    uint16_t checksum = 
        (uint16_t)port + 
        calc_checksum(data, len);
    ser.write9b((checksum >> 8) & 0xFF);
    ser.write9b(checksum & 0xFF);

    if (debug) {
        debug->print("TX[");
        debug->print(port);
        debug->print("] ");
        for (int i = 0; i < len; i++) {
            debug->print(data[i], 16);
            debug->print(" ");
        }
        debug->print(" C:");
        debug->print(checksum, 16);
        debug->println();
    }
}

bool MateNetPort::available()
{
    // Do we have some data to begin reading a frame?
    // NOTE: Does not guarantee that there will be a frame.
    return (ser.available());
}

bool MateNetPort::recv_data(OUT uint8_t* port, OUT uint8_t* data, uint8_t len)
{
    uint8_t rx_len = len;
    if (rx_len == 0)
        return true; // Fully received 0 bytes

    rx_len += 3; // SOP + 2 bytes of checksum

    uint32_t t1 = millis();

    while (ser.available())
    {
        // Rollover-safe timeout check
        if (((uint32_t)millis() - t1) > RX_TIMEOUT) {
            if (debug) debug->println("RX: TIMEOUT");
            return false;
        }

        int16_t b = ser.read9b();
        if (b < 0)
            return false; // No data available yet

        /*if (debug) {
            debug->print("RX: ");
            debug->print(b, 16);
            debug->print(" ");
            debug->print(rx_idx);
            debug->print(" ");
            debug->print(rx_len);
            debug->print(" ");
            debug->print((b & BIT9) ? 1 : 0);
            debug->println();
        }*/
        
        // Looking for start of packet (bit9)
        if (rx_idx == 0) {
            if (!(b & BIT9)) {
                rx_idx = 0;
                return false; // No start of packet found yet
            }
            b &= 0x0FF;
        }
        // Looking for data, but handle receiving a new start of packet (bit9)
        else {
            if (b & BIT9) {
                // Start of packet found in middle of packet, restart RX buffer.
                // This can happen if there was corruption on the bus and we missed a byte,
                // or if we're expecting more data than what was sent.
                if (debug) { debug->println("Unexpected SOP/bit9"); }
                rx_idx = 0;
                b &= 0x0FF;
            }
        }

        if (rx_idx > sizeof(rx_buffer)) {
            // Out of buffer, reset.
            rx_idx = 0;
            return false;
        }

        rx_buffer[rx_idx++] = b;

        if (rx_idx >= rx_len) { // len guaranteed to be >= 1
            if (port != nullptr) {
                *port = rx_buffer[0];
            }
            for (int i = 0; i < len; i++) {
                data[i] = rx_buffer[i+1];
            }
            uint16_t checksum = (rx_buffer[rx_idx-2] << 8) | rx_buffer[rx_idx-1];
            rx_idx = 0;

            if (debug) {
                debug->print("RX[");
                debug->print(*port);
                debug->print("] ");
                for (int i = 0; i < len; i++) {
                    debug->print(data[i], 16);
                    debug->print(" ");
                }
                debug->print(" C:");
                debug->print(checksum, 16);
                debug->println();
            }

            // Validate checksum
            uint16_t checksum_actual = 
                (uint16_t)*port +
                calc_checksum(data, len);
            
            if (checksum != checksum_actual) {
                if (debug) {
                    debug->print("Invalid checksum. Exp:");
                    debug->print(checksum, 16);
                    debug->print(" Act:");
                    debug->println(checksum_actual, 16);
                }
                return false;
            }

            return true; // Packet fully received!
        }
    }
    return false; // Have not yet received 'len' bytes
}

uint16_t MateNetPort::calc_checksum(uint8_t* data, uint8_t len)
{
    uint16_t sum = 0;
    for (int i = 0; i < len; i++) {
        sum += data[i];
    }
    return sum;
}

