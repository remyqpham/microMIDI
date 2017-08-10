/* Encoder Library - Basic Example
 * http://www.pjrc.com/teensy/td_libs_Encoder.html
 *
 * This example code is in the public domain.
 */
#include "Adafruit_TLC59711.h"
#include "MIDIUSB.h"

#include <Encoder.h>
#include <SPI.h>

#define NUM_TLC59711 1

#define data 16
#define clock 15

const int ButtonPin = 9;

int buttonState = 0;

Adafruit_TLC59711 tlc = Adafruit_TLC59711(1, clock, data);

// Change these two numbers to the pins connected to your encoder.
//   Best Performance: both pins have interrupt capability
//   Good Performance: only the first pin has interrupt capability
//   Low Performance:  neither pin has interrupt capability
Encoder myEncA(1, 14);
Encoder myEncB(0, A0);
Encoder myEncC(2,4);
//   avoid using pins with LEDs attached

char str[50];
long oldPositionA  = -999;
long oldPositionB  = -999;
long oldPositionC  = -999;

long oldTime = 0;
long newTime = 0;
int noteSend=0;

void setup() {
  Serial.begin(115200);
  Serial.println("Basic Encoder Test:");

  oldTime = millis();
  noteSend=0;

  pinMode(ButtonPin, INPUT);
  tlc.begin();
  tlc.write();
}




void loop() {
  
  buttonState = digitalRead(ButtonPin);
   
  long newPositionA = myEncA.read();
  long newPositionB = myEncB.read();
  long newPositionC = myEncC.read();
  if (newPositionA != oldPositionA && newPositionA>=0 && newPositionA <= 255) {
    oldPositionA = newPositionA;
    sprintf(str,"A:%d\n",newPositionA);
    colorWipe(pow(newPositionA,1.5),0,0,5);
    Serial.println(str);
  }
  if (newPositionB != oldPositionB && newPositionB>=0 && newPositionB <= 255) {
    oldPositionB = newPositionB;
    sprintf(str,"B:%d\n",newPositionB);
    colorWipe(0,pow(newPositionB,1.5),0,5);
    Serial.println(str);
  }
  if (newPositionC != oldPositionC && newPositionC>=0 && newPositionC <= 255) {
    oldPositionC = newPositionC;
    sprintf(str,"C:%d\n",newPositionC);
    colorWipe(0,0,pow(newPositionC,1.5),5);
    Serial.println(str);
  }


  //MIDI
  newTime = millis();

  if(buttonState==HIGH && noteSend==0){
    oldTime = newTime;
    midiEventPacket_t noteOn = {0x09, 0x90 | 0, 72, 64};    
    Serial.println("Sending note on");
    MidiUSB.sendMIDI(noteOn); //sending channel0, middle C, norm. velocity
    MidiUSB.flush();
    noteSend = 1;    
    
  }else if(buttonState==LOW && noteSend==1 && newTime-oldTime>10){
    midiEventPacket_t noteOff = {0x08, 0x80 | 0, 72, 64};
    Serial.println("Sending note off");
    MidiUSB.sendMIDI(noteOff);
    MidiUSB.flush();
    noteSend = 0;
    oldTime = newTime;
  }
  
  /*
  if((newTime - oldTime)>=1500 && noteSend == 0){
    midiEventPacket_t noteOn = {0x09, 0x90 | 0, 72, 64};
    Serial.println("Sending note on");
    MidiUSB.sendMIDI(noteOn); //sending channel0, middle C, norm. velocity
    MidiUSB.flush();
    noteSend = 1;
    oldTime = newTime;
  }
  else if((newTime - oldTime)>=500 && noteSend == 1){
    midiEventPacket_t noteOff = {0x08, 0x80 | 0, 72, 64};
    Serial.println("Sending note off");
    MidiUSB.sendMIDI(noteOff);
    MidiUSB.flush();
    noteSend = 0;
    oldTime = newTime;
  }
  */

  

  
}

// Fill the dots one after the other with a color
void colorWipe(uint16_t r, uint16_t g, uint16_t b, uint8_t wait) {
  for(uint16_t i=0; i<8*NUM_TLC59711; i++) {
      tlc.setLED(i, r, g, b);
      tlc.write();
      delay(wait);
  }
}
