#include "Wire.h"
#include "LiquidCrystal.h" //library for the LCD
// Connect via i2c, default address #0 (A0-A2 not jumpered)
LiquidCrystal lcd(0);

//options
#define showTimeMax 18000 //max length of show
#define showTimeMin 14000 //min length of show
#define motorRunTime 3000 //time to run motor after insert detected. 
#define updatePeriod 8 //time between PWM updates for RGB lights
#define pulseRate  0.005 //ammount to increment the pulse brightness this is the white fade when nothing else is happening
#define pulseRange 40 //127.5 is full bright per pulse, 63.75 is half
#define colourRate 0.025 //how fast to change between colours
#define ignorePeriod 250 //how long to ignore between interupts to get rid of bouncing switches
#define LCDupRate 10000 //how fast to update the LCD while waiting
#define LCDupRate2 5000 //how fast to update the LCD while running
#define danceTimeMin 1000 //what is the minimum time he should be dancing
#define danceTimeMax 6000 //what is the maximum time he should be dancing

//global variables
int showTime = showTimeMin;
volatile int danceTime = danceTimeMin;
boolean danceStop = false; //false if moving, true if stationary this is used to create the pauses in his dance
volatile boolean insertDetect = false; //boolean to allow me to reset things that dont get reset in an interupt. 
float pulseBrightness = 0; 
float pulsePosition = 4.712; 
float redBrightness = 0; 
float greenBrightness = 0; 
float blueBrightness = 0; 
float redPosition = 4.712; 
float greenPosition = 6.806;  //the variables here set the three lights to be out of phase
float bluePosition = 8.907;  //basicaly I am running three sin waves, one for each colour
int LCDprogram;

//this function sets a timer that automatically increments
elapsedMillis timeSinceInsert;
elapsedMillis timeSinceUpdate;
elapsedMillis timeSinceLCD;
elapsedMillis timeSinceLCDupdate;
elapsedMillis timeSinceDanceMove;
//there are a lot of timers because there is a lot going on

//What pins are hooked up to which devices
#define redPin A6
#define greenPin A7
#define bluePin A9
#define irPin 7
#define irPin2 8
#define motorPin 0
#define fanPin 10
#define motorEnable 1
#define robotDir 2
#define feederDir 3
#define robotSpd 4
#define feederSpd 9


void setup() {
  //set up the pins
  pinMode(irPin, INPUT);
  pinMode(irPin2, INPUT);
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
  pinMode(motorPin, OUTPUT);
  pinMode(motorEnable, OUTPUT);
  pinMode(robotDir, OUTPUT);
  pinMode(feederDir, OUTPUT);
  pinMode(robotSpd, OUTPUT);
  pinMode(feederSpd, OUTPUT);
  pinMode(fanPin, OUTPUT);

  //set up the interrupts which read from the IR sensors
  attachInterrupt(irPin, insertDetected, FALLING); //this causes the interupt to trigger when the pin goes low
  attachInterrupt(irPin2, insertDetected, FALLING);

  //this seed is used for random program elements like showtime and flashes
  randomSeed(analogRead(A5)); //read from an unconnected pin = randomish

  // set up the LCD's number of rows and columns: 
  lcd.begin(16, 2);
  // Print a message to the LCD.
  lcd.setCursor(0,0);
  lcd.print("Hello World");
  lcd.setCursor(0,1);
  lcd.print("Now Loading..."); //default start up message
}


