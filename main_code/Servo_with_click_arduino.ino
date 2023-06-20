/* Sweep
 by BARRAGAN <http://barraganstudio.com>
 This example code is in the public domain.

 modified 8 Nov 2013
 by Scott Fitzgerald
 https://www.arduino.cc/en/Tutorial/LibraryExamples/Sweep
*/

#include <Servo.h>

#define pushButtonPin 2 
#define servoPin 3
int switchState = LOW;    // Initial state of the switch
int previousSwitchState = LOW;
bool isSwitchOn = false;  // Variable to hold the toggled state
Servo myservo;  // create servo object to control a servo
// twelve servo objects can be created on most boards

int pos = 0;    // variable to store the servo position

void setup() {
  myservo.attach(servoPin);  // attaches the servo on pin 9 to the servo object
  pinMode(pushButtonPin,INPUT_PULLUP); // intiates the button bin
  myservo.write(88); // moves servo in a constant velocity

}

void loop() {
  // Read the current state of the switch
  switchState = digitalRead(pushButtonPin);

  // Check if the switch state has changed
  if (switchState != previousSwitchState) {
    // If the switch state is HIGH, toggle the value of isSwitchOn
    if (switchState == HIGH) {
      isSwitchOn = !isSwitchOn;
    }
    // Update the previous switch state
    previousSwitchState = switchState;
  }

  // Use the value of isSwitchOn for further processing
  if (isSwitchOn) {
    myservo.attach(servoPin);
    myservo.write(88);
  } else {
    myservo.detach();
  }

  // Add a small delay to avoid rapid toggling when the switch is pressed
  delay(50);
}
