/*#################__add includes here__#############################################################################*/

#include <Servo.h>            //include servo library               //!!!!these librarys may need to be installed!!!!
#include <EEPROM.h>           //include Eeprom library
#include <SparkFun_TB6612.h>  //include sparkfun TB6612 library

/*#################__add defines here__##############################################################################*/

#define AIN1 12       //AI2 of control board connected to Arduino pin 12
#define AIN2 11       //AI2 of control board connected to Arduino pin 11
#define PWMA 10       //PWMA of control board connected to Arduino pin 10
#define STBY 13       //STBY of control board connected to Arduino pin 13
#define BIN1 7        //BI1 of control board connected to Arduino pin 7
#define BIN2 6        //BI2 of control board connected to Arduino pin 6
#define PWMB 5        //PWMB of control board connected to Arduino pin 5
#define ServoPWM 3    //Servo PWM input connected to Arduino pin 3
#define ModeSelect 8  //User mode select switch ci=onnected to Arduino pin 8

#define servo_addr 0    //address 0 location for storing servo position
#define mode_addr 1     //address 1 location for storing mode
#define lastMode_addr 2 //address 2 location for storing lastMode

const int offsetA = 1;  //needed by sparkfun library to set up the motor instance(actuator) below, (value can be 1 or -1) 
const int offsetB = 1;  //needed by sparkfun library to set up the motor instance(servo) below, (value can be 1 or -1)
/*#################__add variable's here__############################################################################*/

int servoPosition = 0;  //initialize variable servoPosition
int buttonState = 0;    //initialize variable buttonState
int mode = 0;           //initialize variable mode
int lastMode = 0;       //initialize variable lastMode

/*#################__add instance's here__#############################################################*/

Servo myservo;                                            //set up an instance of Servo called myservo 
Motor actuator = Motor(AIN1, AIN2, PWMA, offsetA, STBY);  //set up an instance of Motor called actuator 
Motor servo = Motor(BIN1, BIN2, PWMB, offsetB, STBY);     //set up an instance of Motor called servo

/*#################__add function prototypes here__####################################################*/

void mode_0(void); //wait here for user command (push button)
void mode_1(void); //extract horizontally
void mode_2(void); //extract vertically
void mode_3(void); //retract mode 2
void mode_4(void); //retract mode 1

/*#################__add setup here__########################################################################################################################################################*/

void setup() {
  servoPosition = EEPROM.read(servo_addr); //recover servo to this last known position after power loss
  myservo.write(servoPosition);            //start servo in last known position
  mode = EEPROM.read(mode_addr);           //recover last known user mode after power loss
  lastMode = EEPROM.read(lastMode_addr);   //recover last known pervious user mode after power loss
  pinMode(ModeSelect, INPUT_PULLUP);       //setup up Arduino pin 8(ModeSelect) as input pin from user mode select switch, pullup activated, pin 8 sets high and is pulled low by switch press
  myservo.attach(ServoPWM);                //attaches Arduino PWM pin 3(ServoPWM) to the servo motor, controls servo position
}

/*#################__add forever loop here__#################################################################################################################################################*/

void loop() {
  
   if (lastMode != mode){ //check to see if lastMode not equal present mode 
    
      if (mode > 4)      //check if mode selected is greater than 4 if so end of cycle and reset to mode 1
          mode = 1;      //set mode to mode 1
       
      if (mode == 1)     //check if mode 1 is selected
          mode_1();      //excute mode 1 function
   
      if (mode == 2)     //check if mode 2 is selected
          mode_2();      //excute mode 2 function
   
      if (mode == 3)     //check if mode 3 is selected
          mode_3();      //excute mode 3 function
       
      if (mode == 4)     //check if mode 4 is selected
          mode_4();      //excute mode 4 function
   }
   else{                 //else if last mode is equal to present mode
      mode_0();          //excute mode 0 and wait for user input
   }
  
}

/*#################__add functions here__####################################################*/

//wait here for user command (push button)
  
void mode_0(void){ 
  while(lastMode == mode){    //loop here till user has selected new mode
    buttonState = digitalRead(ModeSelect); //Check if user has pressed the button

    if (buttonState == 0)    //Check if user mode select switch has been pressed
        delay(15);           //debounce switch so we dont get more than one single press
      
    if (buttonState == 0){   //Check if user mode select switch is still pressed. 
        mode ++;             //increment mode (0-4 inclusive)
        EEPROM.write(mode_addr, mode); //save selected mode 
    }
  }
}

//extract horizontally

void mode_1(void){
  actuator.drive(255,1700);               //extract horizontally, 255 connects power forward polairty, 1700 is delay in ms it takes to fully extract
  actuator.brake();                       //Cuts power to actuator
  lastMode = mode;                        //last mode = this mode
  EEPROM.write(lastMode_addr, lastMode);  //save last mode
}

//extract vertically

/*if less than 180 degrees of movement is required adjust this "servoPosition <= 180" to between min 0-180 max */
/*speed of servo can be adjusted in two ways for faster increase the step size "servoPosition += 1", or decrease "delay(15)" and opposite for slower*/

void mode_2(void){     
  servo.drive(255);                                                   // connects power forward polairty to servo motor (reverse polairty not used)
  for (servoPosition = 0; servoPosition <= 180; servoPosition += 1) { // goes from 0 degrees to 180 degrees in steps of 1 degree
    myservo.write(servoPosition);                                     // tell servo to go to position in variable servoPosition
    delay(15);                                                        // waits 15ms for the servo to reach the position, speed control
  }
  servo.brake();                                                      //cuts power to servo
  EEPROM.write(servo_addr, 180);                                      //saved servo last position
  lastMode = mode;                                                    //last mode = this mode
  EEPROM.write(lastMode_addr, lastMode);                              //save last mode
}

  //retract vertically
  
/*if less than 180 degrees of movement is required adjust mode 2 frist and give "servoPosition = 180;" the new desired value */
/*speed of servo can be adjusted in two ways for faster increase the step size "servoPosition += 1", or decrease "delay(15)" and opposite for slower*/

void mode_3(void){
  servo.drive(255);                                                   // connects power forward polairty to servo motor (reverse polairty not used)
  for (servoPosition = 180; servoPosition >= 0; servoPosition -= 1) { // goes from 180 degrees to 0 degrees in steps of 1 degree
    myservo.write(servoPosition);                                     // tell servo to go to position in variable servoPosition
    delay(15);                                                        // waits 15ms for the servo to reach the position, speed control
  }
  servo.brake();                                                      //cuts power to servo
  EEPROM.write(servo_addr, 0);                                        //saved servo last position
  lastMode = mode;                                                    //last mode = this mode
  EEPROM.write(lastMode_addr, lastMode);                              //save last mode
}

  //retract horizontally
  
void mode_4(void){ 
  actuator.drive(-255,1700);                 //retract horizontally, -255 connects power reverse polarity, 1700 is delay in ms it takes to fully retract 
  actuator.brake();                          //cuts actuator power
  lastMode = mode;                           //last mode = this mode
  EEPROM.write(lastMode_addr, lastMode);     //save last mode
}
