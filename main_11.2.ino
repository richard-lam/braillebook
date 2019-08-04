/*
  This is a test sketch for the Adafruit assembled Motor Shield for Arduino v2
  It won't work with v1.x motor shields! Only for the v2's with built in PWM
  control

  For use with the Adafruit Motor Shield v2
  ---->  http://www.adafruit.com/products/1438
*/

/*
   implemented the char array flip for all characters [tested]
*/
#include <Wire.h>
#include <Adafruit_MotorShield.h>
#include <stdlib.h>
//#include "utility/Adafruit_PWMServoDriver.h"
#include <SoftwareSerial.h>

#define DISKNUMBER 40
#define DISPLAY_TEXT 48  //0
#define PLAY_AUDIO 49    //1

#define LEDPIN 13
#define SENSORPIN 8
// Create the motor shield object with the default I2C address
Adafruit_MotorShield AFMS = Adafruit_MotorShield();
// Or, create it with a different I2C address (say for stacking)
// Adafruit_MotorShield AFMS = Adafruit_MotorShield(0x61);

// Connect a stepper motor with 200 steps per revolution (1.8 degree)
// to motor port #2 (M3 and M4)
Adafruit_StepperMotor *rotMotor = AFMS.getStepper(200, 2);
//200 steps per revolution to motor port #1 (M1 and M2)
Adafruit_StepperMotor *linMotor = AFMS.getStepper(200, 1);

// constants that wont change
const int previousLineButtonPin = 4;
const int nextLineButtonPin = 5;
const int playAudioButtonPin = 3;
const int powerButtonPin = 10;
const int audioTx = 6;  //SIN
const int audioRx = 7;  //SOUT

// global variables:
int ct = 1;
int currentPosition[DISKNUMBER];
int nextPosition[DISKNUMBER];
int turnNumber[DISKNUMBER];
String currentDisplay;
int justPoweredOn = 1;

// button states:
int playAudioButtonState;
int playAudioLastButtonState = LOW;
int nextLineButtonState;
int nextLineLastButtonState = LOW;
int previousLineButtonState;
int previousLineLastButtonState = LOW;
int currentPowerButtonState = LOW;

//sensor states:
int sensorState = 0, lastState = 0;       

// button time variables:
long playAudioLastDebounceTime = 0;
long nextLineLastDebounceTime = 0;
long previousLineLastDebounceTime = 0;
long debounceDelay = 50;



SoftwareSerial emicSerial = SoftwareSerial(audioRx, audioTx);

void setup() {
  //sensor setup
  // initialize the LED pin as an output:
  pinMode(LEDPIN, OUTPUT);
  // initialize the sensor pin as an input:
  pinMode(SENSORPIN, INPUT);
  pinMode (previousLineButtonPin, INPUT);
  pinMode (nextLineButtonPin, INPUT);
  pinMode (playAudioButtonPin, INPUT);
  pinMode (audioRx, INPUT);
  pinMode (audioTx, OUTPUT);
  pinMode (powerButtonPin, INPUT);
  digitalWrite(SENSORPIN, HIGH); // turn on the pullup
  currentPowerButtonState = digitalRead(powerButtonPin);
}

