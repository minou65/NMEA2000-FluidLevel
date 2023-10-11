# NEMA Fluidmonitor

<img title="Title" src="img/IMG_0270.jpeg" width=600>

- [NEMA Fluidmonitor](#nema-fluidmonitor)
  - [Description](#description)
  - [NMEA 2000](#nmea-2000)
  - [Librarys](#librarys)
  - [Hardware](#hardware)
  - [Configuration](#configuration)
  - [Tank](#tank)
    - [Fluid type](#fluid-type)
    - [Capacity](#capacity)
    - [Height](#height)
  - [Sensor](#sensor)
    - [Calibration factor](#calibration-factor)
    - [Upper dead zone](#upper-dead-zone)
    - [Lower dead zone](#lower-dead-zone)
  - [WiFi ](#wifi-)
    - [Default Password ](#default-password-)
    - [Default IP address ](#default-ip-address-)
    - [OTA ](#ota-)
    - [Configuration options ](#configuration-options-)
    - [Blinking codes ](#blinking-codes-)
    - [Reset ](#reset-)

## Description
This sensor can be used to determine the fill level in a tank. The sensor VL53L0X is used for this and is a Time-of-Flight (TOF) laser-ranging module. It can measure absolute distances up to 2m, setting a new benchmark in ranging performance levels.

A measurement is taken every second and the average value is calculated from 60 measurements.

## NMEA 2000

The following PNG are send by this sensor. 

- 127505

## Librarys
- VL53L0X
- prampec/IotWebConf
- NMEA2000
- NMEA200_ESP

## Hardware
[Hardware description](/doc/hardware.md)

## Configuration

## Tank
<img title="System configuration" src="img/IMG_0273.jpeg" width=400>

### Fluid type
Select the fluid type.

### Capacity
The liters that can stored in the tank.

### Height
This is used to calculate the amount of liquid in the tank.

## Sensor
<img title="System configuration" src="img/IMG_0274.jpeg" width=400>

### Calibration factor

### Upper dead zone

### Lower dead zone

## WiFi <a name="wifi"></a>
<img title="System configuration" src="img/IMG_0272.jpeg" width=400>

### Default Password <a name="wifipassword"></a>
When not connected to an AP the default password is 123456789

### Default IP address <a name="wifiipaddress"></a>
When in AP mode, the default IP address is 192.168.4.1

### OTA <a name="wifiota"></a>
OTA is enabled, use default IP address or if connected to a AP the correct address.
Port is the default port.

### Configuration options <a name="wificonfiguration"></a>
After the first boot, there are some values needs to be set up.
These items are maked with __*__ (star) in the list below.

You can set up the following values in the configuration page:

-  __Thing name__ - Please change the name of the device to
a name you think describes it the most. It is advised to
incorporate a location here in case you are planning to
set up multiple devices in the same area. You should only use
english letters, and the "_" underscore character. Thus, must not
use Space, dots, etc. E.g. `lamp_livingroom` __*__
- __AP password__ - This password is used, when you want to
access the device later on. You must provide a password with at least 8,
at most 32 characters.
You are free to use any characters, further more you are
encouraged to pick a password at least 12 characters long containing
at least 3 character classes. __*__
- __WiFi SSID__ - The name of the WiFi network you want the device
to connect to. __*__
- __WiFi password__ - The password of the network above. Note, that
unsecured passwords are not supported in your protection. __*__

### Blinking codes <a name="wifiblinkingcodes"></a>
Prevoius chapters were mentioned blinking patterns, now here is a
table summarize the menaning of the blink codes.

- __Rapid blinking__ (mostly on, interrupted by short off periods) -
Entered Access Point mode. This means the device create an own WiFi
network around it. You can connect to the device with your smartphone
(or WiFi capable computer).
- __Alternating on/off blinking__ - Trying to connect the configured
WiFi network.
- __Mostly off with occasional short flash__ - The device is online.

### Reset <a name="wifireset"></a>
When CONFIG_PIN is pulled to ground on startup, the Thing will use the initial
password to buld an AP. (E.g. in case of lost password)

Reset pin is D3 / IO17
