/* microMIDI 1.0
   Remy Pham
   Patrick Ayers
*/

#include "MIDIUSB.h"
#include <Encoder.h>
#include <SPI.h>

#define NUM_BUTTONS 6
#define NUM_ENCODERS 4 //to be changed to four later
#define NUM_BANKS 16
#define DEBOUNCE 50
#define data 16
#define clock 15
#define ORIGINAL 0
#define TRANSLATED 1
#define OLD 0
#define NEW 1

const int ButtonPin [NUM_BUTTONS] = {9, 8, 7, 6, 5, 10};
int buttonState [NUM_BUTTONS]; //1 is little button, 2+3+4 are encoders for now, 5 is bank navigation (up one) and 6 is bank navi (down one)
int bank = 1;

// Change these two numbers to the pins connected to your encoder.
//   Best Performance: both pins have interrupt capability
//   Good Performance: only the first pin has interrupt capability
//   Low Performance:  neither pin has interrupt capability

Encoder myEncA(0, 10);
Encoder myEncB(1, 9);
Encoder myEncC(2, 8);
Encoder myEncD(3, 6);

//   avoid using pins with LEDs attached

char str[50];
long encPosition [2][NUM_ENCODERS][2][NUM_BANKS+1];//encPosition[OLD or NEW value][which encoder is sending data, referred to as 'i'][ORIGINAL or TRANSLATED value][bank indicator 1-16 (0 is original bank)]
long oldTime[NUM_BUTTONS];
long newTime[NUM_BUTTONS];
uint8_t noteSend [NUM_BUTTONS] ;

void setup() {

  Serial.begin(115200);                   //Setting up the MIDI baud rate

  for (int i = 0; i < NUM_BUTTONS; i++) {
    noteSend[i] = newTime[i] = 0;
    oldTime[i] = millis();
    pinMode(ButtonPin[i], INPUT);
  }

  for (int i = 0; i < NUM_ENCODERS; i++) { //set the encoder to begin on startup in the middle of the range
    for (int b = 0; b < NUM_BANKS+1; b++){ // da whole damn array gets filled yo
      encPosition[OLD][i][ORIGINAL][b] = 255;
      encPosition[OLD][i][TRANSLATED][b] = (511 - encPosition[OLD][i][ORIGINAL][b]) / 4;
      encPosition[NEW][i][TRANSLATED][b] = (511 - encPosition[NEW][i][ORIGINAL][b]) / 4;
      writeEncoder(i, 255);//initial encoder value (original)
    }
  }
}

void loop() {

  initializeButtons();
  initializeEncoders();

  readButtons();
  readBankSelector();

}

void readButtons() {  //MIDI
  for (int i = 0; i < NUM_BUTTONS - 2; i++) { //ignore last two buttons for bank
    newTime[i] = millis();
    buttonInput(i);
  }
}

void readBankSelector() {
  for (int i = NUM_BUTTONS - 2; i < NUM_BUTTONS; i++) { //bank buttons
    newTime[i] = millis();
    bankSelectorInput(i);    
  }
}

