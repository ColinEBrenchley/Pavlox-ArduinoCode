#include <Wire.h>
#include <Servo.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>
#include <AccelStepper.h>

#define NOTE_B0  31
#define NOTE_C1  33
#define NOTE_CS1 35
#define NOTE_D1  37
#define NOTE_DS1 39
#define NOTE_E1  41
#define NOTE_F1  44
#define NOTE_FS1 46
#define NOTE_G1  49
#define NOTE_GS1 52
#define NOTE_A1  55
#define NOTE_AS1 58
#define NOTE_B1  62
#define NOTE_C2  65
#define NOTE_CS2 69
#define NOTE_D2  73
#define NOTE_DS2 78
#define NOTE_E2  82
#define NOTE_F2  87
#define NOTE_FS2 93
#define NOTE_G2  98
#define NOTE_GS2 104
#define NOTE_A2  110
#define NOTE_AS2 117
#define NOTE_B2  123
#define NOTE_C3  131
#define NOTE_CS3 139
#define NOTE_D3  147
#define NOTE_DS3 156
#define NOTE_E3  165
#define NOTE_F3  175
#define NOTE_FS3 185
#define NOTE_G3  196
#define NOTE_GS3 208
#define NOTE_A3  220
#define NOTE_AS3 233
#define NOTE_B3  247
#define NOTE_C4  262
#define NOTE_CS4 277
#define NOTE_D4  294
#define NOTE_DS4 311
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_FS4 370
#define NOTE_G4  392
#define NOTE_GS4 415
#define NOTE_A4  440
#define NOTE_AS4 466
#define NOTE_B4  494
#define NOTE_C5  523
#define NOTE_CS5 554
#define NOTE_D5  587
#define NOTE_DS5 622
#define NOTE_E5  659
#define NOTE_F5  698
#define NOTE_FS5 740
#define NOTE_G5  784
#define NOTE_GS5 831
#define NOTE_A5  880
#define NOTE_AS5 932
#define NOTE_B5  988
#define NOTE_C6  1047
#define NOTE_CS6 1109
#define NOTE_D6  1175
#define NOTE_DS6 1245
#define NOTE_E6  1319
#define NOTE_F6  1397
#define NOTE_FS6 1480
#define NOTE_G6  1568
#define NOTE_GS6 1661
#define NOTE_A6  1760
#define NOTE_AS6 1865
#define NOTE_B6  1976
#define NOTE_C7  2093
#define NOTE_CS7 2217
#define NOTE_D7  2349
#define NOTE_DS7 2489
#define NOTE_E7  2637
#define NOTE_F7  2794
#define NOTE_FS7 2960
#define NOTE_G7  3136
#define NOTE_GS7 3322
#define NOTE_A7  3520
#define NOTE_AS7 3729
#define NOTE_B7  3951
#define NOTE_C8  4186
#define NOTE_CS8 4435
#define NOTE_D8  4699
#define NOTE_DS8 4978
#define REST      0
#define FULLSTEP 4

int tempo = 105;
int melody[] = {
  NOTE_B4, 16, NOTE_B5, 16, NOTE_FS5, 16, NOTE_DS5, 16, //1
  NOTE_B5, 32, NOTE_FS5, -16, NOTE_DS5, 8, NOTE_C5, 16,
  NOTE_C6, 16, NOTE_G6, 16, NOTE_E6, 16, NOTE_C6, 32, NOTE_G6, -16, NOTE_E6, 8,

  NOTE_B4, 16,  NOTE_B5, 16,  NOTE_FS5, 16,   NOTE_DS5, 16,  NOTE_B5, 32,  //2
  NOTE_FS5, -16, NOTE_DS5, 8,  NOTE_DS5, 32, NOTE_E5, 32,  NOTE_F5, 32,
  NOTE_F5, 32,  NOTE_FS5, 32,  NOTE_G5, 32,  NOTE_G5, 32, NOTE_GS5, 32,  NOTE_A5, 16, NOTE_B5, 8
};

int notes = sizeof(melody) / sizeof(melody[0]) / 2;
int wholenote = (60000 * 4) / tempo;
int divider = 0, noteDuration = 0;

LiquidCrystal_I2C lcd1(0x25, 16, 4); // << Address 1
LiquidCrystal_I2C lcd2(0x27, 16, 4); // << Address 2
LiquidCrystal_I2C lcd3(0x26, 16, 4); // << Address 3
LiquidCrystal_I2C lcd4(0x23, 16, 4); // << Address 4

Servo servo;

