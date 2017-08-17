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

const int ButtonPin [NUM_BUTTONS] = {9,8,7,6};

int buttonState [NUM_BUTTONS]; //1 is little button, 2+3+4 are encoders for now

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
    pinMode(ButtonPin[i], INPUT);
  }

  tlc.begin();
  tlc.write();
}

void loop() {

  for (int i=0; i<NUM_BUTTONS; i++){
    buttonState[i]=digitalRead(ButtonPin[i]);
  }
   
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
    buttonInput(i);
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

void buttonInput(int b){
  if(buttonState[b]==HIGH && noteSend[b]==0 && newTime[b]-oldTime[b]>DEBOUNCE){
    oldTime[b] = newTime[b];
    midiEventPacket_t noteOn = {0x09, 0x90 | 0, 79+b, 64};  //stupid test  
    //midiEventPacket_t noteOn = {0x0B, 0xB0 | 0, 80+b, 127};
    Serial.println("Sending note on");
    MidiUSB.sendMIDI(noteOn); //sending channel0, G4, norm. velocity
    MidiUSB.flush();
    noteSend[b] = 1;    
    
  }else if(buttonState[b]==LOW && noteSend[b]==1 && newTime[b]-oldTime[b]>DEBOUNCE){
    midiEventPacket_t noteOff = {0x08, 0x80 | 0, 79+b, 64};
    //midiEventPacket_t noteOff = {0x0B, 0xB0 | 0, 80+b, 0};
    Serial.println("Sending note off");
    MidiUSB.sendMIDI(noteOff);
    MidiUSB.flush();
    noteSend[b] = 0;
    oldTime[b] = newTime[b];
  }
   
}

