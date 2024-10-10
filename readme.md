# NMEA 2000 Fluidmonitor

<img title="Title" src="img/IMG_0270.jpeg" width=600>

## Table of contents
- [NMEA 2000 Fluidmonitor](#nmea-2000-fluidmonitor)
  - [Table of contents](#table-of-contents)
  - [Description](#description)
  - [NMEA 2000](#nmea-2000)
  - [Librarys](#librarys)
  - [Hardware](#hardware)
  - [Configuration](#configuration)
    - [System Configuration](#system-configuration)
      - [Thing name](#thing-name)
      - [AP password](#ap-password)
      - [WiFi SSID](#wifi-ssid)
      - [WiFi password](#wifi-password)
      - [AP offline mode after (minutes)](#ap-offline-mode-after-minutes)
    - [NMEA 2000 Settings](#nmea-2000-settings)
      - [Instance](#instance)
      - [SID](#sid)
    - [Tank](#tank)
      - [Fluid type](#fluid-type)
      - [Capacity](#capacity)
      - [Height](#height)
    - [Sensor](#sensor)
      - [Calibration factor](#calibration-factor)
      - [Upper dead zone](#upper-dead-zone)
      - [Lower dead zone](#lower-dead-zone)
  - [Default password](#default-password)
  - [Default IP address](#default-ip-address)
  - [Firmware Update](#firmware-update)
  - [Blinking codes](#blinking-codes)
  - [Reset](#reset)

## Description
A sensor based on a VL53L0X to determine the fill level in a tank. The VL53L0X is a Time-of-Flight (TOF) laser-ranging module. It can measure absolute distances up to 2m
This sensor can be used to determine the fill level in a tank. The sensor VL53L0X is used for this and is a Time-of-Flight (TOF) laser-ranging module. It can measure absolute distances up to 2m, setting a new benchmark in ranging performance levels.

A measurement is taken every second. The average value is determined from 60 measurements.

 The system communicates via the NMEA 2000 protocol, sending temperature values and alarms as NMEA 2000 messages over an NMEA bus. Configuration is done through a web interface, and real-time values can be viewed on a website. Additionally, there's a link on the configuration page for convenient firmware updates.

 <img src="img/IMG_0271.jpeg" width=500>

## NMEA 2000

The following PNG is send by this sensor. 

- 127505

## Librarys
- [VL53L0X](https://github.com/pololu/vl53l0x-arduino)
- [NMEA2000](https://github.com/ttlappalainen/NMEA2000)
- [NMEA200_ESP32](https://github.com/ttlappalainen/NMEA2000_esp32)
- [AsyncTCP (3.2.6) __"__](https://github.com/mathieucarbou/AsyncTCP)
- [ESPAsyncWebServer (3.3.12) __*__](https://github.com/mathieucarbou/ESPAsyncWebServer)
- [Webserial (2.0.7) __*__](https://github.com/ayushsharma82/WebSerial)
- [IotWebConf](https://github.com/minou65/IotWebConf)
- [IotWebConfAsync (1.0.2) __*__](https://github.com/minou65/IotWebConfAsync)
- [IotWebRoot](https://github.com/minou65/IotWebRoot)

__*__ new version and/or new repo

## Hardware
[Hardware description](/doc/hardware.md)

## Configuration
After the first boot, there are some values needs to be set up.
These items are maked with __*__ (star) in the list below.

### System Configuration

#### Thing name
Please change the name of the device to a name you think describes it the most. It is advised to incorporate a location here in case you are planning to set up multiple devices in the same area. You should only use english letters, and the "_" underscore character. Thus, must not use Space, dots, etc. E.g. `fresh_water` __*__

#### AP password
This password is used, when you want to access the device later on. You must provide a password with at least 8, at most 32 characters. You are free to use any characters, further more you are encouraged to pick a password at least 12 characters long containing at least 3 character classes. __*__

#### WiFi SSID
The name of the WiFi network you want the device to connect to. __*__

#### WiFi password
The password of the network above. Note, that unsecured passwords are not supported in your protection. __*__

#### AP offline mode after (minutes)
If you don’t plan to connect the sensor to a WiFi network, you don’t need to configure the two options above. If you want to disable the WiFi after a certain time, you can use this option. Specify how long the WiFi should remain enabled after turning on the sensor. Valid values are from 0 to 30 minutes. A value of 0 means that WiFi is always enabled.

### NMEA 2000 Settings

#### Instance
This should be unique at least on one device. May be best to have it unique over all devices sending this PGN. __*__

#### SID
Sequence identifier. In most cases you can use just 255 for SID. The sequence identifier field is used to tie different PGNs data together to same sampling or calculation time.

### Tank
#### Fluid type
Select the fluid type. can be one of the following
- Fuel
- Water
- Gray water
- Live well
- Oil
- Black water
- Gasoline fuel

#### Capacity
The volume in liters that can be held in the tank. __*__

#### Height
This value is used to calculate the amount of liquid in the tank. __*__

### Sensor
#### Calibration factor
You can calibrate this with a 2 point slope. Take one measurement at 50mm and another at 300mm. Write down the value that the sensor measures. Then calculate the factor according to the following formula

```
factor = (300mm - 50 mm) / ( X1 - X2)

X1 = value for the 300mm measurement
X2 = value for the 50mm measurement

```
Use the factor as your calibration factor. This 2 point slope should give you a good linear fit.

#### Upper dead zone
This value determines the dead zone at the top of the tank. If the reading is less than this value, then the tank is considered as full.

#### Lower dead zone
This value determines the dead zone at the bottom of the tank. If the measured value is greater than the tank height - lower dead zone, then the tank is considered as full.

## Default password
When not connected to an AP the default password is 123456789

## Default IP address
When in AP mode, the default IP address is 192.168.4.1

## Firmware Update
To update the firmware, navigate to the Configuration page and click on the Firmware Update link. Follow the on-screen instructions to complete the update process.

## Blinking codes
Prevoius chapters were mentioned blinking patterns, now here is a table summarize the menaning of the blink codes.

| Blinking Pattern | Meaning |
| --- | --- |
| Rapid blinking <\br>(mostly on, interrupted by short off periods) | Entered Access Point mode. This means the device creates its own WiFi network. You can connect to the device with your smartphone or WiFi capable computer. |
| Alternating on/off blinking | Trying to connect to the configured WiFi network. |
| Mostly off with occasional short flash | The device is online. |
| Mostly off with occasional long flash | The device is in offline mode |

## Reset
When CONFIG_PIN is pulled to ground on startup, the Thing will use the initial
password to buld an AP. (E.g. in case of lost password)

Reset pin is GPIO 13