AccelStepper auger(FULLSTEP, 14, 16, 15, 17);

const int commonPin = 2;
const int buttonPins[] = {3,4,5,6,7,8};
const int servoPin = 9;
const int buzzer = 10;

unsigned long lastFire = 0;

bool task1Done = false;
bool task2Done = false;
bool task3Done = false;
bool task4Done = false;
bool task5Done = false;
bool task6Done = false;
bool allDone = false;
bool runMotor = false;
bool timerCounting = false;
bool rest = false;
bool timerStarted = false;
bool stopTimer = false;

String input1 = "";
String input2 = "";
String input3 = "";
String input4 = "";
String input5 = "";
String input6 = "";

String bufferString = "";
int counterInput = 0;
String c2 = "";
int timerCount = 0;
int timerStart = 0;
int delayBuffer = 1000;

signed short minutes = 40;
signed short seconds = 0;
char timeline[16];

void setup() {
  configureCommon(); // Setup pins for interrupt

  attachInterrupt(digitalPinToInterrupt(commonPin), pressInterrupt, FALLING);

  Serial.begin(9600);

  servo.attach(servoPin);
  servo.write(1);
  
  lcd1.init();
  lcd2.init();
  lcd3.init();
  lcd4.init();

  lcd1.backlight();
  lcd2.backlight();
  lcd3.backlight();
  lcd4.backlight();

  lcd1.setCursor(0, 0);
  lcd1.print("Task 1");
  lcd1.setCursor(0, 3);
  lcd1.print("Task 2");

  lcd2.setCursor(0, 0);
  lcd2.print("Task 3");
  lcd2.setCursor(0, 3);
  lcd2.print("Task 4");

  lcd3.setCursor(0, 0);
  lcd3.print("Task 5");
  lcd3.setCursor(0, 3);
  lcd3.print("Task 6");

  lcd4.setCursor(0, 0);
  lcd4.print("Time Left:");

  auger.setMaxSpeed(2000.0);
  auger.setAcceleration(200.0);
  auger.setSpeed(2000);
  auger.moveTo(2038);

  Serial.println("Ready");
}