void loop() {
  int command;

  while (ct == 0) {
    if (checkPowerButton()){
      checkPlayAudioButton();
        checkNextLineButton();
        checkPreviousLineButton();
    if (Serial.available() > 0) {
      char newDisplay[DISKNUMBER / 2 + 1];

        for (int g = 0; g < DISKNUMBER / 2 + 1; g++) {
          newDisplay[g] = 0;
        }

        String incoming = " ";
        incoming = Serial.readString();
        command = incoming.charAt(0);
        incoming = incoming.substring(1);
        incoming.toCharArray(newDisplay, DISKNUMBER / 2 + 1);

        if (( (DISKNUMBER / 2) % 2) == 0) { //even number of disks
          for (int x = 0; x < DISKNUMBER / 4; x++) {
            char temp = newDisplay[x];
            newDisplay[x] = newDisplay[(DISKNUMBER / 2) - 1 - x];
            newDisplay[(DISKNUMBER / 2) - 1 - x] = temp;
          }
        }
        else { //odd number of disks
          for (int y = 0; y < DISKNUMBER / 4; y++) {
            char temp = newDisplay[y];
            newDisplay[y] = newDisplay[(DISKNUMBER / 2) - 1 - y];
            newDisplay[(DISKNUMBER / 2) - 1 - y] = temp;
          }
        }

        delay(1);

        if (command == DISPLAY_TEXT) {
          //Serial.println("Start out");
          determineNextPos(newDisplay);
          turnOut();
          //Serial.println("End");
          currentDisplay = incoming;
          ct++;
        }
        else if (command == PLAY_AUDIO) {
          sendTextToAudio(currentDisplay);
        }
      }
    }
  }

  while (ct == 1) {
    if (checkPowerButton()){
        checkPlayAudioButton();
        checkNextLineButton();
        checkPreviousLineButton();
        if (Serial.available() > 0) {
          char newDisplay[DISKNUMBER / 2 + 1];
          for (int g = 0; g < DISKNUMBER / 2 + 1; g++) {
            newDisplay[g] = 0;
          }
          String incoming = " ";
          incoming = Serial.readString();
          command = incoming.charAt(0);
          incoming = incoming.substring(1);
          incoming.toCharArray(newDisplay, DISKNUMBER / 2 + 1);
    
          if (( (DISKNUMBER / 2) % 2) == 0) { //even number of disks
            for (int x = 0; x < DISKNUMBER / 4; x++) {
              char temp = newDisplay[x];
              newDisplay[x] = newDisplay[(DISKNUMBER / 2) - 1 - x];
              newDisplay[(DISKNUMBER / 2) - 1 - x] = temp;
            }
          }
          else { //odd number of disks
            for (int y = 0; y < DISKNUMBER / 4; y++) {
              char temp = newDisplay[y];
              newDisplay[y] = newDisplay[(DISKNUMBER / 2) - 1 - y];
              newDisplay[(DISKNUMBER / 2) - 1 - y] = temp;
            }
          }
    
          delay(1);
    
          if (command == DISPLAY_TEXT) {
            //Serial.println("Start in");
            determineNextPos(newDisplay);
            turnIn();
            //Serial.println("End");
            currentDisplay = incoming;
            ct--;
          }
          else if (command == PLAY_AUDIO) {
            sendTextToAudio(currentDisplay);
          }
        }
    }
  }
}

void genTurns() {

  resetTurnCt();
  int turnCounter = 0;

  for (int i = DISKNUMBER - 1; i > -1; i--) {

    if ((nextPosition[i] - currentPosition[i] - turnCounter >= 0) && (nextPosition[i] - currentPosition[i] - turnCounter <= 4)) {
      turnNumber[i] = nextPosition[i] - currentPosition[i] - turnCounter;
      currentPosition[i] = nextPosition[i];
      turnCounter = (turnCounter + turnNumber[i]) % 8;
      //Serial.print(turnCounter);
      //Serial.print(",");
      //Serial.println(turnNumber[i]);
    }
    else if ((nextPosition[i] - currentPosition[i] - turnCounter >= 0) && (nextPosition[i] - currentPosition[i] - turnCounter > 4)) {
      turnNumber[i] = -8 + (nextPosition[i] - currentPosition[i] - turnCounter);
      currentPosition[i] = nextPosition[i];
      turnCounter = (turnCounter + 8 + turnNumber[i]) % 8;
      //Serial.print(turnCounter);
      //Serial.print(",");
      //Serial.println(turnNumber[i]);
    }
    else if ((nextPosition[i] - currentPosition[i] - turnCounter <= 0) && (nextPosition[i] - currentPosition[i] - turnCounter >= -4)) {
      turnNumber[i] = nextPosition[i] - currentPosition[i] - turnCounter;
      currentPosition[i] = nextPosition[i];
      turnCounter = (turnCounter + 8 + turnNumber[i]) % 8;
      /*Serial.print(turnCounter);
        Serial.print(",");
        Serial.println(turnNumber[i]);*/
    }
    else if ((nextPosition[i] - currentPosition[i] - turnCounter <= 0) && (nextPosition[i] - currentPosition[i] - turnCounter < -4)) {
      turnNumber[i] = 8 + (nextPosition[i] - currentPosition[i] - turnCounter);
      currentPosition[i] = nextPosition[i];
      turnCounter = (turnCounter + turnNumber[i]) % 8;
      //Serial.print(turnCounter);
      //Serial.print(",");
      //Serial.println(turnNumber[i]);
    }
  }
}

