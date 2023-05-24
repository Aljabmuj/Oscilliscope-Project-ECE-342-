import processing.serial.*;

// Declare variables and constants
Serial myPort;
int data;
int margin = 100;
String inString;
float prevX0 = margin;
float prevX1 = margin;
float prevY0 = 250;
float prevY1 = 250;
boolean firstContact = false;
float[] x0 = new float[500];
float[] x1 = new float[500];
float[] y0 = new float[500];
float[] y1 = new float[500];
String amplitude0 = "?";
String amplitude1 = "?";
int inByte; 


// Start setup 
void setup(){
 // Establish screen size
 size(700, 500);
 background(0);
 // Establis port
 myPort = new Serial(this, "/dev/cu.usbmodem137715601", 4608000);
 serialEvent(myPort);
}

 


void draw(){
  // Reset background
  background(0);
  setGraph();
  
  // Display points on the graph
  for(int i=0; i<500; i++){
    stroke(255, 255, 204);
    line(prevX1, prevY1, x1[i], y1[i]);
    stroke(246, 25, 25);
    line(prevX0, prevY0, x0[i], y0[i]);
    prevX0 = x0[i];
    prevX1 = x1[i];
    prevY0 = y0[i];
    prevY1 = y1[i];
  }
  
  // Display the amplitude of channels
  textSize(12);
  fill(126);
  text(amplitude0, 25, margin + 65);
  text(amplitude1, 25, margin + 215);
  
  // Reset intial starting point
  prevX0 = margin;
  prevX1 = margin;
  prevY0 = height/2;
  prevY1 = height/2;
  
  // Tell Arduino it needs more data
  myPort.write('A');
}

void setGraph(){
  // Add graph outline and text
  textSize(12);
  stroke(126);
  line(width/2, margin, width/2, height-margin);
  line(margin, height/2, width - margin, height/2);
  line(margin, margin, width - margin, margin);
  line(margin, height - margin, width - margin, height - margin);
  line(margin, margin, margin, height - margin);
  line(width - margin, margin, width - margin, height - margin);
  line(20, margin, 80, margin);
  line(20, margin + 130, 80, margin + 130);
  line(20, margin, 20, margin + 130);
  line(80, margin, 80, margin + 130);
  fill(246, 25, 25);
  text("Channel 1", 25, margin + 25);
  fill(126);
  text("Vpp", 25, margin + 50);
  fill(255, 255, 204);
  text("Channel 2", 25, margin + 175);
  fill(126);
  text("Vpp", 25, margin + 200);
  line(20, margin + 150, 80, margin + 150);
  line(20, margin + 280, 80, margin + 280);
  line(20, margin + 150, 20, margin + 280);
  line(80, margin + 150, 80, margin + 280);
  textSize(48);
  fill(102, 0, 204);
  text("Osciloscope Hype Squad", 105, 55);
  line(margin, 0, margin, 80);
  line(width - margin, 0, width - margin, 80);
  line (margin, 80, width - margin, 80);
}


void serialEvent(Serial myPort){
  // Import new data
 inByte = myPort.read();
 if (firstContact == false){
   if(inByte == 'A'){
     // If a and no first contact send A
     myPort.clear();
     firstContact = true;
     myPort.write('A');
   }
 }
 else{
   // If not A and first contact is established update the plotting values
   myPort.bufferUntil('\n');
   String inString = myPort.readStringUntil('\n');
   if (inString != null){
     String[] stringSplit = split(inString, ',');
     for (int i=0; i<500; i++){
       int bit = int(stringSplit[i]);
       if (inByte == 'R'){
         y0[i] = map(bit, 0, 1024, margin, height-margin);
         x0[i] = i + margin;
       }
       else {
         y0[i] = height/2;
         x0[i] = margin;
       }
       if (inByte == 'L'){
         y1[i] = map(bit, 0, 1024, margin, height-margin);
         x1[i] = i + margin;
       }
       else {
         y1[i] = height/2;
         x1[i] = margin;
       }
     }
     // Read new amplitudes
     if (inByte == 'R'){
       amplitude0 = stringSplit[501];
     }
     if (inByte == 'L'){
       amplitude1 = stringSplit[501];
     }
   }
   }
  
}
