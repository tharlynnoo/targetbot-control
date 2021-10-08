#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <AccelStepper.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
RF24 radio(7, 8); // CE, CSN
const byte addresses[][6] = {"00001", "00002"};

LiquidCrystal_I2C lcd(0x38, 16, 2);

// All functions declaration for c++
void motorStop();
void motorRun();
void homing();
void inputAction();
void printScreen();
void testSeq();
void seqStarter();
void testSeq();
void singleRun();
void lrSeq();
void lcdStopped();
void setInputFlags();
void resolveInputFlags();
void inputAction(int input);
void movinRight();
void movinLeft();
void centerMove();
void movinCenter();

//--------------------------Stepper
//Stepper
#define stepPin 4
#define dirPin 5
#define enbPin 6
//Homing & Control
#define buttonPin 9 //home sens

int buttonState = 0;
long degreeNow = 0;
int motionNow = 0;

int buttonState2 = 1;

long TravelX;
int move_finished = 1;
long initial_homing = -1;
//----------------------

AccelStepper stepper(AccelStepper::DRIVER, stepPin, dirPin);
//--------------------------Stepper

//--------------------------MenuLogic
//Input & Button Logic
const int numOfInputs = 4;
const int inputPins[numOfInputs] = {5,6,7,4};
int inputState[numOfInputs] = {LOW,LOW,HIGH,HIGH};
int lastInputState[numOfInputs] = {LOW,LOW,LOW,LOW};
bool inputFlags[numOfInputs] = {LOW,LOW,LOW,LOW};
long lastDebounceTime[numOfInputs] = {0,0,0,0};
long debounceDelay = 5;

//LCD Menu Logic
//---Screen
const int numOfScreens = 3;
int currentScreen = 0;
String screens[numOfScreens][2] = {{"Ready! Speed Sel","s"}, {"<....Homing....>", " Recalibrate!"}, {"<....Center....>","     Move"}};
//---Speed Select
const int numOfParameters1 = 2;
int currentParameter1 = 0;
String parameter1[numOfParameters1][1] = {{"*2m in 2.5"}, {"*2m in 5"}};
//--------------------------MenuLogic

//---SpeedSelection & initial position
int startPos = 169;
int endPos = 3982;

//Speed Logic ------
void (*speedCall[]) () = {
  []{stepper.setMaxSpeed(1650); //parameter 0
    lcd.setCursor(0,1);
    lcd.print("2.5 s");},
  []{stepper.setMaxSpeed(825);  //parameter 1
    lcd.setCursor(0,1);
    lcd.print("5.0 s");},
  []{stepper.setMaxSpeed(825);  //parameter 2
    lcd.setCursor(0,1);
    lcd.print("In Progress...");}
};


void setup() {
  //----Greetings
  lcd.begin();
  lcd.print("TargetBot v1");
  lcd.setCursor(0,1);
  lcd.print("Manufactured By");
  delay(500);
  lcd.setCursor(0,1);
  lcd.print("MYOMA ind.     ");
  delay(1500);
  //----Greetings

  //----Menu Logic Group
  /*for(int i = 0; i < numOfInputs; i++) {
    pinMode(inputPins[i], INPUT);
    digitalWrite(inputPins[i], HIGH); // pull-up 20k
  }*/
  //----Menu Logic Group
  
  //----Stepper Group
  stepper.setEnablePin(enbPin);     //setting Enable Pin
  motorStop();
  
  pinMode(buttonPin, INPUT);        //Homing

  motorRun();                       //Power on the motor
  homing();                         //Homing Sequence
  
  //Serial.begin(9600);
  lcd.clear();
  //lcd.print("Starting");
  /*inputFlags[1] = HIGH; //trigger the menu
  inputFlags[0] = HIGH;*/
  printScreen();
  //Serial.begin(9600);
  radio.begin();
  radio.openWritingPipe(addresses[0]); // 00001
  radio.openReadingPipe(1, addresses[1]); // 00002
  radio.setPALevel(RF24_PA_LOW);
}

