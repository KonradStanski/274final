/*---------------------------------------------------
#    Name: Konrad Staniszewski, Andrew Rooney
#    ID: 1531593, 1496508
#    CMPUT 274, Fall  2018
#
#    Final Project
# ---------------------------------------------------*/

/*
Functions for movement:
void left( void );
    turns 90 degrees left
void right( void );
    turns 90 degrees right
void forward( void );
    goes forward one block

Functions for communication:
int * receivePath( void );
    Reads in a character array of the path instructions

Wiring instructions:
    //Stepper Drivers
    Arduino digital pin 3     // IN1 on the ULN2003 driver 1
    Arduino digital pin 4     // IN2 on the ULN2003 driver 1
    Arduino digital pin 5     // IN3 on the ULN2003 driver 1
    Arduino digital pin 6     // IN4 on the ULN2003 driver 1
    Arduino digital pin 8     // IN1 on the ULN2003 driver 2
    Arduino digital pin 9     // IN2 on the ULN2003 driver 2
    Arduino digital pin 10    // IN3 on the ULN2003 driver 2
    Arduino digital pin 11    // IN4 on the ULN2003 driver 2
    5-12V Power Source        // +VCC on ULN2003 driver 1 and 2
    Arduino GND               // GND on ULN2003 driver 1 and 2

    //Bluetooth
    Arduino digital pin 14 TX // RX on HC-05 Module
    Arduino digital pin 15 RX // TX on HC-05 Module
    Arduino 5-12V             // VCC on HC-05 Module
    Arduino GND               // GND on HC-05 Module

    //Arduino
    5-12V Power Source        // Arduino VIN
    GND Power Source          // Arduino GND
*/

// Library includes
#include <Arduino.h>
#include <AccelStepper.h>


// Motor pin definitions
#define HALFSTEP 8

#define motorPin1  3     // IN1 on the ULN2003 driver 1
#define motorPin2  4     // IN2 on the ULN2003 driver 1
#define motorPin3  5     // IN3 on the ULN2003 driver 1
#define motorPin4  6     // IN4 on the ULN2003 driver 1

#define motorPin5  8     // IN1 on the ULN2003 driver 2
#define motorPin6  9     // IN2 on the ULN2003 driver 2
#define motorPin7  10    // IN3 on the ULN2003 driver 2
#define motorPin8  11    // IN4 on the ULN2003 driver 2


// Initialize with pin sequence IN1-IN3-IN2-IN4 for using the AccelStepper with 28BYJ-48
AccelStepper stepper1(HALFSTEP, motorPin1, motorPin3, motorPin2, motorPin4);
AccelStepper stepper2(HALFSTEP, motorPin5, motorPin7, motorPin6, motorPin8);


// variables
// wheel is 8.47cm diameter/ circ = 26.6cm
// 4096 steps in a full rotation/ 1539 steps for 10cm
// about 1645 steps for a 90deg turn
int turnSteps = 1645; // number of steps for a 90 degree turn
int lineSteps = -3400; //number of steps to drive straight
int stepperSpeed = 1000; //speed of the stepper (steps per second)
int steps1 = 0; // keep track of the step count for motor 1
int steps2 = 0; // keep track of the step count for motor 2


//Define functions:
void resetMot(void);
void left(void);
void right(void);
void forward(void);
int * receivePath(void);
void process(int length, int * direc);


void resetMot(void){
/* Resets the stepper motor state to zero the motors
*/
    stepper1.setMaxSpeed(2000.0); // max number of steps per second
    stepper1.move(1); // necessay to get it going
    stepper1.setSpeed(stepperSpeed); // apply stepper speed

    stepper2.setMaxSpeed(2000.0);
    stepper2.move(-1);
    stepper2.setSpeed(stepperSpeed);
}


void left(void){
/* Turns the robot 90 degrees left
*/
    int target = turnSteps;
    stepper1.move(target);
    stepper1.setSpeed(stepperSpeed);
    stepper2.move(target);
    stepper2.setSpeed(stepperSpeed);
    steps1 = stepper1.distanceToGo();
    while(steps1){
        stepper1.runSpeedToPosition();
        stepper2.runSpeedToPosition();
        steps1 = stepper1.distanceToGo();
    }
    stepper1.stop();
    stepper2.stop();
    delay(50);
    return;
}


