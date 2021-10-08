#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
RF24 radio(7, 8); // CE, CSN
const byte address[6] = "00001";
LiquidCrystal_I2C lcd(0x27, 16, 2);

// All functions declaration for c++
void motorStop();
void motorRun();
void homing();
void inputAction();
void printScreen();
void seqStarter();
void testSeq();
void singleRun();
void lcdStopped();
void setInputFlags();
void resolveInputFlags();
void inputAction(int input);
void movinRight();
void movinLeft();
void centerMove();
void movinCenter();
void transmit();
void radioWait();

// transmittion call function
void transmit(char text) {
  radio.write(&text, sizeof(text));
}

//--------------------------MenuLogic
//Input & Button Logic
const int numOfInputs = 4;
const int inputPins[numOfInputs] = {2,3,4,5};
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

//Speed Logic ------
void (*speedCall[]) () = {
  []{ lcd.setCursor(0,1); //p0
    lcd.print("2.5 s");},
  []{ lcd.setCursor(0,1); //p1
    lcd.print("5.0 s");},
  []{ lcd.setCursor(0,1); //p2
    lcd.print("In Progress...");}
};

//position mark
int motionNow = 0;
int centerNow = 0;


void setup() {
  //Radio session
  radio.begin();
  radio.openWritingPipe(address);
  radio.setPALevel(RF24_PA_HIGH);
  radio.stopListening();
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
  for(int i = 0; i < numOfInputs; i++) {
    pinMode(inputPins[i], INPUT);
    digitalWrite(inputPins[i], HIGH); // pull-up 20k
  }
  //----Menu Logic Group
  
  lcd.clear();
  printScreen();
  
}

void loop() {
    setInputFlags();
    resolveInputFlags();
    /*if(Serial.available()) {
      String incData = Serial.readString();
      incData.trim();
      if(incData == "S1R") {
        currentParameter1 = 0; //speed 2.5
        seqStarter();
        printScreen();
      }
      else if(incData == "S2R") {
        currentParameter1 = 1; //speed 5
        seqStarter();
        printScreen();
      }
      else if(incData == "CAL") {
        homing();
        printScreen();
      }
      else if(incData == "CEN") {
        centerMove();
        printScreen();
      }
      // UPDATE SCREEN Move to insdie call function
      
    }*/
    // char myChar[10] = "char";
    // String myString = String(myChar);
    // delay(500);
    // lcd.print(myString);
    // delay(1000);
    // while(myString == "char") {
    //   lcd.clear();
    //   lcd.print(millis()/1000);
    //   delay(1000);
    // }


}



void homing() {

  //Serial.println("Stepper is Homing . . .");
  //transmit("CALI");
  const char text[] = "CALI";
  radio.write(&text, sizeof(text));
  
  lcd.clear();
  lcd.print("<....Homing....>");
  lcd.setCursor(0,1);
  lcd.print(" In Progress...");
  }

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

void radioWait() {
  
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

void moZero() {
      movinLeft();
      speedCall[currentParameter1] ();
      motionNow = 1;
      centerNow = 0;
      
}

void moOne() {
      movinRight();
      speedCall[currentParameter1] ();
      motionNow = 0;
      centerNow = 0;
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
  //transmit("CENTER");
  const char text[] = "Hello World";
  radio.write(&text, sizeof(text));

  if(centerNow == 0) {
          movinCenter(); //LCD Print
          speedCall[2] ();
          motionNow = 1;
          centerNow = 1;
  } else {
    movinRight();
    speedCall[2] ();
    motionNow = 0;
    centerNow = 0;
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
