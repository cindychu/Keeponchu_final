/* My Keepon Arduino controller
   Copyright © 2012 BeatBots LLC

   This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   For a copy of the GNU General Public License, see
   http://www.gnu.org/licenses/gpl.html

   For support, please post issues on the Github project page at
   http://github.com/BeatBots/MyKeepon

   Learn more about My Keepon at
   http://mykeepon.beatbots.net

   Keepon® is a trademark of BeatBots LLC.
*/

#include <Wire.h>

#include <String.h>
#include <avr/pgmspace.h>

#define cbi(sfr, bit) _SFR_BYTE(sfr) &= ~_BV(bit)
#define sbi(sfr, bit) _SFR_BYTE(sfr) |= _BV(bit)

#define MK_FREQ 49600L // Set clock to 50kHz (actualy 49.6kHz seems to work better)
#define SOUND (byte)0x52  // Sound controller (device ID 82).  Write to 0xA4, read from 0xA5.
#define BUTTON (byte)0x50 // Button controller (device ID 80). Write to 0xA0, read from 0xA1.
#define MOTOR (byte)0x55  // Motor controller (device ID 85).  Write to 0xAA, read from 0xAB.


const static char d1[] PROGMEM ="d1";
const static char d2[] PROGMEM ="d2";
const static char movepan0[] PROGMEM ="MOVE PAN <0>;";
const static char movepan_15[] PROGMEM ="MOVE PAN <-15>;";
const static char movepan15[] PROGMEM ="MOVE PAN <15>;";
const static char movepan25[] PROGMEM ="MOVE PAN <25>;";
const static char movepan60[] PROGMEM ="MOVE PAN <60>;";
const static char moveponup[] PROGMEM ="MOVE PON UP;";
const static char movepondown[] PROGMEM ="MOVE PON DOWN;";
const static char movesideright[] PROGMEM = "MOVE SIDE RIGHT;";
const static char movesideleft[] PROGMEM = "MOVE SIDE LEFT;";
const static char movesidecenter[] PROGMEM = "MOVE SIDE CENTER;";
const static char movesidec[] PROGMEM="MOVE SIDE CENTERFROMRIGHT;";
const static char movetilt_45[] PROGMEM="MOVE TILT <-45>;";
const static char movetilt_100[] PROGMEM ="MOVE TILT <-100>;";
const static char movetilt0[] PROGMEM  ="MOVE TILT <0>;";
const static char movetilt15[] PROGMEM="MOVE TILT <15>;";
const static char movetilt50[] PROGMEM="MOVE TILT <50>;";
const static char movetilt100[] PROGMEM ="MOVE TILT <100>;";
const static char speedponside50[] PROGMEM = "SPEED PONSIDE <50>;";
const static char speedponside200[] PROGMEM ="SPEED PONSIDE <200>;";
const static char speedponside255[] PROGMEM ="SPEED PONSIDE <255>;";
const static char speedtilt200[] PROGMEM="SPEED TILT <200>;";
const static char speedtilt50[] PROGMEM="SPEED TILT <50>;";

const static int num_actions[]={8,9,3,1,2,4,9,9,8,3,2};

const static char* const INTRO[] PROGMEM = {speedponside50,movesideright,d1,d1,d1,d1,speedponside200,movesidec};
const static char* const SURPRISE[] PROGMEM = {movepan0,speedponside200,moveponup,speedtilt200,movetilt0,d2,movetilt_100,d2,movetilt_100}; 
const static char* const FART[] PROGMEM = {moveponup,d1,moveponup}; 
const char* const LEFT[] PROGMEM = {movepan_15}; 
const char* const RIGHT[] PROGMEM = {movepan25,d1};
const char* const FARRIGHT[] PROGMEM = {movepan60,speedtilt200,d2,movetilt_100};
const char* const WIGGLE[] PROGMEM = {speedponside255, movesideright,d2, movesideleft, d2, movesideright, d2, movesideleft, movesidecenter}; 
const char* const NO[] PROGMEM = {movepan_15, d2, movepan15, d2, movepan_15, d2, movepan15, d2, movepan0}; 
const char* const YES[] PROGMEM={speedtilt200,movetilt_45,d2,movetilt15,d2,movetilt_45,d2,movetilt0};
const char* const CENTER[] PROGMEM={movepan0,speedtilt200,movetilt0}; 
const char* const END[] PROGMEM= {speedtilt50,movetilt100};

void setup()
{
  pinMode(SDA, OUTPUT); // Data wire on My Keepon
  pinMode(SCL, OUTPUT); // Clock wire on My Keepon
  digitalWrite(SDA, LOW);
  digitalWrite(SCL, LOW);
  Serial.begin(115200);
  while(!Serial);
}

