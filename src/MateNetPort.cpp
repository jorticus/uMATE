#include "uMate.h"

#define MATENET_BAUD 9600
#define RX_TIMEOUT 100 // ms

MateNetPort::MateNetPort(Stream9b& ser, Stream* debug)
  : debug(debug), ser(ser)
{
    
}

void MateNetPort::begin()
{
    //ser.begin(MATENET_BAUD);
}

void MateNetPort::send_data(uint8_t byte0, uint8_t* data, uint8_t len)
{
    if (data == nullptr || len == 0)
        return;

    // First byte has bit 9 set (port)
    ser.write9b(byte0 | BIT8);
    ser.flush();

    // Data payload
    for (int i = 0; i < len; i++) {
        ser.write9b(data[i]);
    }

    // Data checksum
    uint16_t checksum = 
        (uint16_t)byte0 + 
        calc_checksum(data, len);
    ser.write9b((checksum >> 8) & 0xFF);
    ser.write9b(checksum & 0xFF);

    if (debug) {
        debug->print("TX[");
        debug->print(byte0);
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

CommsStatus MateNetPort::recv_data(OUT uint8_t* byte0, OUT uint8_t* data, OUT uint8_t* len)
{
    // Maximum number of RX bytes
    // (SOP + 2 bytes of checksum + data[] buffer length)
    uint8_t max_rx_len = (*len) + 3; 

    uint32_t t1 = millis();

    if (!ser.available())
        return CommsStatus::NoData; // No data available yet

    while (true)
    {
        // Rollover-safe timeout check
        if (((uint32_t)millis() - t1) > RX_TIMEOUT) {
            if (debug) debug->println("RX: TIMEOUT");
            return CommsStatus::Timeout;
        }

        // Allow time for one byte to be received
        delayMicroseconds(800*2);
        
        int16_t b = ser.read9b();
        if (b < 0) {
            return CommsStatus::NoData; // No data available yet
        }
        
        // Looking for start of packet (bit9)
        if (rx_idx == 0) {
            if (!(b & BIT8)) {
                rx_idx = 0;
                return CommsStatus::NoStartOfPacketFound; // No start of packet found yet
            }
            b &= 0x0FF;
        }
        // Looking for data, but handle receiving a new start of packet (bit9)
        else {
            if (b & BIT8) {
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
            return CommsStatus::BufferOverrun;
        }

        rx_buffer[rx_idx++] = b;

        // NOTE: This makes the assumption that bytes are received back-to-back.
        // The Outback products actually seem to know the correct size of each packet/response,
        // but to make this library more flexible we need to find the end of a packet without knowing it's size.
        // To do this we assume that if no data is available, the packet is complete.
        if (!ser.available() || (rx_idx >= max_rx_len)) {
            if (rx_idx < 3)
                return CommsStatus::InsufficientData; // Not enough data received

            //if (debug) debug->println("RX: EOP");
            
            // The first byte is either the port or the response command / error indicator
            if (byte0 != nullptr) {
                *byte0 = rx_buffer[0];
            }

            // Checksum is always contained in the last two bytes
            uint16_t checksum = (rx_buffer[rx_idx-2] << 8) | rx_buffer[rx_idx-1];

            // Payload is between the SOP & checksum
            // rx_idx is guaranteed to have more than 3 but not more than len+3 bytes.
            *len = rx_idx - 3;
            for (int i = 0; i < (*len); i++) {
                data[i] = rx_buffer[i+1];
            }

            if (debug) {
                debug->print("RX[");
                debug->print(*byte0);
                debug->print("] ");
                for (int i = 0; i < (*len); i++) {
                    debug->print(data[i], 16);
                    debug->print(" ");
                }
            
                debug->print(" C:");
                debug->print(checksum, 16);
                debug->println();
            }

            // Validate checksum
            uint16_t checksum_actual = 
                (uint16_t)*byte0 +
                calc_checksum(data, (*len));

            rx_idx = 0;
            
            if (checksum != checksum_actual) {
                if (debug) {
                    debug->print("Invalid checksum. Exp:");
                    debug->print(checksum, 16);
                    debug->print(" Act:");
                    debug->println(checksum_actual, 16);
                }
                return CommsStatus::BadChecksum;
            }

            return CommsStatus::Success; // Packet fully received!
        }
    }
}

uint16_t MateNetPort::calc_checksum(uint8_t* data, uint8_t len)
{
    uint16_t sum = 0;
    for (int i = 0; i < len; i++) {
        sum += data[i];
    }
    return sum;
}

