#
# MATE Tap Script
# Forwards intercepted MATE communications to Wireshark via named pipe
#
# Usage:
# .\Tap.py COMxx
#
# NOTE: Currently only supported on Windows.
# Linux support should be possible with modifications to the named pipe interface.
#

from time import sleep
from serial import Serial  # pySerial
from datetime import datetime
import os
import sys
import time
import win32pipe, win32file, win32event, pywintypes, winerror # pywin32
import subprocess
import struct
import errno

# https://wiki.wireshark.org/CaptureSetup/Pipes

    
if sys.version_info < (3,):
    raise Exception('Python 3 required')

if len(sys.argv) < 2:
    print("Usage:")
    print(os.path.basename(sys.argv[0]) + " COM1")
    exit(1)

COM_PORT = sys.argv[1]
COM_BAUD = 115200

WIRESHARK_LAUNCH = True

COMBINE_CMD_RESPONSE = True

WIRESHARK_PATH = r'C:\Program Files\Wireshark\Wireshark.exe'
WIRESHARK_PIPE = r'\\.\pipe\wireshark-mate'
WIRESHARK_DLT  = 147 # DLT_USER0

LUA_SCRIPT_PATH = os.path.join(os.path.dirname(os.path.realpath(__file__)), 'mate_dissector.lua')

if not os.path.exists(LUA_SCRIPT_PATH):
    raise Exception('mate_dissector.lua not found: ' + LUA_SCRIPT_PATH)

PIPE_CONNECT_TIMEOUT = 30000 # millisec

BUS_A = 'A'
BUS_B = 'B'

END_OF_PACKET_TIMEOUT = 0.02 # seconds

def create_pipe(name):
    # WINDOWS:
    return win32pipe.CreateNamedPipe(
        name,
        win32pipe.PIPE_ACCESS_OUTBOUND | win32file.FILE_FLAG_OVERLAPPED,
        win32pipe.PIPE_TYPE_MESSAGE | win32pipe.PIPE_WAIT,
        1, 65536, 65536,
        300,
        None)
    # UNIX:
    # try:
    #     return os.mkfifo(name);
    # except FileExistsError:
    #     pass
    # except:
    #     raise

def connect_pipe(pipe, timeoutMillisec = 1000):
    # WINDOWS:
    overlapped = pywintypes.OVERLAPPED()
    overlapped.hEvent = win32event.CreateEvent(None, 0, 0, None)
    rc = win32pipe.ConnectNamedPipe(pipe, overlapped)
    if rc == winerror.ERROR_PIPE_CONNECTED:
        win32event.SetEvent(overlapped.hEvent)
    rc = win32event.WaitForSingleObject(overlapped.hEvent, timeoutMillisec)
    overlapped = None
    if rc != win32event.WAIT_OBJECT_0:
        raise TimeoutError("Timeout while waiting for pipe to connect")
    # UNIX:
    #pipe.open()

def write_pipe(pipe, buf):
    try:
        # WINDOWS:
        win32file.WriteFile(pipe, bytes(buf))
        # UNIX:
        #pipe.write(buf)
    except OSError as e:
        # SIGPIPE indicates the fifo was closed
        if e.errno == errno.SIGPIPE:
            return False
    return True
    
def send_frame(pipe, bus, data):
    # send pcap packet through the pipe
    now = datetime.now()
    timestamp = int(time.mktime(now.timetuple()))
    pcap_header = struct.pack("=iiiiB",
        timestamp,        # timestamp seconds
        now.microsecond,  # timestamp microseconds
        len(data)+1,        # number of octets of packet saved in file
        len(data)+1,        # actual length of packet
        bus
    )
    if not write_pipe(pipe, pcap_header + bytes(data)):
        return
    
def send_combined_frames(pipe, busa, busb):
    if not busa: busa = []
    if not busb: busb = []
    
    data = struct.pack("=BBB", 0x0F, len(busa), len(busb))
    data += bytes(busa)
    data += bytes(busb)

    # send pcap packet through the pipe
    now = datetime.now()
    timestamp = int(time.mktime(now.timetuple()))
    pcap_header = struct.pack("=iiii",
        timestamp,        # timestamp seconds
        now.microsecond,  # timestamp microseconds
        len(data),        # number of octets of packet saved in file
        len(data),        # actual length of packet
    )
    if not write_pipe(pipe, pcap_header + bytes(data)):
        return