void turnIn() {
  genTurns();
  for (int i = 0; i < DISKNUMBER; i++) {

    if (turnNumber[i] >= 0) {
      rotMotor->step(25 * turnNumber[i], BACKWARD, DOUBLE);
      if (turnNumber[i] != 0)
        delay(50);
      if (i < DISKNUMBER - 1) {
        linMotor->step(20, BACKWARD, DOUBLE);
        if (turnNumber[i + 1] != 0)
          delay(50);
      }
    }
    else {
      turnNumber[i] = turnNumber[i] * (-1);
      rotMotor->step(25 * turnNumber[i], FORWARD, DOUBLE);
      if (turnNumber[i] != 0)
        delay(50);
      if (i < DISKNUMBER - 1) {
        linMotor->step(20, BACKWARD, DOUBLE);
        if (turnNumber[i + 1] != 0)
          delay(50);
      }
    }
  }
}

void turnOut() {
  genTurns();
  for (int k = DISKNUMBER - 1; k > -1; k--) {

    if (turnNumber[k] >= 0) {
      rotMotor->step(25 * turnNumber[k], BACKWARD, DOUBLE);
      if (turnNumber[k] != 0)
        delay(50);
      if (k > 0) {
        linMotor->step(20, FORWARD, DOUBLE);
        if (turnNumber[k - 1] != 0)
          delay(50);
      }
    }
    else {
      turnNumber[k] = turnNumber[k] * (-1);
      rotMotor->step(25 * turnNumber[k], FORWARD, DOUBLE);
      if (turnNumber[k] != 0)
        delay(50);
      if (k > 0) {
        linMotor->step(20, FORWARD, DOUBLE);
        if (turnNumber[k - 1] != 0)
          delay(50);
      }
    }
  }
}

void resetTurnCt() {
  for (int i = 0; i < DISKNUMBER; i++) {
    turnNumber[i] = 0;
  }
}

