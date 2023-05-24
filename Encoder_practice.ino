// Rotary Encoder Inputs
#include <iostream>
#include <math.h> 
#define CLK 0 //define the digital inputs for the rotary encoders and their digital pins 
#define DT 1
#define CLK_2 2 
#define DT_2 3 

float sample = 1; //variable to represent sampling frequency 
//int array[1023]; //establishing array with a size of 1023 bits 

int counter = 0;
int freq = 0; //new counter that represents the change in frequency 
int currentCLK; //checks what is currently the state of CLK
int currentCLK_2; 
int lastCLK; //saved last state of CLK 
int lastCLK_2; 

void setup() {

	// Set enco
	pinMode(CLK,INPUT);
	pinMode(DT,INPUT);
  pinMode(CLK_2, INPUT); 
  pinMode(DT_2, INPUT); 


	// Setup Serial Monitor
	Serial.begin(115200);

	// check the state of CLK on both encoders 
	lastCLK = digitalRead(CLK); //reads the input of the CLK pin, which is digital pin 0 
  lastCLK_2 = digitalRead(CLK_2); 
	
	// Call on the update() function when any high/low changed seen
	// on the interrrupts in digital pins 0-3 
	attachInterrupt(0, update, CHANGE); //CHANGE in this instance triggers the interrupt when the encoder increments or decrements! 
	attachInterrupt(1, update, CHANGE);
  attachInterrupt(2, update_2, CHANGE);
	attachInterrupt(3, update_2, CHANGE);

}

void loop() { //create a sine wave for testing purposes 
for(int x = 0; x < 1023; x++){ //create an array of 1023, 10-bits

      float i = (x)/sample; //divide the value of x by the sample 

      float omega = (float)freq*0.1; //2
   
   Serial.println(counter * sin(i*omega));
    delay(50); //delay the wave so we can clearly see the wave
  }
  }

void update(){
	// Read the current state of CLK
	currentCLK = digitalRead(CLK);

	// If last and current state of CLK are different, the encoder will emmit a pulse 
	// React to only 1 state change to avoid double count
	if (currentCLK != lastCLK  && currentCLK == 1){

		 //This condition compares DT to CLK. 
		if (digitalRead(DT) != currentCLK) {
			if (counter == 0){ 
        counter = 0; 
      } 
      else if(counter != 0){  //If DT differs, it's an indication that the knob is going counter clockwise. 
		  // this means we should decrement, which will allow for the wave to return to a previous state
        counter--; 
      }
		} else {
			// Encoder is rotating clockwise, so increment
			counter++; 
			
		}
    if(counter >= 5){ 
      counter = 5; //for testing purposes, we cap the limit for the amplitude to be 5 
    }
	}

	// Remember last CLK state, this keeps track of what the amplitude is 
	lastCLK = currentCLK;
}

void update_2(){
	// Read the current state of CLK
	currentCLK_2 = digitalRead(CLK_2);

	// If last and current state of CLK are different, then pulse occurred
	// React to only 1 state change to avoid double count
	if (currentCLK_2 != lastCLK_2  && currentCLK_2 == 1){

		// If the DT state is different than the CLK state then //If statement comparing the DT state and the CLK state
		if (digitalRead(DT_2) != currentCLK_2) {
			if (freq == 0){ 
        freq = 0; 
      } 
      else if(freq != 0){ //decrement if it's not 0, as it means it will be going counter clockwise 
        freq--; 
      }
      
		} else {
			freq++; 
	}

   if(freq >= 10){ 
      freq = 10; //limit the frequency change to 10 on the knob 
    }
    
}
	// Remember last CLK state
	lastCLK_2 = currentCLK_2;
}