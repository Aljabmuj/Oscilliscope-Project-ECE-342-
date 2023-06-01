// Rotary Encoder Inputs
#include <iostream>
#include <math.h>
#include <SPI.h>
#include <SD.h>

int pinCS = 44;
int input_p = 10; 

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
}
//}

void loop() {                       //create a sine wave for testing purposes


  for (int x = 0; x < 1023; x++) {  //create an array of 1023, 10-bits

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