def main():

    s = Serial(COM_PORT, COM_BAUD)
    try:

        if WIRESHARK_LAUNCH:
            #open Wireshark, configure pipe interface and start capture (not mandatory, you can also do this manually)
            wireshark_cmd=[
                WIRESHARK_PATH, 
                '-i'+WIRESHARK_PIPE,
                '-k',
                '-o','capture.no_interface_load:TRUE',
                '-X','lua_script:'+LUA_SCRIPT_PATH
            ]
            proc=subprocess.Popen(wireshark_cmd)

        # Create named pipe
        pipe = create_pipe(WIRESHARK_PIPE)

        try:
            print("PCAP pipe created: %s" % WIRESHARK_PIPE)
            print("Waiting for connection...")
            connect_pipe(pipe, PIPE_CONNECT_TIMEOUT)
            print("Pipe opened")

            # Send PCAP header
            buf = struct.pack("=IHHiIII",
                0xa1b2c3d4,   # magic number
                2,            # major version number
                4,            # minor version number
                0,            # GMT to local correction
                0,            # accuracy of timestamps
                65535,        # max length of captured packets, in octets
                WIRESHARK_DLT, # data link type (DLT)
            )
            if not write_pipe(pipe, buf):
                return

            s.timeout = 1.0 # seconds

            print("Serial port opened, listening for MateNET data...")
            
            prev_bus = None
            prev_packet = None

            while True:
                s.timeout = END_OF_PACKET_TIMEOUT
                try:
                    ln = s.readline()
                    if ln:
                        ln = ln.decode('ascii', 'ignore').strip()
                    if ln and ':' in ln:
                        print(ln)
                        bus, rest = ln.split(': ')
                        payload = [int(h, 16) for h in rest.split(' ')]
                       
                        
                        #data = bytes([int(bus, 16)])
                        data = bytes([])

                        if len(payload) > 2:
                            if (payload[0] & 0x100) == 0:
                                print("Invalid frame: bit9 not set!")
                                continue
                            if any([b & 0x100 for b in payload[1:]]):
                                print("Invalid frame: bit9 set in middle of frame!")
                                continue

                        for b in payload:
                            # Discard 9th bit before encoding PCAP
                            data += bytes([b & 0x0FF])

                        # for hx in rest.split(' '):
                        #     # Each word is 9-bits, so pack into 2 bytes for PCAP (16-bit word)
                        #     #word = int(hx, 16)
                        #     #b = struct.pack(">H", word)
                        #     # Discard 9th bit, assume it is clear
                        #     data += (b & 0xFF)

                        #print(' '.join('%.2x' % c for c in data))

                        # Send PCAP packet through the pipe
                        #now = datetime.now()
                        #timestamp = int(time.mktime(now.timetuple()))
                        #pcap_header = struct.pack("=IIII",
                        #    timestamp,        # timestamp seconds
                        #    now.microsecond,  # timestamp microseconds
                        #    len(data),        # number of octets of packet saved in file
                        #    len(data),        # actual length of packet
                        #)
                        #if not write_pipe(pipe, pcap_header + bytes(data)):
                        #    return
                        
                        if COMBINE_CMD_RESPONSE:
                            
                            if bus == 'A':
                                if prev_bus == 'A':
                                    # Previous frame was from the same bus, better
                                    # send this to wireshark even though it has 
                                    # no corresponding frame from bus B
                                    #send_combined_frames(pipe, prev_pkt, None)
                                    send_frame(pipe, 0xA, prev_pkt)
                                    
                                # Store packet until we receive a frame from B
                                prev_bus = 'A'
                                prev_pkt = data
                                
                            elif bus == 'B':
                                # Combine packet from A & B
                                send_combined_frames(pipe, prev_pkt, data)
                                
                                prev_pkt = None
                                prev_bus = 'B'
                                
                        else:
                            # Encode the bus identifier as hex (0xA or 0xB)
                            busid = int(bus,16)
                            send_frame(pipe, busid, data)
                            
                except ValueError as e:
                    raise
                    continue

                sleep(0.001)
        finally:
            pipe.close()
    finally:
        s.close()

if __name__ == "__main__":
    main()