void loop() {
    lcd3.setCursor(0,0);
    timerCount = millis();    
    
    if(Serial.available() > 0)
    {     
     char c = Serial.read();
     bufferString += c;
     if(c == '\n' && bufferString.charAt(0) == '1' && task1Done == false){
        input1 = bufferString;
        input1.remove(input1.length() -  1);
        input1.remove(0,1);
        
        lcd1.setCursor(0,0);
        lcd1.print("                    ");
        lcd1.setCursor(0,0);
        lcd1.print(input1);
        
        counterInput++;
        bufferString = "";
      }
      else if(c == '\n' && bufferString.charAt(0) == '2' && task2Done == false){
        input2 = bufferString;
        input2.remove(input2.length() - 1);
        input2.remove(0,1);
        
        lcd1.setCursor(0,3);
        lcd1.print("                    ");
        lcd1.setCursor(0,3);
        lcd1.print(input2);
        counterInput++;
        bufferString = "";
      }
      else if(c == '\n' && bufferString.charAt(0) == '3' && task3Done == false){
        input3 = bufferString;
        input3.remove(input3.length() - 1);
        input3.remove(0,1);
        
        lcd2.setCursor(0,0);
        lcd2.print("                    ");
        lcd2.setCursor(0,0);
        lcd2.print(input3);
        counterInput++;
        bufferString = "";
      }
      else if(c == '\n' && bufferString.charAt(0) == '4' && task4Done == false){
        input4 = bufferString;
        input4.remove(input4.length() - 1);
        input4.remove(0,1);
        
        lcd2.setCursor(0,3);
        lcd2.print("                    ");
        lcd2.setCursor(0,3);
        lcd2.print(input4);
        counterInput++;
        bufferString = "";
      }
      else if(c == '\n' && bufferString.charAt(0) == '5' && task5Done == false){
        input5 = bufferString;
        input5.remove(input5.length() - 1);
        input5.remove(0,1);
        
        lcd3.setCursor(0,0);
        lcd3.print("                    ");
        lcd3.setCursor(0,0);
        lcd3.print(input5);
        counterInput++;
        bufferString = "";
      }
      else if(c == '\n' && bufferString.charAt(0) == '6' && task6Done == false){
        input6 = bufferString;
        input6.remove(input6.length() - 1);
        input6.remove(0,1);
        
        lcd3.setCursor(0,3);
        lcd3.print("                    ");
        lcd3.setCursor(0,3);
        lcd3.print(input6);
        counterInput++;
        bufferString = "";
      }
      c2 = c;
     }

     if(timerCounting == false && c2 != ""){
      timerCounting = true;
      if(timerStarted = false){
        timerStart = millis();
        timerStarted = true;
        }
     }   

  
  if (!allDone){
    if (task1Done){
      lcd1.setCursor(0, 0);
      lcd1.print("Task Done           ");
    }
    if (task2Done){
      lcd1.setCursor(0, 3);
      lcd1.print("Task Done           ");
    }
    if (task3Done){
      lcd2.setCursor(0, 0);
      lcd2.print("Task Done           ");
    }
    if (task4Done){
      lcd2.setCursor(0, 3);
      lcd2.print("Task Done           ");
    }
    if (task5Done){
      lcd3.setCursor(0, 0);
      lcd3.print("Task Done           ");
    }
    if (task6Done){
      lcd3.setCursor(0, 3);
      lcd3.print("Task Done           ");
    }
    if ((task1Done == true) && (task2Done == true) && (task3Done == true) && (task4Done == true) && (task5Done == true) && (task6Done == true)){
      allDone = true;
      servo.write(180);
      
      for (int thisNote = 0; thisNote < notes * 2; thisNote = thisNote + 2) {
      // calculates the duration of each note
      divider = melody[thisNote + 1];
      if (divider > 0) {
        // regular note, just proceed
        noteDuration = (wholenote) / divider;
      } else if (divider < 0) {
        // dotted notes are represented with negative durations!!
        noteDuration = (wholenote) / abs(divider);
        noteDuration *= 1.5; // increases the duration in half for dotted notes
      }
  
      // we only play the note for 90% of the duration, leaving 10% as a pause
      tone(buzzer, melody[thisNote], noteDuration * 0.9);
  
      // Wait for the specief duration before playing the next note.
      delay(noteDuration);
  
      // stop the waveform generation before the next note.
      noTone(buzzer);
      }
    }
  }  
  
  if (runMotor == true){
    turnMotor();
    runMotor = false;
  }
  
  if (timerCounting && (timerCount - timerStart) >= delayBuffer && !stopTimer){
    lcd4.setCursor(0,1);
    sprintf(timeline,"%0.2d mins %0.2d secs", minutes, seconds);
    lcd4.print(timeline);

    if(seconds == 0){
      seconds = 59;
      minutes--;
    } else{
      seconds--;   
    }
    if (minutes == 0 && seconds == 0){
      minutes = 20;
      seconds = 0;
      lcd4.setCursor(0,0);
      lcd4.print("Time off:    ");

      servo.write(180);
    }
    if(allDone){
      stopTimer = true;
    }
    timerStart = timerCount;
  }
}

void pressInterrupt() { // ISR
  if (millis() - lastFire < 200) { // Debounce
    return;
  }
  lastFire = millis();

  configureDistinct(); // Setup pins for testing individual buttons

  for (int i = 0; i < sizeof(buttonPins) / sizeof(int); i++) { // Test each button for press
    if (!digitalRead(buttonPins[i])) {
      press(i);
    }
  }

  configureCommon(); // Return to original state
}

void configureCommon() {
  pinMode(commonPin, INPUT_PULLUP);

  for (int i = 0; i < sizeof(buttonPins) / sizeof(int); i++) {
    pinMode(buttonPins[i], OUTPUT);
    digitalWrite(buttonPins[i], LOW);
  }
}

void configureDistinct() {
  pinMode(commonPin, OUTPUT);
  digitalWrite(commonPin, LOW);

  for (int i = 0; i < sizeof(buttonPins) / sizeof(int); i++) {
    pinMode(buttonPins[i], INPUT_PULLUP);
  }
}

void press(int button) { // Our handler
  if ((button + 1) == 1 && !task1Done){
    task1Done = true;
    runMotor = true;
  } else if ((button + 1) == 2 && !task2Done){
    task2Done = true;
    runMotor = true;
  } else if ((button + 1) == 3 && !task3Done){
    task3Done = true;
    runMotor = true;
  } else if ((button + 1) == 4 && !task4Done){
    task4Done = true;
    runMotor = true;
  } else if ((button + 1) == 5 && !task5Done){
    task5Done = true;
    runMotor = true;
  } else if ((button + 1) == 6 && !task6Done){
    task6Done = true;
    runMotor = true;
  }
}

void turnMotor(){
  auger.move(2038);
  while (auger.isRunning()){
    auger.run();
  }
}
