libxcomfort
===========

libxcomfort is an open source library for controlling Moeller / Eaton's xComfort devices through the CKOZ-00/03 USB dongle. Currently RS232 is not implemented, only the USB protocol.
It can currently be used for:
* Getting/setting the dim value.
* Starting/stopping gradual up/down dimming
* Turning actuators/dimmers hard on/off.
* Some other commands like reporting temperatures/measurements have been partially reverse engineered but not implemented

The main library code is located under "CKOZ-00_03" and can be used to control the CKOZ/00/03 communication interface. This interface is used for integrating Moeller systems with for instance a computer program.
Note that the devices to control need to be paired to the comm interface using the _programming_ interface and the MRF program from Moeller.

There also exists some code for controlling devices directly using the CRSZ-00/01 _programming_ interface, that is actually used for programming devices. Working RS232 packet for setting a dimmer actuator to a desired level has been implemented for this device.

All the code is GPL licensed.

Software requirements
* libusb 1
* libmicrohttpd

Concepts
* Datapoint: This is an integer assigned to a particular device association when programming the interface using CRSZ-00/01
* Level: Goes from 0-100

Tips
* When programming the interface using CRSZ-00/01, use standard programming for each button, but set the dim range to 0%-100% so that you can dim the whole range.