void bootup()
{
  digitalWrite(SDA, LOW);
  digitalWrite(SCL, LOW);
  Serial.print(F("Waiting for My Keepon... "));
  while (analogRead(0) < 512); // Wait until we see voltage on A0 pin
  delay(3000);
  Serial.println(F("My Keepon detected."));
  Wire.begin();
  Serial.write((byte)0);
}

boolean isNextWord(char* msg, char* wordToCompare, int* i) {
  int j = 0;
  while (msg[j] == wordToCompare[j++]);
  if (j >= strlen(wordToCompare)) {
    *i = *i+j;
    return 1;
  } 
  else return 0;
}

int nextInt(char* msg) {
  int j = 0;
  int value = 0;
  int negative = 0;
  while (!(isDigit(msg[j]) || msg[j]=='-')) j++;
  if (msg[j] == '-') {
    negative = 1;
    j++;
  }
  while (isDigit(msg[j])) {
    value *= 10;
    value += msg[j++]-'0';
  }
  if (negative) value *= -1;
  return value;
}

boolean parseMsg(char* msg, byte* cmd, byte* device) {
  int i = 0, value;

  if (isNextWord(&msg[i], "SOUND", &i)) {
    *device = SOUND;
    if (isNextWord(&msg[i], "PLAY", &i)) {
      cmd[0] = 0x01;
      cmd[1] = B10000000 | (63 & nextInt(&msg[i]));
    } 
    else if (isNextWord(&msg[i], "REPEAT", &i)) {
      cmd[0] = 0x01;
      cmd[1] = B11000000 | (63 & nextInt(&msg[i]));
    } 
    else if (isNextWord(&msg[i], "DELAY", &i)) {
      cmd[0] = 0x03;
      cmd[1] = (byte)nextInt(&msg[i]);
    } 
    else if (isNextWord(&msg[i], "STOP", &i)) {
      cmd[0] = 0x01;
      cmd[1] = B00000000;
    } 
    else {
      Serial.println(F("Unknown command."));
      return false;
    }
  } 
  else if (isNextWord(&msg[i], "SPEED", &i)) {
    *device = MOTOR;
    if (isNextWord(&msg[i], "PAN", &i)) {
      cmd[0] = 5;
      cmd[1] = (byte)nextInt(&msg[i]);
    } 
    else if (isNextWord(&msg[i], "TILT", &i)) {
      cmd[0] = 3;
      cmd[1] = (byte)nextInt(&msg[i]);
    } 
    else if (isNextWord(&msg[i], "PONSIDE", &i)) {
      cmd[0] = 1;
      cmd[1] = (byte)nextInt(&msg[i]);
    } 
    else {
      Serial.println(F("Unknown command."));
      return false;
    }
  } 
  else if (isNextWord(&msg[i], "MOVE", &i)) {
    *device = MOTOR;
    if (isNextWord(&msg[i], "PAN", &i)) {
      cmd[0] = 4;
      cmd[1] = (byte)(nextInt(&msg[i]) + 127);
    } 
    else if (isNextWord(&msg[i], "TILT", &i)) {
      cmd[0] = 2;
      cmd[1] = (byte)(nextInt(&msg[i]) + 127);
    } 
    else if (isNextWord(&msg[i], "SIDE", &i)) {
      cmd[0] = 0;
      if (isNextWord(&msg[i], "CYCLE", &i))
        cmd[1] = 0;
      else if (isNextWord(&msg[i], "CENTERFROMLEFT", &i))
        cmd[1] = 1;
      else if (isNextWord(&msg[i], "RIGHT", &i))
        cmd[1] = 2;
      else if (isNextWord(&msg[i], "CENTERFROMRIGHT", &i))
        cmd[1] = 3;
      else if (isNextWord(&msg[i], "LEFT", &i))
        cmd[1] = 4;
      else {
        Serial.println(F("Unknown command."));
        return false;
      }
    } 
    else if (isNextWord(&msg[i], "PON", &i)) {
      cmd[0] = 0;
      if (isNextWord(&msg[i], "UP", &i))
        cmd[1] = -1;
      else if (isNextWord(&msg[i], "HALFDOWN", &i))
        cmd[1] = -2;
      else if (isNextWord(&msg[i], "DOWN", &i))
        cmd[1] = -3;
      else if (isNextWord(&msg[i], "HALFUP", &i))
        cmd[1] = -4;
      else {
        Serial.println(F("Unknown command."));
        return false;
      }
    } 
    else if (isNextWord(&msg[i], "STOP", &i)) {
      cmd[0] = 6;
      cmd[1] = 16;
    } 
    else {
      Serial.println(F("Unknown command."));
      return false;
    }    
  } 
  else if (isNextWord(&msg[i], "MODE", &i)) {
    if (isNextWord(&msg[i], "DANCE", &i)) {
      cmd[0] = 6;
      cmd[1] = 0;
    } 
    else if (isNextWord(&msg[i], "TOUCH", &i)) {
      cmd[0] = 6;
      cmd[1] = 1;
    } 
    else if (isNextWord(&msg[i], "TEMPO", &i)) {
      cmd[0] = 6;
      cmd[1] = 2;
    } 
    else if (isNextWord(&msg[i], "SLEEP", &i)) {
      cmd[0] = 6;
      cmd[1] = 240;
    } 
    else {
      Serial.println(F("Unknown command."));
      return false;
    }
  } 
  else {
    Serial.println(F("Unknown command."));
    return false;
  }
  return true;
}

