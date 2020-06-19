# ESP8266 BASED WIFI LED DRIVER

ESP8266 based WiFi LED driver is a high capacity led driver, which can be operated via MQTT or with a physical button. It has 3-channels capable of delivering 1A current per channel. 

> **WARNING:**  **!!!  USE THIS DEVICE AT YOUR OWN RISK! I AM NOT AN EXPERT ON ELECTRONICS MALFUNCTIONS CAN HAPPEN WITH THE BOARD (fire hazard etc.) Always unplug it, if you do not use !!!**


# Features

- Controllable via WIFI + MQTT
- 3 channels, 1A current per channel
- Lights are dimmable
- Can be set up in a browser, no need to hard-code WiFi credentials
- Physical switch to turn the LED strips on and off
- MQTT Status feedback
- Openhab compatible
- Wired and OTA firmware update 

# Hardware

## Parts list

- 10K resistor
- ESP12
- LM2596SX-3.3/NOPB
- 560uF capacitor
- Inductor
- 680uF capacitor
- SS34-E3/57T schottky diode
- Header-Male-2.54_1x8
- Terminal Block 
- 100nF capacitor (optional)
- IRLB8743PBF transistor
- DC-005-5A-3.0 socket

Detailed parts list in the pcb folder BOM file.

## Accessories needed

- Power suppply 12V 5A min. rated supply with 3.0 mm x 5.5 mm plug
- USB-TTL CP2102 (to upload firmware to the device) 
- Dupont cables (5 pieces)
- Momentary switch (optional)
- analog LED strip (3, 1A max)

## Assembly

- Order the PCB (gerber file in the pcb folder)
- Order parts 
- Solder the parts to the PCB. Beware the order of the soldering, if you are using soldering iron


The final device should look like this:



# Firmware

## Required software environment 

- Arduino IDE to flash the device before the first usage
- MQTT client to send commands to the unit via WiFi (it can be an app, or software running on a computer)
- MQTT broker on running on the same network as the LED driver  

## Firmware flash

- To upload the firmware you need to install Arduino IDE
- You need to install the ESP board in Arduino IDE (instructions [here](https://randomnerdtutorials.com/how-to-install-esp8266-board-arduino-ide/))
- Install IotWebConf and MQTT library for Arduino by Joel Gaehwiler (guide to install Arduino libraries [here](https://www.arduino.cc/en/guide/libraries))
- Open the firmware in Arduino IDE (located in the firmware folder: wifi_ledstrip.ino)
- Connect the CP2102 USB-TTL device to the computer via USB
- Connect the CP2102 USB-TTL device to the board, wiring diagram:

+----------------+               +----------------+
|                |               |                |
|             GND+---------------+GND             |
|                |               |                |
|              RX+---------------+TX  Wifi LED    |
|     CP2102     |               |     Driver     |
|              TX+---------------+RX              |
|                |               |                |
|             DTR+---------------+DTR             |
|                |               |                |
|             RTS+---------------+RTS             |
|                |               |                |
+----------------+               +----------------+

- Upload the firmware with Arduino IDE

# Setup

- The device will be in AP mode first when powered on, later it switches to client mode and try to connect to a hotspot 
- Search for a network on a WiFi capable device (smartphone, computer)
- Connect to the device (SSID: led_driver, password: proba1234)
- Go to the http://192.168.4.1  address in a browser
- Go to the configuration page
- You can set:
		- Thing name: device name
		- AP password: the password of the device if it is in AP mode
		- WiFi SSID: name of the WiFi hotspot where the device will connect if it switches to client mode
		- WiFi password: password of the WiFi hotspot where the device will connect if it switches to client mode
		- MQTT server address: address of the MQTT broker
		- MQTT topic name: the mqtt topic which will be used to send messages to the device
		- MQTT name: MQTT name of the device
		- Default light intensity: numeric value which sets the light intensity (between 0 and 100) if the button pressed on the device

## Usage

- Plug in the device, it should automatically connect to the WiFi network and to the MQTT broker
- Connect to the MQTT broker on your device (smartphone, tablet, computer, any device that can run MQTT device)
- Send the requested light intensity value to the ```
/MQTT topic name/MQTT name/action``` topic
- If the message received and the light intensity value is different than the previous you should get a message ```/MQTT topic name/MQTT name/status``` topic which shows you the light intensity which was just set
- If you would like to use the device without giving MQTT commands you can use a physical momentary switch, you can connect the switch to the SW pins

## TODO

- Improve thermal performance
- Create secure MQTT connection capability

## Copyright

See [LICENSE](https://github.com/as-ideas/TransformerTTS/blob/master/LICENSE) for details.

## Credits

- The firmware heavily based on Balázs Kelemen's [IotWebConf](https://github.com/prampec/IotWebConf) library
