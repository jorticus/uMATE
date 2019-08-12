# μMATE

An Arduino library for the Outback MATE protocol.

Also see [pyMATE](https://github.com/jorticus/pymate), my Python implementation of the MATE protocol.

You will need a simple adapter circuit and a TTL serial port. For more details, see [jared.geek.nz/pymate](http://jared.geek.nz/pymate)

You will also need my [9-bit HW Serial library](https://github.com/jorticus/Arduino-Serial9b), or your own implementation. Your device MUST have at least 2 hardware serial ports! The Sparkfun Pro Micro works, since it has 1 USB serial port and 1 HW serial port. The Arduino Uno will NOT work as it only has 1 port!

## Installation

Simply clone to your Arduino libraries folder (eg. Documents\Arduino\libraries\uMATE)

Several examples are included with this library, and were built for the Sparkfun Pro Micro:

## Bridge.ino

This bridges a MATEnet bus to a host computer via USB.

Packets are wrapped into an HLDC-encoded frame, and can be processed by my pyMate library.
This provides a more robust method for interfacing with the network, since Linux/Windows 
struggles with 9-bit data.

## MXEmulator.ino

This emulates an Outback MX Charge Controller, with a hard-coded status packet.

## DeviceEmulator.ino

This emulates an Outback Hub with 3 attached devices on ports 1..3 (MX, FX, FlexnetDC).

Each child device spits out a hard-coded example status packet.

You should be able to plug a MATE directly to the arduino, with appropriate level conversion (the MATE expects 0V/24V logic), or you can connect it to your computer via a USB<>UART adapter and communicate via [pyMATE](https://github.com/jorticus/pymate).

## DeviceScan.ino

This scans the mate bus for attached devices.

You can connect this with the DeviceEmulator example above, or with a real Outback MATE network with attached devices (with appropriate level conversion!)

## Footnotes

Pull-requests are welcome.

The Outback bus operates at your battery's supply voltage (which could be 24 or 48V), so you MUST have appropriate level conversion AND isolation, or you risk damage to your Arduino and/or computer! Use this library at your own risk!