boolean buttonState[8];
const char dance[] PROGMEM ="DANCE";
const char empty[] PROGMEM ="";
const char head[] PROGMEM ="HEAD";
const char touch[] PROGMEM ="TOUCH";
const char right[] PROGMEM ="RIGHT";
const char front[] PROGMEM ="FRONT";
const char left[] PROGMEM ="LEFT";
const char back[] PROGMEM ="BACK";
const char* const buttonName [] PROGMEM = {dance,empty,head,touch,right,front,left,back};
//char* buttonName[] = {
//  "DANCE", "", "HEAD", "TOUCH",
//  "RIGHT", "FRONT", "LEFT", "BACK"};

boolean motorState[8];
const char ponfinished[] PROGMEM ="PON FINISHED";
const char sidefinished[] PROGMEM ="SIDE FINISHED";
const char tiltfinished[] PROGMEM ="TILT FINISHED";
const char panfinished[] PROGMEM ="PAN FINISHED";
const char ponstalled[] PROGMEM ="PAN STALLED";
const char sidestalled[] PROGMEM ="SIDE STALLED";
const char tiltstalled[] PROGMEM ="TILT STALLED";
const char panstalled[] PROGMEM ="PAN STALLED";
const char* const motorName [] PROGMEM = {ponfinished,sidefinished,tiltfinished,panfinished,ponstalled,sidestalled,tiltstalled,panstalled};
//char* motorName[] = {
//  "PON FINISHED", "SIDE FINISHED", "TILT FINISHED", "PAN FINISHED",
//  "PON STALLED", "SIDE STALLED", "TILT STALLED", "PAN STALLED"};

int encoderState[4], audioState[5], emfState[3], positionState[3];
const char tiltnoreach[] PROGMEM ="TILT NOREACH";
const char tiltforward[] PROGMEM ="TILT FORWARD";
const char tiltback[] PROGMEM ="TILT BACK";
const char tiltup[] PROGMEM ="TILT UP";
const char ponhalfdown[] PROGMEM ="PON HALFDOWN";
const char ponup[] PROGMEM ="PON UP";
const char pondown[] PROGMEM ="PON DOWN";
const char ponhalfup[] PROGMEM ="PON HALFUP";
const char sidecenter[] PROGMEM ="SIDE CENTER";
const char sideleft[] PROGMEM ="SIDE LEFT";
const char sideright[] PROGMEM = "SIDE RIGHT";
const char panback[] PROGMEM ="PAN BACK";
const char panright[] PROGMEM ="PAN RIGHT";
const char panleft[] PROGMEM = "PAN LEFT";
const char pancenter[] PROGMEM ="PAN CENTER";
const char* const encoderName [] PROGMEM = {tiltnoreach,tiltforward,tiltback,tiltup,ponhalfdown,ponup,pondown,ponhalfup,sidecenter,sideleft,sideright,sidecenter,panback,panright,panleft,pancenter};
//char* encoderName[] = {
//  "TILT NOREACH", "TILT FORWARD", "TILT BACK", "TILT UP",
//  "PON HALFDOWN", "PON UP", "PON DOWN", "PON HALFUP",
//  "SIDE CENTER", "SIDE LEFT", "SIDE RIGHT", "SIDE CENTER",
//  "PAN BACK", "PAN RIGHT", "PAN LEFT", "PAN CENTER"};