void loop() {
    //setInputFlags();
    //resolveInputFlags();
    delay(5);
  radio.startListening();
  if (radio.available()) {
    char text[32] = "";
    radio.read(&text, sizeof(text));
        String radioCall = String(text);
      if(radioCall == "SP1RUN") {
        currentParameter1 = 0; //speed 2.5
        seqStarter();
        printScreen();
      }
      else if(radioCall == "SP2RUN") {
        currentParameter1 = 1; //speed 5
        seqStarter();
        printScreen();
      }
      else if(radioCall == "CALI") {
        homing();
        printScreen();
      }
      else if(radioCall == "CENTER") {
        centerMove();
        printScreen();
      }

  }
  delay(5);
  radio.stopListening();
  const char text[] = "Hi";
  radio.write(&text, sizeof(text));
}




void motorStop() {
  digitalWrite(enbPin, HIGH);
}
void motorRun() {
  digitalWrite(enbPin, LOW);
}

void homing() {
  stepper.setMaxSpeed(600);
  stepper.setAcceleration(400);

  //Serial.println("Stepper is Homing . . .");
  lcd.clear();
  lcd.print("<....Homing....>");
  lcd.setCursor(0,1);
  lcd.print(" In Progress...");

  while (!digitalRead(buttonPin)) {
    stepper.moveTo(initial_homing);
    initial_homing--;
    stepper.run();
    //delay(2);
  }

  stepper.setCurrentPosition(0);
  stepper.setMaxSpeed(600);
  stepper.setAcceleration(4000);
  initial_homing=1;

  while (digitalRead(buttonPin)) {
    stepper.moveTo(initial_homing);
    initial_homing++;
    stepper.run();
    //delay(2);  
  }

  stepper.setCurrentPosition(0);
  //Serial.println("Homing Completed");
  lcd.setCursor(0,1);
  lcd.print("   Completed!   ");
  stepper.setMaxSpeed(600);
  stepper.setAcceleration(4000);
  delay(500);
  //Serial.println("Starting . . .");
  lcd.clear();
  lcd.print("Starting . . .");
  delay(1000);                      //Delay Set
  motionNow = 0;
  stepper.runToNewPosition(startPos);
  //motorStop();
  //----Stepper Group
}
/*void setInputFlags() {
  for(int i = 0; i < numOfInputs; i++) {
    int reading = digitalRead(inputPins[i]);
    if (reading != lastInputState[i]) {
      lastDebounceTime[i] = millis();
    }
    if ((millis() - lastDebounceTime[i]) > debounceDelay) {
      if (reading != inputState[i]) {
        inputState[i] = reading;
        if (inputState[i] == HIGH) {
          inputFlags[i] = HIGH;
          Serial.print(inputFlags[i]);
        }
      }
    }
    lastInputState[i] = reading;
  }
}*/
void setInputFlags() {
  for(int i = 0; i < numOfInputs; i++) {
    int reading = digitalRead(inputPins[i]);
    if (reading != lastInputState[i]) {
      lastDebounceTime[i] = millis();
      debounceDelay = 5; //don't think we need if statement
    }
    if ((millis() - lastDebounceTime[i]) > debounceDelay) {
      if (reading == LOW) {
          inputFlags[i] = HIGH;
          lastDebounceTime[i] = millis();
          debounceDelay = 500;
          Serial.print(inputFlags[i]);
      }
    }
    lastInputState[i] = reading;
  }
}

void resolveInputFlags() {
  for(int i = 0; i < numOfInputs; i++) {
    if(inputFlags[i] == HIGH) {
      inputAction(i);
      inputFlags[i] = LOW;
      printScreen();
    }
  }
}

