 import processing.serial.*;

  Serial teensy;  // The serial port of the teensy kit 
  float x = 0; 
  float y = 250; //half of the height! 
  float space = 0;
  float zoom; 
  
  PFont f0; 
  PImage logo;
  
   void grid(){ //establish Oscilloscope grid
    stroke(255,0,0); 
    line(0,height/2, width, height/2); 
  } 

  void setup () {
    //size(900,500); //Set the window of graph 
  
    teensy = new Serial(this, Serial.list()[0], 115200); //Take input from the Teensy board, including pin A0 and A1 

    teensy.bufferUntil('\n'); //buffer function to generate 
    
    fullScreen(); //creates a display for the whole screen 

    background(0); // set the background color 
    
    zoom = 1.0f; 
  }
  
  void GraphRules(){ 
    f0 = createFont("Arial",30,true);
  textFont(f0);
  fill(255);
  textAlign(CENTER); //RIGHT LEFT
  text("Oscilliscope Hype Squad",width/2,height-450);
  
  image(logo,width-100,0,100,200); //add image
  }
  void draw () {
    GraphRules(); 
    grid();
    rect(x, height*0.2, 1, 
    stroke(153,0,0); //color of the line 
    line(x, y, x, space); //create the line 
    y = space; 
    // at the edge of the screen, go back to the beginning:
    if (x >= width) {
      x = 0;
      background(0); 
    } else {
      // increment the horizontal position:
      x++;
    }
  }

  void serialEvent (Serial Teensy) {
    // get the ASCII string:
    String sample = Teensy.readStringUntil('\n');

    if (sample != null) {
      // trim off any whitespace:
      sample = trim(sample);
      // convert to an int and map to the screen height:
      space = float(sample);
      //println(space);
      space = map(space, 0, 1023, 0, height);
    }
  }
