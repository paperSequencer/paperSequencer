## Paper Sequencer Project by :  

- Adan Suleiman
- Yazan Daoud
- Nashwa Jaber
- 
## Details about the project

Paper Sequencer is an auditory device designed to emit a distinctive sound upon detecting a black square within any of its five discs.

our user can: 

- Upload new sound files via a webpage or simply enjoy our pre-downloaded default sounds!

- Start and stop the system: click to start and click to stop the disks from spinning!

- Control the volume: move a button left and right to raise and reduce the volume!
 
## Folder description :
* ESP32: source code for the esp side (firmware).
* Documentation: wiring diagram + basic operating instructions
* Unit Tests: tests for individual hardware components (input / output devices)
* flutter_app : dart code for our Flutter app.
* Parameters: contains description of configurable parameters 
* Assets: 3D printed parts, Audio files used in this project,
* You can find the following files:
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

## Arduino/ESP32 libraries used in this project:
## Arduino/ESP libraries installed for the project:
* XXXX - version XXXXX
* XXXX - version XXXXX
* XXXX - version XXXXX

## Project Poster:
 
This project is part of ICST - The Interdisciplinary Center for Smart Technologies, Taub Faculty of Computer Science, Technion
https://icst.cs.technion.ac.il/
