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