void right( void ){
/* Turns the robot 90 degrees right
*/
    int target = -turnSteps;
    stepper1.move(target);
    stepper1.setSpeed(stepperSpeed);
    stepper2.move(target);
    stepper2.setSpeed(stepperSpeed);
    steps1 = stepper1.distanceToGo();
    while(steps1){
        stepper1.runSpeedToPosition();
        stepper2.runSpeedToPosition();
        steps1 = stepper1.distanceToGo();
    }
    stepper1.stop();
    stepper2.stop();
    delay(50);
    return;
}


void forward( void ){
/* Mmoves the robot forward 22 cm
*/
    int target = lineSteps;
    stepper1.move(-target);
    stepper1.setSpeed(stepperSpeed);
    stepper2.move(target);
    stepper2.setSpeed(stepperSpeed);
    steps1 = stepper1.distanceToGo();
    while(steps1){
        stepper1.runSpeedToPosition();
        stepper2.runSpeedToPosition();
        steps1 = stepper1.distanceToGo();
    }
    stepper1.stop();
    stepper2.stop();
    delay(50);
    return;
}


int * receivePath( void ) {
/* This function reads in the path sent in the "NSWE" string format from serial3
which is the bluetooth module. It then creates an array using malloc and returns the pointer to the array
this array is an array of ints which means that we will be using the ascii values of "N", "W", "S", "E"
*/
    static int pathlen;
    int pathin;
    while (Serial3.available() == 0) { }// block until read in length
    // Read in path length
    pathlen = Serial3.read();
    Serial.print("length received: ");
    Serial.println(pathlen);

    // Read in path
    static int * path;
    path = (int*)malloc(pathlen+1 * sizeof(int));
    path[0] = pathlen;
    for (int i = 1; i <= pathlen; i++) {
        while (Serial3.available() == 0){ } // wait until available
        Serial.print(i);
        Serial.print(" path received: ");
        pathin = Serial3.read();
        Serial.print(char(pathin));
        path[i] = pathin;
        Serial.println(path[i]);
    }
    delay(500); // delay needed to let arduino do some processing and flush serial
    return path;
}


void process(int length, int * direc){
/* This function reads along the length of the direc array given the pointer to the array
it uses the ascii decimal values of the letters for comparison.
If then uses a "state" machine to determine what steps to do next
The debug statemetns can be read if you connect a minicom terminal to /dev/rfcommX
*/
    Serial3.println("entering processing");
    Serial3.flush();
    Serial3.print("length is: ");
    Serial3.flush();
    Serial3.println(length);
    Serial3.flush();


    //Start with a move south
    forward();

    // Start state machine
    int state[1]; // init state
    state[0] = direc[1]; // get first state

    Serial3.print("first state: ");
    Serial3.flush();
    Serial3.println(char(state[0]));
    Serial3.flush();

    int next[1]; // init next
    for(int i = 2; i <= length; i++){
        next[0] = direc[i]; // set next state

        Serial3.print(" next state: ");
        Serial3.flush();
        Serial3.print(char(next[0]));
        Serial3.flush();

        // a if else state machine that has 4 possible moves and accepts varyations of current state and next state combinations
        if((state[0] == 78 && next[0] == 78) || (state[0] == 83 && next[0] == 83) || (state[0] == 69 && next[0] == 69) || (state[0] == 87 && next[0] == 87)){
            Serial3.println(" enter forward");
            Serial3.flush();
            forward();
        }else if((state[0] == 78 && next[0] == 83) || (state[0] == 83 && next[0] == 78) || (state[0] == 69 && next[0] == 87) || (state[0] == 87 && next[0] == 69)){
            Serial3.println(" enter turn 180 forward");
            Serial3.flush();
            right();
            right();
            forward();
        }else if((state[0] == 78 && next[0] == 69) || (state[0] == 83 && next[0] == 87) || (state[0] == 69 && next[0] == 83) || (state[0] == 87 && next[0] == 78)){
            Serial3.println(" enter right + forward");
            Serial3.flush();
            right();
            forward();
        }else if((state[0] == 78 && next[0] == 87) || (state[0] == 83 && next[0] == 69) || (state[0] == 69 && next[0] == 78) || (state[0] == 87 && next[0] == 83)){
            Serial3.println(" enter left + forward");
            Serial3.flush();
            left();
            forward();
        }
        state[0] = next[0];
    }
}


void setup() {
    init(); // itit arduino functions
    Serial.begin(9600);
    Serial3.begin(9600);
    resetMot(); // initialize the motors
}


int main() {
    setup();
    int *path; // initialize the path
    path = receivePath(); // receive the path array pointer
    int length = path[0]; // get the length from the first element of the path array
    process(length, path); // process the path and navigate the maze
    return 0;
}
