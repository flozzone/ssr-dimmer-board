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