void inputAction(int input) {
  //Serial.print(input);
  if(input == 0) {            //button 0 Left Button
    if (currentScreen == 0) {
      currentScreen = numOfScreens-1;
    }else{
      currentScreen--;
    }
  }else if(input == 1) {      //button 1 Right Button
    if (currentScreen == numOfScreens-1) {
      currentScreen = 0;
    }else{
      currentScreen++;
    }
  }else if(input == 2) {      //button 2 SEL Button
      //speed Menu
      if (currentScreen == 0) {
      if (currentParameter1 == numOfParameters1-1) {
        currentParameter1 = 0;
      }else{
        currentParameter1++;}
      }
      // Homing Menu
      if (currentScreen == 1) {
        homing();
        inputFlags[0] = HIGH;
      }
      // Center Menu
      if (currentScreen == 2) {
        centerMove();
      }
      
  }else if(input == 3) {      //button 4 Start Button
      //speed Menu - Start
      if (currentScreen == 0) {
         seqStarter(); //call seqStarter function
      }
      // Homing Menu
      if (currentScreen == 1) {
        homing();
        inputFlags[0] = HIGH;
      }
      // Center Menu
      if (currentScreen == 2) {
        centerMove();
      }
  }
}


void printScreen() {
  
  lcd.clear();
  lcd.print(screens[currentScreen][0]);
  lcd.setCursor(0,1);
  
  if (currentScreen == 0) {
    lcd.print(parameter1[currentParameter1][0]);
  }
  lcd.print(" ");
  lcd.print(screens[currentScreen][1]);
}



void testSeq() {
  lcd.clear();
  lcd.print("Test Sequence....");
  lcd.setCursor(0,1);
  lcd.print(" Demo Progress...");
  delay(3000);
  lcd.setCursor(0,1);
  lcd.print(" Demo Completed! ");
  delay(2000);
}

void moZero() {
      movinLeft();
      speedCall[currentParameter1] ();
      motionNow = 1;
      //motorRun();
      stepper.runToNewPosition(endPos);
      //motorStop();
      
}

void moOne() {
      movinRight();
      speedCall[currentParameter1] ();
      motionNow = 0;
      //motorRun();
      stepper.runToNewPosition(startPos);
      //motorStop();
}

void seqStarter() {
  switch (currentParameter1) {
    case 0:
      singleRun();
      break;
    case 1:
      singleRun();
      break;     
  }
}

void singleRun() {
  if(motionNow == 0) {
          moZero(); //Move left
          lcdStopped();
          delay(20);
  } else if (motionNow == 1) {
          moOne(); //Move Right
          lcdStopped();
          delay(20);
  }
}

void centerMove() {
  long position = stepper.currentPosition();
  if(position != ((startPos+endPos)/2)) {
          movinCenter(); //LCD Print
          speedCall[2] ();
          motionNow = 1;
          stepper.runToNewPosition((startPos+endPos)/2);

  } else {
    movinRight();
    speedCall[2] ();
    motionNow = 0;
    stepper.runToNewPosition(startPos);
  }

}

void lrSeq() {
  //stepper.runToNewPosition(1500);
  if (motionNow == 0) {
      movinLeft();
      lcd.setCursor(0,1);
      lcd.print("2.5 s in Seq!");
      
      stepper.setMaxSpeed(1650);
      stepper.runToNewPosition(endPos);
      lcdStopped();
      delay(500);

      movinRight();
      lcd.setCursor(0,1);
      lcd.print("5 s in Seq!");
      
      stepper.setMaxSpeed(825);
      stepper.runToNewPosition(startPos);
      lcdStopped();
      
      } else if (motionNow == 1) {
      movinRight();
      lcd.setCursor(0,1);
      lcd.print("5 s in Seq!");
        
      stepper.setMaxSpeed(1650);
      stepper.runToNewPosition(startPos);
      lcdStopped();
      delay(500);

      movinLeft();
      lcd.setCursor(0,1);
      lcd.print("5 s in Seq!");

      stepper.setMaxSpeed(825);
      stepper.runToNewPosition(endPos);
      lcdStopped();
      
      }
}

void lcdStopped() {
  lcd.setCursor(0, 0);
  lcd.print("<<  Stopped  >>");
}

void movinRight() {
  lcd.clear();
  lcd.print("Moving Right--->");
}

void movinLeft() {
  lcd.clear();
  lcd.print("<---Moving Left");
}

void movinCenter() {
  lcd.clear();
  lcd.print("<----Moving---->");
}
