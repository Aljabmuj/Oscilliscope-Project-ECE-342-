// Rotary Encoder Inputs
#include <iostream>
#include <math.h>
#include <SPI.h>
#include <SD.h>

float freq;
float sample;
float omega;

int pinCS = 44;
int input_p = 10; 
int state; 
int last = LOW; 

File myFile;


void setup() {


  pinMode(pinCS, OUTPUT);
  // Setup Serial Monitor
  Serial.begin(115200);
  //while(!Serial){
  //;
  //}

  button(); 

  if (SD.begin(BUILTIN_SDCARD)) {
    Serial.println("SD card is ready to use.");
  } else {
    Serial.println("SD card initialization failed");
    return;
  }

  //Serial.print("Initializing....");

  //if(!SD.begin(BUILTIN_SDCARD)){
  //Serial.println("mission failed");
  //while (1);
  //}
  //Serial.println("complete.");

  //myFile = SD.open("test.csv", FILE_WRITE);
  // if the file opened okay, write to it:

  //if (myFile) {

  //Serial.print("Writing to test.csv...");

  //myFile.println("testing 1, 2, 3.");

  // close the file:

  //myFile.close();

  //Serial.println("done.");

  // } else {

  // if the file didn't open, print an error:

  // Serial.println("error opening test.csv");

  // }


  // re-open the file for reading:

  //myFile = SD.open("test.csv");

  //if (myFile) {

  //Serial.println("test.csv:");


  // read from the file until there's nothing else in it:

  //while (myFile.available()) {

  //Serial.write(myFile.read());

  //}

  // close the file:

  //myFile.close();

  //} else {

  // if the file didn't open, print an error:

  // Serial.println("error opening test.csv");
}
//}

void loop() {                       //create a sine wave for testing purposes


  for (int x = 0; x < 1023; x++) {  //create an array of 1023, 10-bits

    //float i = (x) / sample;  //divide the value of x by the sample
    //float i = x; 

    float i = x; 

    myFile = SD.open("test.csv", FILE_WRITE);
    if (myFile) {
      myFile.print(sin(i));
      myFile.print(","); 
      myFile.println(i); 
      myFile.close();
    } else {
      Serial.println("error opening test.csv");
    }
    delay(50);
  }
  //float omega = (float)freq*0.1; //2

  //Serial.println(counter * sin(i*omega));
  //Serial.println(sin(i));
  //delay(50); //delay the wave so we can clearly see the wave
}

void button(){ 
  int push = digitalRead(input_p); 
  pinMode(input_p, INPUT); 
  if(push == HIGH){ 
    SD.begin(BUILTIN_SDCARD); 
  }
  else{
    button();
  }
  delay(10); 
}
