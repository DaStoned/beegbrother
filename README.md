# beegbrother

Bee hive monitoring with ESP8266 and cheap load sensors.

## What's this?

The aim of this project is to monitor a bee hive - specifically the weight and internal temperature/humidity.

## Redistribution

Licensed under GPLv3, feel free to grab your copy.

# Hardware

- MCU: ESP8266 on a Wemos D1 Mini board

  https://wiki.wemos.cc/products:d1:d1_mini

- Load sensors: 50 kg cells in a HX711 H-bridge and 3D printed holder

  https://www.instructables.com/id/Arduino-Bathroom-Scale-With-50-Kg-Load-Cells-and-H/
  https://www.thingiverse.com/thing:3287167

- Temperature/humidity sensor: AM2302

  https://cdn-shop.adafruit.com/datasheets/Digital+humidity+and+temperature+sensor+AM2302.pdf

- Power supply: ???

# Software

## Set up the build environment

This project is built using the ESP8266 Non-OS SDK (v3.0.0) from Espressif. I'm using this easy-to-use installer to install it:

https://github.com/piersfinlayson/esp-open-sdk

Once the SDK is installed to directory $SDK_LOCATION, update your PATH to use it (assuming you chose the default standalone option):

```
export PATH="$SDK_LOCATION/xtensa-lx106-elf/bin:$PATH"
```

## Build and install the firmware

1. Copy `Makefile.local.sample` to `Makefile.local`.

2. Update the parameters in `Makefile.local`.

   Set the WiFi SSID and password.

   Set the correct flash map, mode and frequency to match your ESP8266 device (default parameters are for the Wemos D1 mini).

3. Build

  ```
  $ make
  ```

4. Flash on device

  ```
  $ make flash
  ```
# Prototype 

Initial prototype to see how the ESP8266, load sensors and temp/humidity sensor could be brought together. Arduino board not used except the breadboard.

![Early prototype](doc/img/01_early_prototype.jpg?raw=true "Early prototype")
 
A proper prototype with a hand-made scale body, 3D printed load sensor holders and a basic waterproof enclosure for the MCU board.

![Prototype overview](doc/img/02_prototype_scale.jpg?raw=true "Prototype overview")

![Prototype load sensor holder](doc/img/doc/img/03_prototype_scale_load_sensor.jpg?raw=true "Prototype load sensor holder")
