## Paper Sequencer Project by :  

- Adan Suleiman
- Yazan Daoud
- Nashwa Jaber

## Details about the project

Paper Sequencer is an auditory device designed to emit a distinctive sound upon detecting a black square within any of its five discs.

our user can: 

- Upload new sound files via a webpage or simply enjoy our pre-downloaded default sounds!

- Start and stop the system: click to start and click to stop the disks from spinning!

- Control the volume: move a button left and right to raise and reduce the volume!

## Wiring diagram and pins:

![Alt text](images/wiring_diagram.JPG)

SD:

GND - GND

MISO - 19

MOSI - 23

SCK - 18

CS - 5

3V3 - 3V3


Max98357 Audio Amplifier:
Field
Pin
GND
GND
DOUT
25
BCLK
27
LRC
26
VIN
VIN


Servo motor:
Field
Pin
GND
GND
VIN
5v
ServoPin
9


Sensors:
Field
Pin
IR_PIN1
35
IR_PIN2
34
IR_PIN3
32
IR_PIN4
33
IR_PIN5
22
VIN
5v
GND
GND






Speaker:
Field
Pin
FMT
GND
XMT
3.3v
DOUT
25
BCLK
27
LRC
26
VIN
5v
GND
GND




## Folder description :
 You can find the following files:
- **Servo_with_click_arduino.ino:**
  A code that runs servo in a constant speed and a button can be attached.
- **Audio_and_sensor_simple_test.ino:**
  A test that takes an audio file from an SD and releases it once the sensor encounters a black square.
- **Audio_with_SD_simple_test.ino:**
  A test that takes an audio file from an SD and starts playing it.
- **basic_finished.ino:**
  Our project's main code.
- **user_interface.html:**
  Our user interface webpage.
- **http_handling.ino:**
  using esp32 as an access point.

## Arduino/ESP libraries installed for the project:
* ESP32Servo.h
* SD.h
* I2Saudio.h
* WiFi.h
* WebServer.h
* ESPAsyncWebServer.h

## Project Poster:
 
This project is part of ICST - The Interdisciplinary Center for Smart Technologies, Taub Faculty of Computer Science, Technion
https://icst.cs.technion.ac.il/

![Alt text](images/poster.png)