unsigned long updatedButton = 0, updatedMotor = 0;
void query() {
  int i;
  byte buttonResponse, motorResponse;
  int intResponse;

  if (millis() - updatedButton > 100) {
    updatedButton = millis();
    Wire.requestFrom((int)BUTTON, 1);
    if (Wire.available() >= 1) {
      buttonResponse = Wire.read();
      for (i = 0; i < 8; i++) {
        if (i != 1) {
          if (buttonResponse & (1<<i)) {
            if (!buttonState[i]) {
              char buffer[6];
              Serial.print(F("BUTTON "));
              strcpy_P(buffer, (char*)pgm_read_word(&(buttonName[i]))); 
              Serial.print(buffer);
              Serial.println(F(" ON"));
              buttonState[i] = 1;
            }
          }
          else if (buttonState[i]) {
            char buffer[6];
            Serial.print(F("BUTTON "));
            strcpy_P(buffer, (char*)pgm_read_word(&(buttonName[i]))); 
            Serial.print(buffer);
            Serial.println(F(" OFF"));
            buttonState[i] = 0;
          }
        }
      }
    }
  }

  if (millis() - updatedMotor > 300) {
    updatedMotor = millis();
    Wire.requestFrom((int)MOTOR, 13);
    if (Wire.available() >= 13) {
      motorResponse = Wire.read();
      for (i = 0; i < 8; i++) {
        char buffer[14];
        if (motorResponse & (1<<i)) {
          if (!motorState[i]) {
            Serial.print(F("MOTOR "));
            strcpy_P(buffer, (char*)pgm_read_word(&(motorName[i]))); 
            Serial.println(buffer);
            motorState[i] = 1;
          }
        } 
        else if (motorState[i]) {
          motorState[i] = 0;
        }
      }
      motorResponse = Wire.read();
     if (motorResponse != audioState[0]) {
        //Serial.print("AUDIO TEMPO ");
        //Serial.println(motorResponse);
        audioState[0] = motorResponse;
      }
      motorResponse = Wire.read();
      if (motorResponse != audioState[1]) {
        //Serial.print("AUDIO MEAN ");
        //Serial.println(motorResponse);
        audioState[1] = motorResponse;
      }
      motorResponse = Wire.read();
      if (motorResponse != audioState[2]) {
        //Serial.print("AUDIO RANGE ");
        //Serial.println(motorResponse);
        audioState[2] = motorResponse;
      }
      motorResponse = Wire.read();
      for (i = 0; i < 4; i++) {
         char buffer[13];
        if ((motorResponse & (3<<(2*i))) != encoderState[i]) {
          encoderState[i] = motorResponse & (3<<(2*i));
          Serial.print(F("ENCODER "));
          strcpy_P(buffer, (char*)pgm_read_word(&(encoderName[4*i+(encoderState[i]>>(2*i))]))); 
          Serial.println(buffer);
        }
      }
      motorResponse = Wire.read();
      intResponse = motorResponse;
      if (intResponse > 0) intResponse -= 127;
      if (intResponse != emfState[0]) {
        Serial.print(F("EMF PONSIDE "));
        Serial.println(intResponse);
        emfState[0] = intResponse;
      }
      motorResponse = Wire.read();
      intResponse = motorResponse;
      if (intResponse > 0) intResponse -= 127;
      if (intResponse != emfState[1]) {
        Serial.print(F("EMF TILT "));
        Serial.println(intResponse);
        emfState[1] = intResponse;
      }
      motorResponse = Wire.read();
      intResponse = motorResponse;
      if (intResponse > 0) intResponse -= 127;
      if (intResponse != emfState[2]) {
        Serial.print(F("EMF PAN "));
        Serial.println(intResponse);
        emfState[2] = intResponse;
      }
      motorResponse = Wire.read();
      if (motorResponse != audioState[3]) {
        //      Serial.print("AUDIO ENVELOPE ");
        //      Serial.println(motorResponse);
        audioState[3] = motorResponse;
      }
      motorResponse = Wire.read();
      if (motorResponse != audioState[4]) {
        //Serial.print("AUDIO BPM ");
        //Serial.println(motorResponse);
        audioState[4] = motorResponse;
      }
      motorResponse = Wire.read();
      intResponse = motorResponse - 127;
      if (intResponse != positionState[0]) {
        Serial.print(F("POSITION PONSIDE "));
        Serial.println(intResponse);
        positionState[0] = intResponse;
      }
      motorResponse = Wire.read();
      intResponse = motorResponse - 127;
      if (intResponse != positionState[1]) {
        Serial.print(F("POSITION TILT "));
        Serial.println(intResponse);
        positionState[1] = intResponse;
      }
      motorResponse = Wire.read();
      intResponse = motorResponse - 127;
      if (intResponse != positionState[2]) {
        Serial.print(F("POSITION PAN "));
        Serial.println(intResponse);
        positionState[2] = intResponse;
      }
    }
  }
}