void determineNextPos(char letters[]) {
  for (int g = 0; g < DISKNUMBER; g++) {
    nextPosition[g] = 0;
  }
  int holdSize = DISKNUMBER / 2 + 1;
  //char letters[holdSize];

  //newDisplay.toCharArray(letters, holdSize);
  int diskNum = 0;

  for (int k = 0; k < holdSize - 2; k++) {
    //Serial.print(letters[k]);
  }
  //Serial.println(letters[holdSize - 2]);

  for (int i = 0; i < holdSize; i++) {
    diskNum = i * 2;
    if (letters[i] == 'a' || letters[i] == 'c' || letters[i] == 'd' || letters[i] == 'e' || letters[i] == '1' || letters[i] == '3' || letters[i] == '4' || letters[i] == '5') {
      nextPosition[diskNum + 1] = 4;
    }
    else if (letters[i] == 'i' || letters[i] == 'j' || letters[i] == 'w' || letters[i] == ',' || letters[i] == '.' || letters[i] == ':' || letters[i] == '9' || letters[i] == '0') {
      nextPosition[diskNum + 1] = 2;
    }
    else if (letters[i] == 'b' || letters[i] == 'f' || letters[i] == 'g' || letters[i] == 'h' || letters[i] == '2' || letters[i] == '6' || letters[i] == '7' || letters[i] == '8') {
      nextPosition[diskNum + 1] = 6;
    }
    else if (letters[i] == 'k' || letters[i] == 'm' || letters[i] == 'n' || letters[i] == 'o' || letters[i] == 'u' || letters[i] == 'x' || letters[i] == 'y' || letters[i] == 'z') {
      nextPosition[diskNum + 1] = 5;
    }
    else if (letters[i] == 'l' || letters[i] == 'p' || letters[i] == 'q' || letters[i] == 'r' || letters[i] == 'v') {
      nextPosition[diskNum + 1] = 7;
    }
    else if (letters[i] == 's' || letters[i] == 't' || letters[i] == ';' || letters[i] == '!' || letters[i] == '?') {
      nextPosition[diskNum + 1] = 3;
    }
    else if (letters[i] == '#') {
      nextPosition[diskNum + 1] = 1;
    }
    else if (letters[i] == ' ') {
      nextPosition[diskNum] = 0;
      nextPosition[diskNum + 1] = 0;
    }

    if (letters[i] == 'a' || letters[i] == 'b' || letters[i] == 'k' || letters[i] == 'l' || letters[i] == ',' || letters[i] == ';' || letters[i] == '1' || letters[i] == '2') {
      nextPosition[diskNum] = 0;
    }
    if (letters[i] == 'c' || letters[i] == 'f' || letters[i] == 'i' || letters[i] == 'm' || letters[i] == 'p' || letters[i] == 's' || letters[i] == '3' || letters[i] == '6' || letters[i] == '9') {
      nextPosition[diskNum] = 4;
    }
    if (letters[i] == 'd' || letters[i] == 'g' || letters[i] == 'j' || letters[i] == 'n' || letters[i] == 'q' || letters[i] == 't' || letters[i] == '4' || letters[i] == '7' || letters[i] == '0') {
      nextPosition[diskNum] = 6;
    }
    if (letters[i] == 'e' || letters[i] == 'h' || letters[i] == 'o' || letters[i] == 'r' || letters[i] == ':' || letters[i] == '!' || letters[i] == '5' || letters[i] == '8') {
      nextPosition[diskNum] = 2;
    }
    if (letters[i] == 'u' || letters[i] == 'v' || letters[i] == '?') {
      nextPosition[diskNum] = 1;
    }
    if (letters[i] == 'z' || letters[i] == '.') {
      nextPosition[diskNum] = 3;
    }
    if (letters[i] == 'x') {
      nextPosition[diskNum] = 5;
    }
    if (letters[i] == 'w' || letters[i] == 'y' || letters[i] == '#') {
      nextPosition[diskNum] = 7;
    }
  }
}

void sendTextToAudio(String text) {
  //Serial.print("1");
  emicSerial.print('S');
  emicSerial.print(text);
  //Serial.println(text);
  emicSerial.print('\n');
  while (emicSerial.read() != ':');
}

void checkPlayAudioButton() {
  int playAudioButtonReading = digitalRead(playAudioButtonPin);

  if (playAudioButtonReading != playAudioLastButtonState) {
    playAudioLastDebounceTime = millis();
  }
  if ((millis() - playAudioLastDebounceTime) > debounceDelay) {
    if (playAudioButtonReading != playAudioButtonState) {
      playAudioButtonState = playAudioButtonReading;
      if (playAudioButtonState == HIGH) {
        sendTextToAudio(currentDisplay);
        //sendTextToAudio("play");
        delay(600);
      }
    }
  }
  playAudioLastButtonState = playAudioButtonReading;
}

void checkNextLineButton() {
  int nextLineButtonReading = digitalRead(nextLineButtonPin);

  if (nextLineButtonReading != nextLineLastButtonState) {
    nextLineLastDebounceTime = millis();
  }
  if ((millis() - nextLineLastDebounceTime) > debounceDelay) {
    if (nextLineButtonReading != nextLineButtonState) {
      nextLineButtonState = nextLineButtonReading;
      if (nextLineButtonState == HIGH) {
        Serial.println("1");
        sendTextToAudio("next");
        delay(600);
      }
    }
  }
  nextLineLastButtonState = nextLineButtonReading;
}

