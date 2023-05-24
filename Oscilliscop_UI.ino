
#include <iostream> 
float probe1; 
float probe2; 


//int v_array[1023]; 
//int t_array[1023]; 

void setup() {
//Serial.begin(1000000); //setting up a baud rate of 1000000, which allows 1 Mbps 
Serial.begin(115200);
pinMode(A0, INPUT);                 //set up pin A1 for ADC conversion 
pinMode(A1, INPUT);     
 

//for (int x = 0; x < 1023; x++){ //Original attempt of creating a vector using two arrays 
 // v_array[x]= random(0,3.3); 
  //int voltage = v_array[x]; 
//}

//for (int y = 0; y < 1023; y++){ 
  //t_array[y]= y++; 
  //int time = t_array[y]; 
//}
}

//int *vector[] = {voltage , time}; 

//int function = sin(vector); 

void loop() {
   probe1 = analogRead(A0);  //read data generated from A0 pin and converted into radians
   //float voltage = probe1 * (3.3/1023); 
   //probe2 = analogRead(A2); 
  // test = sin(probe1); 
  //Serial.print("Frequency:");   //label the frequency input 
 // Serial.println(test); 
  //Serial.println(probe1);   //Identify the input for the serial plotter 

  //float voltage1 = (probe1*(0.00322)); //converts probe reading into a voltage value based on the 3.3V input and 1023 bits 
  //Serial.print(analogRead(A1)/1000.0);
  Serial.println(probe1);
  //Serial.print("\n"); 
  
  //Serial.println(analogRead(A1)*(3.3/1023)); 
  //delay(2);
  //  float voltage1 = (probe1*(0.00322)); //converts probe reading into a voltage value based on the 3.3V input and 1023 bits 
  //Serial.print(",");
  //float voltage2 = (probe2 * (0.00322)) - 1.5;
  //Serial.print(voltage1);
  //Serial.println((analogRead(A1)*0.00322)); 
  //Serial.println("Voltage:,time:");   //creating a legend that lists both voltage and time 
 //delayMicroseconds(100);
  //delay(10);//delay by 10 ms / 1000 =
}
