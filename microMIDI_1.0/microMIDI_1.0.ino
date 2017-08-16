/* microMIDI 1.0
 * Remy Pham
 * Patrick Ayers
 */

#include "Adafruit_TLC59711.h"
#include "MIDIUSB.h"

#include <Encoder.h>
#include <SPI.h>

#define NUM_TLC59711 1
#define NUM_BUTTONS 4
#define DEBOUNCE 50

#define data 16
#define clock 15

const int ButtonPin = 9; //lil button
const int EncoderPin1 = 8;//encoder 1
const int EncoderPin2 = 7;//encoder 2
const int EncoderPin3 = 6;//encoder 3

int buttonState = 0;
int encoder1ButtonState = 0;
int encoder2ButtonState = 0;
int encoder3ButtonState = 0;


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

long oldTime[NUM_BUTTONS];
long newTime[NUM_BUTTONS];
uint8_t noteSend [NUM_BUTTONS] ;

void setup() {
  //Setting up the MIDI baud rate
  Serial.begin(115200);

  for(int i=0; i<NUM_BUTTONS; i++){
    noteSend[i]=newTime[i]=0;
    oldTime[i] = millis();
  }


  pinMode(ButtonPin, INPUT);
  pinMode(EncoderPin1, INPUT);
  pinMode(EncoderPin2, INPUT);
  pinMode(EncoderPin3, INPUT);
  tlc.begin();
  tlc.write();
}

void loop() {
  
  buttonState = digitalRead(ButtonPin);
  encoder1ButtonState = digitalRead(EncoderPin1);
  encoder2ButtonState = digitalRead(EncoderPin2);
  encoder3ButtonState = digitalRead(EncoderPin3);
   
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
  for(int i=0; i<NUM_BUTTONS; i++){
    newTime[i] = millis();
  }

  //button MIDI
  if(buttonState==HIGH && noteSend[0]==0 && newTime[0]-oldTime[0]>DEBOUNCE){
    oldTime[0] = newTime[0];
    midiEventPacket_t noteOn = {0x09, 0x90 | 0, 72, 64};    
    Serial.println("Sending note on");
    MidiUSB.sendMIDI(noteOn); //sending channel0, middle C, norm. velocity
    MidiUSB.flush();
    noteSend[0] = 1;    
    
  }else if(buttonState==LOW && noteSend[0]==1 && newTime[0]-oldTime[0]>DEBOUNCE){
    midiEventPacket_t noteOff = {0x08, 0x80 | 0, 72, 64};
    Serial.println("Sending note off");
    MidiUSB.sendMIDI(noteOff);
    MidiUSB.flush();
    noteSend[0] = 0;
    oldTime[0] = newTime[0];
  }

  
  //encoder 1 button MIDI
    if(encoder1ButtonState==HIGH && noteSend[1]==0 && newTime[1]-oldTime[1]>DEBOUNCE){
    oldTime[1] = newTime[1];
    midiEventPacket_t noteOn = {0x09, 0x90 | 0, 74, 64};    
    Serial.println("Sending note on");
    MidiUSB.sendMIDI(noteOn); //sending channel0, D4, norm. velocity
    MidiUSB.flush();
    noteSend[1] = 1;    
    
  }else if(encoder1ButtonState==LOW && noteSend[1]==1 && newTime[1]-oldTime[1]>DEBOUNCE){
    midiEventPacket_t noteOff = {0x08, 0x80 | 0, 74, 64};
    Serial.println("Sending note off");
    MidiUSB.sendMIDI(noteOff);
    MidiUSB.flush();
    noteSend[1] = 0;
    oldTime[1] = newTime[1];
  }


  //encoder 2 button MIDI
    if(encoder2ButtonState==HIGH && noteSend[2]==0 && newTime[2]-oldTime[2]>DEBOUNCE){
    oldTime[2] = newTime[2];
    midiEventPacket_t noteOn = {0x09, 0x90 | 0, 76, 64};    
    Serial.println("Sending note on");
    MidiUSB.sendMIDI(noteOn); //sending channel0, E4, norm. velocity
    MidiUSB.flush();
    noteSend[2] = 1;    
    
  }else if(encoder2ButtonState==LOW && noteSend[2]==1 && newTime[2]-oldTime[2]>DEBOUNCE){
    midiEventPacket_t noteOff = {0x08, 0x80 | 0, 76, 64};
    Serial.println("Sending note off");
    MidiUSB.sendMIDI(noteOff);
    MidiUSB.flush();
    noteSend[2] = 0;
    oldTime[2] = newTime[2];
  }
  
  //encoder 3 button MIDI
    if(encoder3ButtonState==HIGH && noteSend[3]==0 && newTime[3]-oldTime[3]>DEBOUNCE){
    oldTime[3] = newTime[3];
    midiEventPacket_t noteOn = {0x09, 0x90 | 0, 79, 64};    
    Serial.println("Sending note on");
    MidiUSB.sendMIDI(noteOn); //sending channel0, G4, norm. velocity
    MidiUSB.flush();
    noteSend[3] = 1;    
    
  }else if(encoder3ButtonState==LOW && noteSend[3]==1 && newTime[3]-oldTime[3]>DEBOUNCE){
    midiEventPacket_t noteOff = {0x08, 0x80 | 0, 79, 64};
    Serial.println("Sending note off");
    MidiUSB.sendMIDI(noteOff);
    MidiUSB.flush();
    noteSend[3] = 0;
    oldTime[3] = newTime[3];
  }

}



// Fill the dots one after the other with a color
void colorWipe(uint16_t r, uint16_t g, uint16_t b, uint8_t wait) {
  for(uint16_t i=0; i<8*NUM_TLC59711; i++) {
      tlc.setLED(i, r, g, b);
      tlc.write();
      delay(wait);
  }
}