void checkPreviousLineButton() {
  int previousLineButtonReading = digitalRead(previousLineButtonPin);

  if (previousLineButtonReading != previousLineLastButtonState) {
    previousLineLastDebounceTime = millis();
  }
  if ((millis() - previousLineLastDebounceTime) > debounceDelay) {
    if (previousLineButtonReading != previousLineButtonState) {
      previousLineButtonState = previousLineButtonReading;
      if (previousLineButtonState == HIGH) {
        Serial.println("0");
        sendTextToAudio("previous");
        delay(600);
      }
    }
  }
  previousLineLastButtonState = previousLineButtonReading;
}

void runStartupRoutine() {
  delay(1000);
  Serial.begin(9600);           // set up Serial library at 9600 bps
  //Serial.println("Braillebook!");

  AFMS.begin();  // create with the default frequency 1.6KHz
  //AFMS.begin(1000);  // OR with a different frequency, say 1KHz

  emicSerial.begin(9600);
  emicSerial.print('\n');
  while (emicSerial.read() != ':');
  delay(10);
  emicSerial.flush();
  emicSerial.print("V18");
  emicSerial.print('\n');
  emicSerial.print("sBraille book");
  emicSerial.print('\n');
  delay(600);

  rotMotor->setSpeed(1000);
  linMotor->setSpeed(1000);
  int senseCT = 0;
  while (1) {
    sensorState = digitalRead(SENSORPIN);

    // check if the sensor beam is broken
    // if it is, the sensorState is LOW:
    if (sensorState == LOW) {
      // turn LED on:
      digitalWrite(LEDPIN, HIGH);
      if (senseCT) {
        linMotor->step(198, FORWARD, DOUBLE);
        break;
      }
      else {
        linMotor->step(250, BACKWARD, DOUBLE);
        senseCT = 1;

      }

    }
    else {
      // turn LED off:
      digitalWrite(LEDPIN, LOW);
      linMotor->step(1, FORWARD, DOUBLE);
      senseCT = 1;
    }
    lastState = sensorState;
  }

  rotMotor->step(2, FORWARD, DOUBLE);
  delay(100);
  rotMotor->step(2, BACKWARD, DOUBLE);
  delay(100);

  for (int g = 0; g < DISKNUMBER; g++) {
    currentPosition[g] = 0;
    nextPosition[g] = 0;
    turnNumber[g] = 0;
  }
}

void runShutdownRoutine(){
  char newDisplay[DISKNUMBER / 2 + 1];
  for (int g = 0; g < DISKNUMBER / 2 + 1; g++) {
    newDisplay[g] = 0;
  }

  if (ct == 1){
    if (currentDisplay != ""){
      determineNextPos(newDisplay);
      turnIn();
      determineNextPos(newDisplay);
      turnOut();
    }
  }
  else if (ct == 0){
    determineNextPos(newDisplay);
    turnOut();
  }
  delay(1500);
  //reset global variables
  ct = 1;
  currentDisplay = "";
  justPoweredOn = 1;
  playAudioLastButtonState = LOW;
  nextLineLastButtonState = LOW;
  previousLineLastButtonState = LOW;
  currentPowerButtonState = LOW;
  sensorState = 0;
  lastState = 0;
  playAudioLastDebounceTime = 0;
  nextLineLastDebounceTime = 0;
  previousLineLastDebounceTime = 0;
  debounceDelay = 50;

  emicSerial.print('\n');
  emicSerial.print("sGoodbye");
  emicSerial.print('\n');
  delay(1000);

  //end communication with peripherals:
  Serial.end();
  emicSerial.end();
  rotMotor->release();
  linMotor->release();
}

int checkPowerButton(){
  if (digitalRead(powerButtonPin) != currentPowerButtonState){
    if (justPoweredOn == 1){
      justPoweredOn = 0;
      if (digitalRead(powerButtonPin) == LOW){
        currentPowerButtonState = digitalRead(powerButtonPin);
        return LOW;
      }
    }
    if (justPoweredOn == 0){
      currentPowerButtonState = digitalRead(powerButtonPin);
      
      if (currentPowerButtonState == HIGH){
        runStartupRoutine();
      }
      else if (currentPowerButtonState == LOW){
        runShutdownRoutine();
      }
    }
  }
  if (justPoweredOn == 1){
    return LOW;
  }
  else{
    return currentPowerButtonState;
  }
}