/*void groupOfMovements(String* arrayMov, byte* cmd, byte* device){
  String s;
  
  for (int i = 0; i < &arrayMov->length; i++){
    s = &arrayMov[i];
    char msg[32];
    
    for(int n = 0; n < s.length(); n++){
      msg[n] = s[n];
    }
    
    if (parseMsg(msg, cmd, &device)) {
      int result = 1;
      int attempts = 0;
      while (result != 0 && attempts++ < 50) {
        Wire.beginTransmission(device);
        Wire.write((byte)cmd[0]);
        Wire.write((byte)cmd[1]);
        result = (int)Wire.endTransmission();
      }
    }
  }
}*/


void loop() {
  char msg[32];
  byte device, cmd[2];
  char first;
  byte cust;
  byte del;
  
  
//  char* INTRO[] = {"SPEED PONSIDE <50>;", "MOVE SIDE RIGHT;", "d1", "d1", "d1", "d1","SPEED PONSIDE <200>;", "MOVE SIDE CENTERFROMRIGHT;"};
//  char* SURPRISE[] = {"MOVE PAN <0>;","SPEED PONSIDE <200>;", "MOVE PON UP;", "MOVE TILT <0>;","d2", "MOVE TILT <-100>;", "d2", "MOVE TILT <-100>;"}; 
//  char* FART[] = {"SPEED PONSIDE <150>;", "MOVE PON UP;", "d1", "MOVE PON UP;"};
//  char* LEFT[] = {"MOVE PAN <-15>;"};
//  char* RIGHT[] = {"MOVE PAN <25>;"};
//  char* FARRIGHT[] = {"MOVE PAN <60>;", "d2", "MOVE TILT <-100>"};
//  char* WIGGLE[] = {"SPEED PONSIDE <255>;", "MOVE SIDE RIGHT;", "d2", "MOVE SIDE LEFT;" "d2", "MOVE SIDE RIGHT;", "d2", "MOVE SIDE LEFT;", "MOVE SIDE CENTER;" };
//  char* NO[] = {"MOVE PAN <-15>;", "d2", "MOVE PAN <15>;", "d2", "MOVE PAN <-15>;","d2","MOVE PAN <15>;","d2", "MOVE PAN 0;"};
//  char* YES[] = {"SPEED TILT <200>;", "MOVE TILT <-45>;", "d2","MOVE TILT <15>;","d2", "MOVE TILT <-45>;", "d2", "MOVE TILT <0>;"};
//  char* CENTER[] = {"MOVE PAN <O>;", "MOVE TILT <0>;"};
//  char* END[] = {"SPEED TILT <50>;", "MOVE TILT <100>;", "SPEED PONSIDE <50>;", "MOVE SIDE RIGHT;"};
  
  
  bootup();
  
  while (analogRead(0) > 512) {
    query();
    cust = 0;
    if (Serial.available() > 0) {
        int i = 0;
      
        while ((msg[i++] = Serial.read()) != ';' && i < 30 && analogRead(0) > 512) {
          
          if ((msg[i-1] == 'i' || msg[i-1] == 'p' || msg[i-1] == 'x' || msg[i-1] == 'l' || msg[i-1] == 'r' || msg[i-1] == 't' || msg[i-1] == 'w' || msg[i-1] == 'n' || msg[i-1] == 'y' || msg[i-1] == 'c' || msg[i-1] == 'e') && i == 1){
             //String s;
             
             if(msg[i-1] == 'i' && i==1){
              char buffer[sizeof(movesidec)];
              for (int j = 0; j <num_actions[0] /*(sizeof(INTRO)/sizeof(INTRO[0])*/; j++){
                //num_actions[0]
                strcpy_P(buffer, (char*)pgm_read_word(&(INTRO[j]))); // Necessary casts and dereferencing, just copy.
                //s=buffer;
                //s = INTRO[j];
                if (buffer[0] == 'd'){
                  if (buffer[1] == '1'){
                    delay(1000);
                    Serial.println(F("Delay 1.0 second"));
                   }
                  if (buffer[1] == '2'){
                    delay(500);
                    Serial.println(F("Delay 0.5 second"));
                  }
                }
                else{
                  int n = 0;
                  while( n < sizeof(buffer)){
                    msg[n] = buffer[n];
                    n++;
                  }
                  msg[n] = '\0';
                  Serial.println(msg);

              
                  if (parseMsg(msg, cmd, &device)) {
                    int result = 1;
                    int attempts = 0;
                    while (result != 0 && attempts++ < 50) {
                      Wire.beginTransmission(device);
                      Wire.write((byte)cmd[0]);
                      Wire.write((byte)cmd[1]);
                      result = (int)Wire.endTransmission();
                    }
                  }
                }
              }
              cust = 1;
              break;
              }

             if(msg[i-1] == 'p'){
              char buffer[sizeof(movesidec)];
              for (int j = 0; j < (num_actions[1] /*sizeof(SURPRISE)/sizeof(SURPRISE[0])*/); j++){
                strcpy_P(buffer, (char*)pgm_read_word(&(SURPRISE[j]))); // Necessary casts and dereferencing, just copy.
                //s=buffer;
                //delay( 500 );
                //s = SURPRISE[j];
                
               if (buffer[0] == 'd'){
                  if (buffer[1] == '1'){
                    delay(1000);
                    Serial.println(F("Delay 1.0 second"));
                   }
                  if (buffer[1] == '2'){
                    delay(500);
                    Serial.println(F("Delay 0.5 second"));
                  }
                }
                else{
                  int n = 0;
                  while( n < sizeof(buffer)){
                    msg[n] = buffer[n];
                    n++;
                  }
                  msg[n] = '\0';
                  Serial.println(msg);
                 
              
                  if (parseMsg(msg, cmd, &device)) {
                    int result = 1;
                    int attempts = 0;
                    while (result != 0 && attempts++ < 50) {
                      Wire.beginTransmission(device);
                      Wire.write((byte)cmd[0]);
                      Wire.write((byte)cmd[1]);
                      result = (int)Wire.endTransmission();
                    }
                  }
                }
              }
              cust = 1;
              break;
             }
              if(msg[i-1] == 'x'){
              char buffer[sizeof(movesidec)];  
              for (int j = 0; j < num_actions[2] /*sizeof(FART)/sizeof(FART[0])*/; j++){
                
                strcpy_P(buffer, (char*)pgm_read_word(&(FART[j]))); // Necessary casts and dereferencing, just copy.
                //s=buffer;
                //s = FART[j];
                 if (buffer[0] == 'd'){
                  if (buffer[1] == '1'){
                    delay(1000);
                    Serial.println(F("Delay 1.0 second"));
                   }
                  if (buffer[1] == '2'){
                    delay(500);
                    Serial.println(F("Delay 0.5 second"));
                  }
                }
                else{
                  int n = 0;
                  while( n < sizeof(buffer)){
                    msg[n] = buffer[n];
                    n++;
                  }
                  msg[n] = '\0';
                  Serial.println(msg);
              
                  if (parseMsg(msg, cmd, &device)) {
                    int result = 1;
                    int attempts = 0;
                    while (result != 0 && attempts++ < 50) {
                      Wire.beginTransmission(device);
                      Wire.write((byte)cmd[0]);
                      Wire.write((byte)cmd[1]);
                      result = (int)Wire.endTransmission();
                    }
                  }
                }
              }
              cust = 1;
              break;
              }
            
             if(msg[i-1] == 'l'){
               char buffer[sizeof(movesidec)];
              for (int j = 0; j < num_actions[3] /*(sizeof(LEFT)/sizeof(LEFT[0]))*/; j++){
                strcpy_P(buffer, (char*)pgm_read_word(&(LEFT[j])));
                //s = LEFT[j];
                
                if (buffer[0] == 'd'){
                  if (buffer[1] == '1'){
                    delay(1000);
                    Serial.println(F("Delay 1.0 second"));
                   }
                  if (buffer[1] == '2'){
                    delay(500);
                    Serial.println(F("Delay 0.5 second"));
                  }
                }
                else{
                  int n = 0;
                  while( n < sizeof(buffer)){
                    msg[n] = buffer[n];
                    n++;
                  }
                  msg[n] = '\0';
                  Serial.println(msg);
              
                  if (parseMsg(msg, cmd, &device)) {
                    int result = 1;
                    int attempts = 0;
                    while (result != 0 && attempts++ < 50) {
                      Wire.beginTransmission(device);
                      Wire.write((byte)cmd[0]);
                      Wire.write((byte)cmd[1]);
                      result = (int)Wire.endTransmission();
                    }
                  }
                }
              }
              cust = 1;
              break;
              }
              
              if(msg[i-1] == 'r'){
                 char buffer[sizeof(movesidec)];
              for (int j = 0; j <num_actions[4] /*(sizeof(RIGHT)/sizeof(RIGHT[0]))*/; j++){
                strcpy_P(buffer, (char*)pgm_read_word(&(RIGHT[j])));
                //s = RIGHT[j];
                 if (buffer[0] == 'd'){
                  if (buffer[1] == '1'){
                    delay(1000);
                    Serial.println(F("Delay 1.0 second"));
                   }
                  if (buffer[1] == '2'){
                    delay(500);
                    Serial.println(F("Delay 0.5 second"));
                  }
                }
                else{
                  int n = 0;
                  while( n < sizeof(buffer)){
                    msg[n] = buffer[n];
                    n++;
                  }
                  msg[n] = '\0';
                  Serial.println(msg);
              
                  if (parseMsg(msg, cmd, &device)) {
                    int result = 1;
                    int attempts = 0;
                    while (result != 0 && attempts++ < 50) {
                      Wire.beginTransmission(device);
                      Wire.write((byte)cmd[0]);
                      Wire.write((byte)cmd[1]);
                      result = (int)Wire.endTransmission();
                    }
                  }
                }
              }
              cust = 1;
              break;
              }

             if(msg[i-1] == 't'){
               char buffer[sizeof(movesidec)];
              for (int j = 0; j <num_actions[5] /*(sizeof(FARRIGHT)/sizeof(FARRIGHT[0]))*/; j++){
               
                strcpy_P(buffer, (char*)pgm_read_word(&(FARRIGHT[j])));
                //s = FARRIGHT[j];
                 if (buffer[0] == 'd'){
                  if (buffer[1] == '1'){
                    delay(1000);
                    Serial.println(F("Delay 1.0 second"));
                   }
                  if (buffer[1] == '2'){
                    delay(500);
                    Serial.println(F("Delay 0.5 second"));
                  }
                }
                else{
                  int n = 0;
                  while( n < sizeof(buffer)){
                    msg[n] = buffer[n];
                    n++;
                  }
                  msg[n] = '\0';
                  Serial.println(msg);
              
                  if (parseMsg(msg, cmd, &device)) {
                    int result = 1;
                    int attempts = 0;
                    while (result != 0 && attempts++ < 50) {
                      Wire.beginTransmission(device);
                      Wire.write((byte)cmd[0]);
                      Wire.write((byte)cmd[1]);
                      result = (int)Wire.endTransmission();
                    }
                  }
                }
              }
              cust = 1;
              break;
              }
           
             if(msg[i-1] == 'w'){
               char buffer[sizeof(movesidec)];
              for (int j = 0; j < num_actions[6] /*(sizeof(WIGGLE)/sizeof(WIGGLE[0]))*/; j++){
                
                strcpy_P(buffer, (char*)pgm_read_word(&(WIGGLE[j])));
                //s = WIGGLE[j];
                 if (buffer[0] == 'd'){
                  if (buffer[1] == '1'){
                    delay(1000);
                    Serial.println(F("Delay 1.0 second"));
                   }
                  if (buffer[1] == '2'){
                    delay(500);
                    Serial.println(F("Delay 0.5 second"));
                  }
                }
                else{
                  int n = 0;
                  while( n < sizeof(buffer)){
                    msg[n] = buffer[n];
                    n++;
                  }
                  msg[n] = '\0';
                  Serial.println(msg);
              
                  if (parseMsg(msg, cmd, &device)) {
                    int result = 1;
                    int attempts = 0;
                    while (result != 0 && attempts++ < 50) {
                      Wire.beginTransmission(device);
                      Wire.write((byte)cmd[0]);
                      Wire.write((byte)cmd[1]);
                      result = (int)Wire.endTransmission();
                    }
                  }
                }
              }
              cust = 1;
              break;
              }
            if(msg[i-1] == 'n'){
               char buffer[sizeof(movesidec)];
              for (int j = 0; j <num_actions[7] /* (sizeof(NO)/sizeof(NO[0]))*/; j++){
                
                strcpy_P(buffer, (char*)pgm_read_word(&(NO[j])));
                //s = NO[j];
                 if (buffer[0] == 'd'){
                  if (buffer[1] == '1'){
                    delay(1000);
                    Serial.println(F("Delay 1.0 second"));
                   }
                  if (buffer[1] == '2'){
                    delay(500);
                    Serial.println(F("Delay 0.5 second"));
                  }
                }
                else{
                  int n = 0;
                  while( n < sizeof(buffer)){
                    msg[n] = buffer[n];
                    n++;
                  }
                  msg[n] = '\0';
                  Serial.println(msg);
              
                  if (parseMsg(msg, cmd, &device)) {
                    int result = 1;
                    int attempts = 0;
                    while (result != 0 && attempts++ < 50) {
                      Wire.beginTransmission(device);
                      Wire.write((byte)cmd[0]);
                      Wire.write((byte)cmd[1]);
                      result = (int)Wire.endTransmission();
                    }
                  }
                }
              }
              cust = 1;
              break;
              }
        
         if(msg[i-1] == 'y'){
           char buffer[sizeof(movesidec)];
              for (int j = 0; j < num_actions[8] /*(sizeof(YES)/sizeof(YES[0]))*/; j++){
                
                strcpy_P(buffer, (char*)pgm_read_word(&(YES[j])));
                //s = YES[j];
                 if (buffer[0] == 'd'){
                  if (buffer[1] == '1'){
                    delay(1000);
                    Serial.println(F("Delay 1.0 second"));
                   }
                  if (buffer[1] == '2'){
                    delay(500);
                    Serial.println(F("Delay 0.5 second"));
                  }
                }
                else{
                  int n = 0;
                  while( n < sizeof(buffer)){
                    msg[n] = buffer[n];
                    n++;
                  }
                  msg[n] = '\0';
                  Serial.println(msg);
              
                  if (parseMsg(msg, cmd, &device)) {
                    int result = 1;
                    int attempts = 0;
                    while (result != 0 && attempts++ < 50) {
                      Wire.beginTransmission(device);
                      Wire.write((byte)cmd[0]);
                      Wire.write((byte)cmd[1]);
                      result = (int)Wire.endTransmission();
                    }
                  }
                }
              }
              cust = 1;
              break;
              }

          if(msg[i-1] == 'c' && i==1){
             char buffer[sizeof(movesidec)];
              for (int j = 0; j <num_actions[9] /* (sizeof(CENTER)/sizeof(CENTER[0]))*/; j++){
                
                strcpy_P(buffer, (char*)pgm_read_word(&(CENTER[j])));
                //s = CENTER[j];
                 if (buffer[0] == 'd'){
                  if (buffer[1] == '1'){
                    delay(1000);
                    Serial.println(F("Delay 1.0 second"));
                   }
                  if (buffer[1] == '2'){
                    delay(500);
                    Serial.println(F("Delay 0.5 second"));
                  }
                }
                else{
                  int n = 0;
                  while( n < sizeof(buffer)){
                    msg[n] = buffer[n];
                    n++;
                  }
                  msg[n] = '\0';
                  Serial.println(msg);
              
                  if (parseMsg(msg, cmd, &device)) {
                    int result = 1;
                    int attempts = 0;
                    while (result != 0 && attempts++ < 50) {
                      Wire.beginTransmission(device);
                      Wire.write((byte)cmd[0]);
                      Wire.write((byte)cmd[1]);
                      result = (int)Wire.endTransmission();
                    }
                  }
                }
              }
              cust = 1;
              break;
              }

          if(msg[i-1] == 'e' && i==1){
             char buffer[sizeof(movesidec)];
              for (int j = 0; j < num_actions[10] /*(sizeof(ANGLE)/sizeof(ANGLE[0]))*/; j++){
                
                strcpy_P(buffer, (char*)pgm_read_word(&(END[j])));
                //s = END[j];
                 if (buffer[0] == 'd'){
                  if (buffer[1] == '1'){
                    delay(1000);
                    Serial.println(F("Delay 1.0 second"));
                   }
                  if (buffer[1] == '2'){
                    delay(500);
                    Serial.println(F("Delay 0.5 second"));
                  }
                }
                else{
                  int n = 0;
                  while( n < sizeof(buffer)){
                    msg[n] = buffer[n];
                    n++;
                  }
                  msg[n] = '\0';
                  Serial.println(msg);
              
                  if (parseMsg(msg, cmd, &device)) {
                    int result = 1;
                    int attempts = 0;
                    while (result != 0 && attempts++ < 50) {
                      Wire.beginTransmission(device);
                      Wire.write((byte)cmd[0]);
                      Wire.write((byte)cmd[1]);
                      result = (int)Wire.endTransmission();
                    }
                  }
                }
              }
              cust = 1;
              break;
              }
          }  
          
          while (Serial.available() <= 0 && analogRead(0) > 512); 
        }

        if (cust == 0){
          msg[i] = '\0';
          if (parseMsg(msg, cmd, &device)) {
            int result = 1;
            int attempts = 0;
            while (result != 0 && attempts++ < 50) {
              Wire.beginTransmission(device);
              Wire.write((byte)cmd[0]);
              Wire.write((byte)cmd[1]);
              result = (int)Wire.endTransmission();
            }
          }
          Serial.println(F("STANDARD COMMAND"));
        }
    }
  }
}


