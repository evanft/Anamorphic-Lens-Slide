////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Anamorphic Lens Slide Program
//by evanft at AVSForums
//Revision 0
//20-Oct-2019
//
//
//This program controls a stepper motor attached to a slide mechanism that moves an anamorphic lens into and out of position in front of a projector.
//I created it because new motorized slides don't seem to be widely available anymore and used ones are either pricy or only available when also
//purchasing a lens that's more expensive than my projector.
//
//
//Movement is achieved via a three-pin stepper motor. The slide positions are controlled by two limit switches. One for in front of the projector
//and the other to the side of the projector. Movement can be triggered by either an IR input or a 12V trigger, like one provided by most projectors.
//
//Below are the parts I used. These can be found all over Amazon or eBay from a variety of sellers. The whole setup will run about $200-$250 depending on how
//you build the slide and what you have on hand.
//- 400mm Travel Length Linear Stage Actuator (Part number Sfu1605). This comes with a 3.0A 1.8deg 24V stepper motor. I'm sure you could also build one using some
//  extruded aluminium and an off-the-shelf motor.
//- Elegoo Uno. I bought the super starter kit with the various extra little parts, like the IR sensor and remote. I highly recommend getting a kit if you don't
//  already have one. The extra wires, sensors, etc. really do help, especially when you're troubleshooting.
//- TB6600 stepper motor driver. There are a lot of these available from manufacturers in China.
//- Meanwell 24V 6.0A power supply. This is for powering the stepper motor through the motor driver. I imagine many others will work fine as long as it has enough
//  power for your particular stepper motor.
//- Limit switches. Tons of options for these. Just get a bag of them from Amazon or something.
//- LM2596S DC-DC convertor. This steps down the 12V signal to 5V so the Arduino will play nice with it. Not needed if not using the trigger.
//- Sensor shield v5. Not absolutely necessarily, but makes wiring much easier and cleaner.
//
//Here are some things to keep in mind before adapting this to your setup.
//- In my setup left moves it front of the projector and right moves it away from the projector.
//- The IR remote codes are hard coded in the program. These would need to change based on whatever remote you use. You will likely need to 
//  use an IR code capture program of some kind to figure out what the codes are if you're using something different. There are many examples for this
//  online. I used the remote that came with my kit and simply added the codes to my projector in my Harmony hub setup.
//- The pause between motor pulses may differ based on your choice of motor. If you're having problems, this may be causing it.
//- Different stepper motors may have different pin setups or power needs, so the TB6600 may not work for certain motors.
//- I used a three-prong IR sensor that came with my kit. I don't know if different sensors have different pins or communicate differently.
//- My limit switches were wired as normally open (NO). You will need to reverse the states in the program if you wire them as normally closed (NC).
//
//Here are some hurdles I ran into while working on this.
//- On many TB6600 drivers the A and B pins are swapped. Before plugging everything in open up your driver and confirm the pins are correct.
//- For some reason running the serial print out causes some weird behavior with the driver and motor. It almost seems like it's changing the delay
//  between pauses somehow. If you want to use the serial output, just disconnect the power from the driver/motor.
//- If I set my motor enable pin to high while the motor isn't moving, the motor makes a slight buzzing noise. Turning the pin off when not moving fixes this.
//- Pin 1 on the Arduino is a serial pin. Don't connect anything to it.
//- The limit switches need some somewhat special wiring. The common pin needs to be wired to both the communication pin on the Arduino and a 5V source. You 
//  also need to put a resistor in between this pin and the 5V. 5Kohm worked for me. The NO or NC pin needs to be wired to ground.
//
//Lastly, I am not a programmer. I am not an expert. The last time I programmed anything was about a decade ago when I used MATLAB to find the path of a rocket
//for a dynamics project. I picked this up and got it all put together using other people's code and tips/tutorials I found online. I'm sure there are more efficient
//or otherwise superior ways to do this. You are free to take this code, make it better, and distribute it. Just put my name on it or something.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// --------LIBRARIES--------------
#include <IRremote.h>        //Include IR remote library in code

// --------CONSTANTS AND VARIABLES---------------
const int enPin = 4;         //Define motor enable pin
const int dirPin = 5;        //Define motor direction pin
const int stepPin = 6;       //Define motor pulse pin
const int RECV_PIN = 8;      //Define IR receiver input pin
const int SWITCHL = 11;      //Define switch to stop in front of projector
const int SWITCHR = 13;      //Define switch to stop away from projector
const int pause = 450;       //Define motor pulse delay
const int LEFT = 1;          //Define left value for variable controlling motor movement. 
const int RIGHT = 2;         //Define right value for variable controlling motor movement
const int STOP = 0;          //Define stop value for variable controlling motor movement 
const int ON = 1;            //Define ON value for the trigger and infrared switch variables
const int OFF = 0;           //Define OFF value for the trigger and infrared switch variable;
const int bounce = 50;       //Define a bounce delay to avoid double sensing inputs.
boolean leftstate = 0;       //Create a boolean variable for the state of the left limit switch
boolean rightstate = 0;      //Create a boolean variable for the state of the right limit switch
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
  irrecv.enableIRIn();        //Enable IR receiver input
  //Serial.begin(9600);
}

void(* resetFunc) (void) = 0;  //This is a reset function we can use to reset the board with an IR command

//-------MAIN LOOP-----------
//In order to get this working properly, I needed to break the program up into smaller pieces that had little jobs to do. 
//This loop gets those pieces started.
//---------------------------
void loop(){
  readIR();
  readSwitches();
  setDirection();
  startMove();}

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
              //Serial.println("IR LEFT");
              if(movement != LEFT){
              movement = LEFT;}
          break;
          
          case 0xFFC23D: //Right
              //Serial.println("IR RIGHT");
           if(movement != RIGHT){
              movement = RIGHT;}
          break ;  

           case 0xFF02FD: //Stop
              //Serial.println("IR STOP");
           if(movement != STOP){
              movement = STOP;}
          break ;  

           case 0xFF7A85: //Reset the board
              Serial.println("INFRARED RESET");
              resetFunc(); 
         break;     
        }
       delay(bounce);
       key_value = results.value;
       irrecv.resume(); 
}}


//----Limit Switch Read Function---
//Reads the state of the limit switches and sets variable values based on those states. 
//---------------------------------
void readSwitches(){
  rightstate = digitalRead(SWITCHR);
  leftstate = digitalRead(SWITCHL);

  if(rightstate == LOW && movement == RIGHT || leftstate == LOW && movement == LEFT){
    movement = STOP;
  }
}

//----Set Direction Function---
//Sets the direction of the motor's movement based on the variable values set in the IR or trigger read functions.
//-----------------------------
void setDirection(){
  if(movement == RIGHT){         //Set direction as right
    digitalWrite(dirPin,LOW);}
  if(movement == LEFT){          //Set direction as left
    digitalWrite(dirPin,HIGH);}
  if(movement == STOP){
    digitalWrite(enPin,LOW);}    //Set direction as stop
}

//----Start Movement Function---
//Moves the sled based on movement variable values.
//-----------------------------
void startMove(){
if(movement != STOP){
    digitalWrite(enPin,HIGH);
    digitalWrite(stepPin,HIGH);
    delayMicroseconds(pause); 
    digitalWrite(stepPin,LOW);
    delayMicroseconds(pause);}}