void initializeButtons() {
  for (int i = 0; i < NUM_BUTTONS; i++) {
    buttonState[i] = digitalRead(ButtonPin[i]);
  }
}
//int test = 0;
void initializeEncoders() {

  encPosition[NEW][0][ORIGINAL][bank] = myEncA.read();
  encPosition[NEW][1][ORIGINAL][bank] = myEncB.read();
  encPosition[NEW][2][ORIGINAL][bank] = myEncC.read();
  encPosition[NEW][3][ORIGINAL][bank] = myEncD.read();

  for (int i = 0; i < NUM_ENCODERS; i++) {          //constrain the values
    if (encPosition[NEW][i][ORIGINAL][bank] < 0) {
      encPosition[NEW][i][ORIGINAL][bank] = 0;
      encPosition[NEW][i][TRANSLATED][bank] = 0;
      writeEncoder(i, 0);
    } else if (encPosition[NEW][i][ORIGINAL][bank] > 511) {
      encPosition[NEW][i][ORIGINAL][bank] = 511;
      encPosition[NEW][i][TRANSLATED][bank] = (511 - encPosition[NEW][i][ORIGINAL][bank]) / 4;
      writeEncoder(i, 511);
    }
   
    if (encPosition[NEW][i][ORIGINAL][bank] != encPosition[OLD][i][ORIGINAL][bank]) {
      
      encPosition[OLD][i][ORIGINAL][bank] = encPosition[NEW][i][ORIGINAL][bank];
      encPosition[OLD][i][TRANSLATED][bank] = (511 - encPosition[OLD][i][ORIGINAL][bank]) / 4;
           
      if (encPosition[NEW][i][ORIGINAL][bank] % 4 == 3 && encPosition[NEW][i][TRANSLATED][bank] != encPosition[OLD][i][TRANSLATED][bank]) { //filter out extraneous outputs. 3 chosen for end of notch
              
        sprintf(str, "Bank #%d        Encoder #%d's value = %d", bank, i, encPosition[OLD][i][TRANSLATED][bank]); //encPosition[OLD][i][TRANSLATED] is the desired value
        Serial.println(str);
        encPosition[NEW][i][TRANSLATED][bank] = encPosition[OLD][i][TRANSLATED][bank];      
        
        midiEventPacket_t controlChange = {0x0B, 0xB0 | (bank - 1) * 4 + i, 1, encPosition[OLD][i][TRANSLATED][bank]}; //push value in that particular bank
        MidiUSB.sendMIDI(controlChange);
        MidiUSB.flush();
      }      
    }
  }
}

void buttonInput(int b) {         //IF BANK IS CHANGED WHILE BUTTON STATE IS HIGH THE SECOND IF STATEMENT DOESN'T OCCUR. FIX
  if (buttonState[b] == HIGH && noteSend[b] == 0 && newTime[b] - oldTime[b] > DEBOUNCE) {
    oldTime[b] = newTime[b];
    midiEventPacket_t noteOn = {0x09, 0x90 | 0, (bank * 4) + b, 64}; //plays a certain note based on bank value
    Serial.println("Sending note on");
    MidiUSB.sendMIDI(noteOn); //sending channel0, G4, norm. velocity
    MidiUSB.flush();
    noteSend[b] = 1;
  } else if (buttonState[b] == LOW && noteSend[b] == 1 && newTime[b] - oldTime[b] > DEBOUNCE) {
    midiEventPacket_t noteOff = {0x08, 0x80 | 0, (bank * 4) + b, 64};
    Serial.println("Sending note off");
    MidiUSB.sendMIDI(noteOff);
    MidiUSB.flush();
    noteSend[b] = 0;
    oldTime[b] = newTime[b];
  }
}

void writeEncoder(int i, int val) {
  switch (i) {
    case 0:
      myEncA.write(val);
      break;
    case 1:
      myEncB.write(val);
      break;
    case 2:
      myEncC.write(val);
      break;
    case 3:
      myEncD.write(val);
    default:
      break;
  }
  return;
}

void bankSelectorInput(int i) {
  if (buttonState[i] == HIGH && noteSend[i] == 0 && newTime[i] - oldTime[i] > DEBOUNCE) {
    oldTime[i] = newTime[i];
    if (i == NUM_BUTTONS - 2 && bank < 16) {  //"Next bank" button is second from the last
      bank = bank + 1;
      Serial.print("Current Bank: ");
      Serial.println(bank);
      updateEncoders(); 
      
    } else if (i == NUM_BUTTONS - 1 && bank > 1) { //"Previous bank" button is last
      bank = bank - 1;
      Serial.print("Current Bank: ");
      Serial.println(bank);
      updateEncoders();
    }
    noteSend[i] = 1;
  } else if (buttonState[i] == LOW && noteSend[i] == 1 && newTime[i] - oldTime[i] > DEBOUNCE) {
      noteSend[i] = 0;
      oldTime[i] = newTime[i];
  }
}

/*Bring the encoders up to date with the current bank.  Called in bankSelectorInput()*/
void updateEncoders(){
    for(int i=0; i<NUM_ENCODERS; i++){
      writeEncoder(i, encPosition[OLD][i][ORIGINAL][bank]); //reset encoders to current bank values   
    }
}
