

//options
#define showTimeMax 18000 //max length of show
#define showTimeMin 14000 //min length of show
#define motorRunTime 3000 //time to run motor after insert detected. 
#define updatePeriod 8 //time between PWM updates 
#define pulseRate  0.005 //ammount to increment the pulse brightness 
#define colourRate 0.025 //how fast to change between colours
#define ignorePeriod 250 //how long to ignore between interupts to get rid of bouncing
#define pulseRange 40 //127.5 is full bright per pulse, 63.75 is half


//global variables
volatile int showTime = showTimeMin;
float pulseBrightness = 0; 
float pulsePosition = 4.712; 
float redBrightness = 0; 
float greenBrightness = 0; 
float blueBrightness = 0; 
float redPosition = 4.712; 
float greenPosition = 6.806;  //the variables here set the three lights to be out of phase
float bluePosition = 8.907; 

//this function sets a timer that automatically increments
elapsedMillis timeSinceInsert;
elapsedMillis timeSinceUpdate;

//What pins are hooked up to which devices   
#define redPin A6
#define greenPin A7
#define bluePin A9
#define irPin 7
#define irPin2 8
#define motorPin 9
#define fanPin 10

void setup() {
  pinMode(irPin, INPUT);
  pinMode(irPin2, INPUT);
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
  pinMode(motorPin, OUTPUT);
  pinMode(fanPin, OUTPUT);
  attachInterrupt(irPin, insertDetected, FALLING);
  attachInterrupt(irPin2, insertDetected, FALLING);
  randomSeed(analogRead(A5));
}


void loop() { 
  if (timeSinceInsert < showTime){ //should we be running?
    if (timeSinceInsert < motorRunTime) { //should the motors be on
      digitalWrite(motorPin, HIGH);
    }
    else {
      digitalWrite(motorPin, LOW);
    }
    digitalWrite(fanPin, HIGH);

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

  }
  else{  //what? no donation.. ahh well pulse the lights with white. 
    digitalWrite(motorPin, LOW);
    digitalWrite(fanPin, LOW);
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
  }


}

void insertDetected() //yay! a donation
{
  if (timeSinceInsert > ignorePeriod){//ignore inputs for a bit to avoid multiple resets due to bounce
    timeSinceInsert = 0; 
    timeSinceUpdate = 0; 
    showTime = random(showTimeMin,showTimeMax); //set a random show time, because we can. 
  }
}



