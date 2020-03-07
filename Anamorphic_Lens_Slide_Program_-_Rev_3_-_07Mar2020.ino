////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Anamorphic Lens Slide Program
//by evanft at AVSForums
//github.com/evanft
//Revision 2
//07-Mar-2020
//
//Rev log
//- Rev 0: Initial release
//- Rev 1: Added support for 12V trigger input. Removed most of intro text and added note about GitHub page. 
//- Rev 2: Corrected issue which would cause the sled to constantly want to move, then stop, then move, then stop. Fixed by adding in control variable
//  for movement function
//
//This program controls a stepper motor attached to a slide mechanism that moves an anamorphic lens into and out of position in front of a projector.
//I created it because new motorized slides don't seem to be widely available anymore and used ones are either pricy or only available when also
//purchasing a lens that's more expensive than my projector.
//
//
//Movement is achieved via a three-pin stepper motor. The slide positions are controlled by two limit switches. One for in front of the projector
//and the other to the side of the projector. Movement can be triggered by either an IR input or a 12V trigger, like one provided by most projectors.
//
//For more information, please see GitHub site.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// --------LIBRARIES--------------
#include <IRremote.h>        //Include IR remote library in code

// --------CONSTANTS AND VARIABLES---------------
const int enPin = 4;         //Define motor enable pin
const int dirPin = 5;        //Define motor direction pin
const int stepPin = 6;       //Define motor pulse pin
const int RECV_PIN = 8;      //Define IR receiver input pin
const int SWITCHL = 11;      //Define switch to stop in front of projector
const int SWITCHR = 12;      //Define switch to stop away from projector
const int trigPin = A0;      //Define pin for trigger input
const int pause = 450;       //Define motor pulse delay
const int LEFT = 1;          //Define left value for variable controlling motor movement. 
const int RIGHT = 2;         //Define right value for variable controlling motor movement
const int STOP = 0;          //Define stop value for variable controlling motor movement 
const int START = 3;         //Define start value for variable controlling motor movement
const int ON = 1;            //Define ON value for the trigger and infrared switch variables
const int OFF = 0;           //Define OFF value for the trigger and infrared switch variable;
const int bounce = 500;      //Define a bounce delay to avoid double sensing inputs. I like adding this to most of my functions.
const int bigbounce = 2000;  //Define a big bounce delay for the trigger input
boolean leftstate = 0;       //Create a boolean variable for the state of the left limit switch
boolean rightstate = 0;      //Create a boolean variable for the state of the right limit switch
boolean triggerstate = 0;    //Create a boolean variable for the state of trigger input
boolean lasttriggerstate = 0;//Create a boolean variable for the last state of the trigger input
boolean startmove = 0;       //Create a boolean variable for the movement state. 
int movement = 0;            //Create a variable for controlling motor movement

// --------INSTANCES---------------
IRrecv irrecv(RECV_PIN);     //Create instance of 'irrecv'
decode_results results;      //Create instance of 'decode_results'
unsigned long key_value = 0; //Needed for the IR receiver 


//-------SETUP-----------
//Define the various pins as outputs/inputs and enables the IR receiver
//-----------------------
void setup(){
  pinMode(stepPin,OUTPUT);    //Set motor pulse pin as an output
  pinMode(dirPin,OUTPUT);     //Set motor direction pin as an output
  pinMode(enPin,OUTPUT);      //Set motor enable pin as an output
  pinMode(SWITCHL, INPUT);    //Set left position switch as input
  pinMode(SWITCHR, INPUT);    //Set right position switch as input
  pinMode(RECV_PIN, INPUT);   //Set IR receiver pin as input
  pinMode(trigPin, INPUT);    //Set trigger input pin as input
  irrecv.enableIRIn();        //Enable IR receiver input
  Serial.begin(9600);
}

void(* resetFunc) (void) = 0;  //This is a reset function we can use to reset the board with an IR command

//-------MAIN LOOP-----------
//In order to get this working properly, I needed to break the program up into smaller pieces that had little jobs to do. 
//This loop gets those pieces started.
//---------------------------
void loop(){
  readIR();
  readTrigger();
  readSwitches();
  setDirection();
  Move();}

//-------IR Read Function------
//Reads the input from the IR sensor and sets variables based on what it sees. These variables are used
//by other functions. This code is mostly copy/paste from examples online.
//-----------------------------
void readIR(){
if(irrecv.decode(&results)){

  //This little piece of code prevents the value of the IR code from being 0XFFFFFFFF, which is a code often sent by IR remotes after sending the 
  //actual function code.
       if (results.value == 0XFFFFFFFF) 
       results.value = key_value;

   //This section uses a switch function along with the IR input results to set the variables used by the other functions.
        switch(results.value){
          case 0xFF22DD: //Left
             Serial.println("IR LEFT");
              movement = LEFT;
          break;
          
          case 0xFFC23D: //Right
              Serial.println("IR RIGHT");
              movement = RIGHT;
          break ;  

           case 0xFF02FD: //Stop
             Serial.println("IR STOP");
              movement = STOP;
          break ;  

           case 0xFF7A85: //Reset the board
              //Serial.println("INFRARED RESET");
              resetFunc(); 
         break;     
        }
       delay(bounce);
       key_value = results.value;
       irrecv.resume(); 
}}


//-------Trigger Read Function------
//Reads the input from the 12V trigger input and sets variables based on what it sees. 
//-----------------------------
void readTrigger(){
  triggerstate = analogRead(trigPin);
  Serial.println(triggerstate);
  
  if(triggerstate != lasttriggerstate){
    delay(bigbounce);
  
  if(triggerstate != lasttriggerstate){
    if(triggerstate == HIGH){
     Serial.println("TRIGGER HIGH");
      movement = LEFT;}
    
    if(triggerstate == LOW){
     Serial.println("TRIGGER LOW");
      movement = RIGHT;}
      delay(bounce);
   lasttriggerstate = triggerstate;}}
}

//----Limit Switch Read Function---
//Reads the state of the limit switches and sets variable values based on those states. 
//---------------------------------
void readSwitches(){
  rightstate = digitalRead(SWITCHR);
  leftstate = digitalRead(SWITCHL);
  if(rightstate == LOW && movement == RIGHT || leftstate == LOW && movement == LEFT){
    movement = STOP;
    startmove = STOP;
    Serial.println("SWITCH STOP");
    delay(bounce);
  }
}

//----Set Direction Function---
//Sets the direction of the motor's movement based on the variable values set in the IR or trigger read functions.
//-----------------------------
void setDirection(){
  if(movement == RIGHT && rightstate == HIGH){         //Set direction as right
    startmove = START;
    digitalWrite(dirPin,LOW);}
  if(movement == LEFT && leftstate == HIGH){          //Set direction as left
    startmove = START;
    digitalWrite(dirPin,HIGH);}
  if(movement == STOP){                               //Set direction as stop
    startmove = STOP;
    digitalWrite(enPin,LOW);}    
}

//----Start Movement Function---
//Moves the sled based on movement variable values.
//-----------------------------
void Move(){
if(movement != STOP && startmove == START){
    Serial.println("MOVE");
    digitalWrite(enPin,HIGH);
    digitalWrite(stepPin,HIGH);
    delayMicroseconds(pause); 
    digitalWrite(stepPin,LOW);
    delayMicroseconds(pause);}}
