# ssr-dimmer-board

Rebuild of https://github.com/fleshgordo/diskohedron using recent 
SSRs with source code for Arduino microcontroller.

## Hardware

* Schematic: https://github.com/flozzone/ssr-dimmer-board/blob/master/hw/schematic.pdf
* Layout: https://github.com/flozzone/ssr-dimmer-board/blob/master/hw/layout.pdf

## Building

    # prepare build framework
    cd src
    ./configure
    
    # building arduino v3.1 firmware
    cd bin-firmware
    make
    cd ..

    # building client library
    cd bin-client
    make
    cd ..
    
## Uploading

After having run through the building process you can upload the firmware
to the arduino. Be sure to attach the arduino with an USB cable to the 
workstation (It should be recognized as /dev/ttyUSB0).

    cd src/bin-firmware
    make upload
    
## Pin-Mapping

| Pin name 	| Description      	| Type (AVR side) 	| Arduino Pin   	| AVR Pin 	|
|----------	|------------------	|-----------------	|---------------	|---------	|
| ZC       	| Zero cross       	| INPUT           	| digital pin 2 	| PD2     	|
| CH1      	| Channel1 control 	| OUTPUT          	| digital pin 4 	| PD4     	|
| CH2      	| Channel2 control 	| OUTPUT          	| digital pin 5 	| PD5     	|
| CH3      	| Channel3 control 	| OUTPUT          	| digital pin 6 	| PD6     	|
| CH4      	| Channel4 control 	| OUTPUT          	| digital pin 7 	| PD7     	|
| CH5      	| Channel5 control 	| OUTPUT          	| digital pin 8 	| PB0     	|
| GND      	| Ground           	| POWER           	| GND           	| GND     	|

## Serial Protocol

The serial protocol consists of one single 5 byte message, which is described
in the following struct message. 

For the value argument of the message, currently only values between 0 and 16
are allowed.

    struct message {
      byte magic1 = 0xF6;
      byte magic2 = 0x6F;
      enum e_fire_type fire_type;
      enum e_channels channel_nr;
      byte value;
    };

    enum e_fire_type {
        NO_FIRE,
        PHASE_TRAILING_EDGE,
        PHASE_LEADING_EDGE,
        FULL_WAVE_BURST,
        HALF_WAVE_BURST
    };

    enum e_channels {
        CHANNEL1 = 0,
        CHANNEL2,
        CHANNEL3,
        CHANNEL4,
        CHANNEL5
    };

