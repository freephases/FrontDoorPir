/*
  For RF module, connect uno/pro mini/campatible etc:

  Pin 9 - CE
  Pin 10 - CS(N)
  Pin 11 - MOSI
  Pin 12 - MISO
  Pin 13 - SCK

  For RF connect mega:

  Pin 9 - CE
  Pin 53 - CS(N)
  Pin 51 - MOSI
  Pin 50 - MISO
  Pin 52 - SCK

  Both:

  3.3v - VCC
  GND - GND

  For LED TM1637 module:

  Pin 4 - CLK
  Pin 5 - DIO

  LED is digital pin 3, on when PIR sensor is activated
*/
#include  <SPI.h>
#include "nRF24L01.h"
#include <RF24.h>
#include <RF24_config.h>
#include <OnOff.h>

OnOff led(3);

int inputPin = 2;               // choose the input pin (for PIR sensor)
int pirState = LOW;             // we start, assuming no motion detected
int pirVal = 0;                    // variable for reading the pin status
int onState = 0; //0= off, 1=on
unsigned long motionStartedMillis = 100;
unsigned long motionEndedMillis = 0;


uint8_t stateBuffer[12] = {80, 70, 0, 0, 0,0, 0, 0, 0, 0, 0, 68}; 
RF24 radio(9, 10);
const uint64_t pipe = 0xE8E8F0F066LL;//last bytes to change for each PIR

unsigned long lastPirCheckMillis = 0;

bool dataToSend = false;


void setState(uint8_t port, uint8_t state)
{
    if (port>9||port<1) return;
    stateBuffer[1+port]=state;
    dataToSend = true;
}

void sendStateOverRadio()
{  
  if (dataToSend) {
    bool res = radio.write(stateBuffer, sizeof(stateBuffer));
    if (res) {
      dataToSend = false;
    }
  }
}


void pirSetup()
{
  pinMode(inputPin, INPUT);     // declare sensor as input
}

void pirProcess()
{
  if (onState != 0 && motionEndedMillis != 0 && millis() - motionEndedMillis > 15000) {
    onState = 0;
    led.off();
    setState(1, 0);
    
  } else if (onState != 1 && motionStartedMillis != 0 && millis() - motionStartedMillis > 1500
            ) {
    onState = 1;
    led.on();
    setState(1, 1);
  }
}

void pirLoop()
{  
  if (lastPirCheckMillis == 0 || millis() - lastPirCheckMillis > 250) {
    lastPirCheckMillis = millis();
    pirVal = digitalRead(inputPin);  
    if (pirVal == HIGH) {
      if (pirState == LOW) {
        // we have just turned on
        // Serial.println("Motion detected!");
        motionStartedMillis = millis();
        motionEndedMillis = 0;
        // We only want to print on the output change, not state
        pirState = HIGH;
      }
    } else {
      if (pirState == HIGH) {
        // we have just turned of
        //Serial.println("Motion ended!");
        motionEndedMillis = millis();
        motionStartedMillis = 0;
        // We only want to print on the output change, not state
        pirState = LOW;
      }
    }
  }
}



void setup()
{
  radio.begin();
  radio.setRetries(7, 12);//does not work on mini well
  radio.openWritingPipe(pipe);
 // Serial.begin(9600);
  led.on();
  pirSetup();
  delay(500);
  led.off();
}

void loop()
{
  pirLoop();
  pirProcess();
  sendStateOverRadio();
}