void loop() { 
  if (insertDetect==true) { //this is a hook to reset variables right after the IR beams were triggered
  //I did it here to minimize the time spent in an interupt subroutine
    timeSinceInsert = 0; //lets reset a bunch of variables to set things in motion
    timeSinceUpdate = 0; 
    timeSinceLCD = 20000;//setting this high causes the LCD to trigger a change
    timeSinceDanceMove = 0;
    lcd.clear(); //clear the LCD so we don't get left over lines
    showTime = random(showTimeMin,showTimeMax); //set a random show time 
    danceTime = random(danceTimeMin,danceTimeMax); //set a random show time
    insertDetect=false; //Lets not do this variable reset again untill it gets reset by the interupt
  }

  if (timeSinceInsert < showTime){ //should we be running?
    digitalWrite(motorEnable, HIGH);
    digitalWrite(motorPin, HIGH);
    digitalWrite(fanPin, HIGH);

    if (timeSinceInsert < motorRunTime) { //should the feeder motors be on
      digitalWrite(feederDir, LOW);
      analogWrite(feederSpd, 130);
    }
    else {
      digitalWrite(feederDir, LOW); //turn the feeder motors off
      analogWrite(feederSpd, 0);
    }


    if (timeSinceDanceMove < danceTime){ //the robot moves till he hits his dance time, and pauses
      if (danceStop==false){
        digitalWrite(robotDir, LOW);
        analogWrite(robotSpd, 220); //255 = full speed, dont set it to this as it may cause the motor to explode
        //well, kind of explode, more burn out, its rated at 12v, the battery pack outputs 13.5, so by reducing the power here
        //we can run the motor safely
      }
      else{
        digitalWrite(robotDir, LOW);
        analogWrite(robotSpd, 0);
      }
    }
    else{ 
      if (danceStop==false){
        timeSinceDanceMove = 0;
        danceTime = int(random(300,1000)); //this sets a random pause duration for the robot
        danceStop=true;
      }
      else{  
        timeSinceDanceMove = 0;
        danceTime = random(danceTimeMin,danceTimeMax); //set a random dance time
        danceStop=false;
      }
    }


    if (timeSinceUpdate > updatePeriod){ //how long since we last incremented the lighting variables

      redPosition = redPosition + colourRate;
      greenPosition = greenPosition + colourRate + random(0.01,0.2); //the random bit is to make the lights go out of sync
      bluePosition = bluePosition + colourRate + random(0.001,0.03);
      if (redPosition > 10.995) {  //this bit causes the variable to loop over 
        redPosition = 4.712;
      }
      if (greenPosition > 10.995) {
        greenPosition = 4.712;
      }
      if (bluePosition > 10.995) {
        bluePosition = 4.712;
      }
      redBrightness = sin(redPosition) * 127.5 + 127.5; //the function generates a sin wave to bring the lights on and off in a cool way
      greenBrightness = sin(greenPosition) * 127.5 + 127.5;
      blueBrightness = sin(bluePosition) * 127.5 + 127.5;
      timeSinceUpdate = 0;
    }

    analogWrite(redPin,redBrightness); //send the output of the sin wave to the lights
    analogWrite(greenPin,greenBrightness);
    analogWrite(bluePin,blueBrightness);
    
    
    if (timeSinceLCD > LCDupRate2) { //this updates the LCD while the show is happening
      lcd.clear();     
      LCDprogram = int(random(1,8)); //pick a number from one to how many messages are in the next section
      //if you add more messages change the 8 to the new count
      timeSinceLCD = 0;
      timeSinceLCDupdate = 2000;
    }
    if (timeSinceLCDupdate > 250){

      switch (LCDprogram) {//switch case, this compares the random value picked above, and displays the corisponding message
      case 1: 
        if (timeSinceLCD % 2) { //this if statement checks if the time is even or odd, its a cheap hack to get it to flash the LCD
          lcd.setCursor(0,0);
          lcd.print(" ! !PARTY ! ! ! ");
          lcd.setCursor(0,1);
          lcd.print("! ! ! ! ! ! ! ! ");
        }
        else {
          lcd.setCursor(0,0);
          lcd.print("! ! PARTY! ! ! !");
          lcd.setCursor(0,1);
          lcd.print(" ! ! ! ! ! ! ! !");
        }
        break;
      case 2:
        if (timeSinceLCD % 2) {
          lcd.setCursor(0,0);
          lcd.print("* * THANK YOU* *");
          lcd.setCursor(0,1);
          lcd.print("* * * * * * * * ");
        }
        else {
          lcd.setCursor(0,0);
          lcd.print(" * *THANK YOU * ");
          lcd.setCursor(0,1);
          lcd.print(" * * * * * * * *");
        }     
        break;
      case 3:
        lcd.setCursor(0,0);
        lcd.print("    HEY YOU!    ");
        lcd.setCursor(0,1);
        lcd.print(" YOU'RE AWESOME ");
        break;
      case 4:
        lcd.setCursor(0,0);
        lcd.print("   THANK YOU    ");
        lcd.setCursor(0,1);
        lcd.print("INSERT NAME HERE"); 
        break;
      case 5:
        lcd.setCursor(0,0);
        lcd.print("    PLAYER 1    ");
        lcd.setCursor(0,1);
        lcd.print("FLAWLESS VICTORY"); 
        break;
      case 6:
        lcd.setCursor(0,0);
        lcd.print(" BOOMSHAKALAKA! ");
        lcd.setCursor(0,1);
        lcd.print("   THANK YOU    "); 
        break;
      case 7: 
        if (timeSinceLCD % 2) {
          lcd.setCursor(0,0);
          lcd.print("PARTYPARTYPARTY ");
          lcd.setCursor(0,1);
          lcd.print("partypartyparty ");
        }
        else {
          lcd.setCursor(0,0);
          lcd.print("partypartyparty ");
          lcd.setCursor(0,1);
          lcd.print("PARTYPARTYPARTY ");
        }
        break;
      case 8:
        if (timeSinceLCD % 2) {
          lcd.setCursor(0,0);
          lcd.print("THANKYOUTHANKYOU");
          lcd.setCursor(0,1);
          lcd.print("thankyouthankyou");
        }
        else {
          lcd.setCursor(0,0);
          lcd.print("thankyouthankyou");
          lcd.setCursor(0,1);
          lcd.print("THANKYOUTHANKYOU");
        }     
        break;
        
        //to add more messages just add extra case #: break; here remebering to change the random number above
      }
      timeSinceLCDupdate = 0;
    }


  }
  else{  //what? no donation.. ahh well pulse the lights with white. 
    digitalWrite(motorPin, LOW);
    digitalWrite(fanPin, LOW);
    digitalWrite(motorEnable, LOW);

    if (timeSinceUpdate > updatePeriod){
      pulsePosition = pulsePosition + pulseRate;
      if (pulsePosition > 10.995) {
        pulsePosition = 4.712;
      }
      pulseBrightness = sin(pulsePosition) * pulseRange + pulseRange;
      timeSinceUpdate = 0;
    }
    analogWrite(redPin,pulseBrightness);
    analogWrite(greenPin,pulseBrightness);
    analogWrite(bluePin,pulseBrightness);

    if (timeSinceLCD > LCDupRate) {//this changes the LCD when the program is not running
      lcd.clear();
      LCDprogram = int(random(1,11)); //random number again
      timeSinceLCD = 0;
      timeSinceLCDupdate = 2000;
    }
    if (timeSinceLCDupdate > 1000){

      switch (LCDprogram) {
      case 1: 
        lcd.setCursor(0,0);
        lcd.print("Last Donation:  ");
        lcd.setCursor(0,1);
        lcd.print("        ");
        lcd.setCursor(0,1);
        lcd.print(int(timeSinceInsert/1000));
        lcd.setCursor(8,1);
        lcd.print("seconds");
        break;
      case 2:
        lcd.setCursor(0,0);
        lcd.print("    Will work   ");
        lcd.setCursor(0,1);
        lcd.print(" for attention  ");     
        break;
      case 3:
        lcd.setCursor(0,0);
        lcd.print("  Robots need   ");
        lcd.setCursor(0,1);
        lcd.print("    love too    "); 
        break;
      case 4:
        lcd.setCursor(0,0);
        lcd.print("Three laws safe ");
        lcd.setCursor(0,1);
        lcd.print("     ...ish     "); 
        break;
      case 5:
        lcd.setCursor(0,0);
        lcd.print("  Insert Coin   ");
        lcd.setCursor(0,1);
        lcd.print("                ");

        break;
      case 6:
        lcd.setCursor(0,0);
        lcd.print("Man its hot, I  ");
        lcd.setCursor(0,1);
        lcd.print("want a popsicle ");
        break;
      case 7:
        lcd.setCursor(0,0);
        lcd.print("  Art, Science  ");
        lcd.setCursor(0,1);
        lcd.print("and Engineering");
        break;
      case 8:
        lcd.setCursor(0,0);
        lcd.print("Feed me, Seymore");
        lcd.setCursor(0,1);
        lcd.print("                ");
        break;
      case 9:
        lcd.setCursor(0,0);
        lcd.print(" klaatu barada  ");
        lcd.setCursor(0,1);
        lcd.print("     nikto      ");
        break;
      case 10:
        lcd.setCursor(0,0);
        lcd.print("   Beakerhead   ");
        lcd.setCursor(0,1);
        lcd.print("                ");
        break;
      case 11:
        lcd.setCursor(0,0);
        lcd.print("     I dance    ");
        lcd.setCursor(0,1);
        lcd.print(" for the donors ");
        break;
        
        //to add more messages just add extra case #: break; here remebering to change the random number above
      }
      timeSinceLCDupdate = 0;
    }
  }
}

void insertDetected() //yay! a donation this is an interupt handler
{
  if (timeSinceInsert > ignorePeriod){//ignore inputs for a bit to avoid multiple resets due to bounce

    insertDetect=true; //this variable is volatile, which means we can change it here
    //this boolean triggers a hook in the main loop. 

  }
}










