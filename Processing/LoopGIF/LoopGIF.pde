/*
by Ben Provenzano III
*/

import gifAnimation.*;
OPCRecorder opcrecorder;
OPC opc;
Gif loopingGif;

// Movie name and output name
String file = "beachball";

public void setup() {
  size(800, 600);
  frameRate(30);
  
  // Connect to the local instance of fcserver
  opc = new OPC(this, "ledgrid.home", 7890);
  //opc = new OPC(this, "127.0.0.1", 7890);

  // Map an 8x8 grid of LEDs (Non-reversed)
  //opc.ledGrid8x8(0, width/2, height/2, height / 10.0, 0 ,false);
  // Map an 8x8 grid of LEDs (Reversed, Zigzag)
  //opc.ledGrid8x8(0, width/2, height/2, height / 38, 0 ,true);

  // 93-LED LED Fadecandy Ring
  //opc.ledRing(0, 1, width/2, height/2, width * 1, 0);
  //opc.ledRing(8, 8, width/2, height/2, width * 1 / 52, 0);
  //opc.ledRing(20, 12, width/2, height/2, width * 2 / 52, 0);
  //opc.ledRing(36, 16, width/2, height/2, width * 3 / 52, 0);
  //opc.ledRing(60, 24, width/2, height/2, width * 4 / 52, 0);
  //opc.ledRing(92, 32, width/2, height/2, width * 5 / 52, 0);
  
  // 420 LED Fadecandy Strips
  //opc.ledStrip(0, 60, width/2, height * 1 / 16, width / 70.0, 0, false); 
  //opc.ledStrip(60, 60, width/2, height * 3 / 16, width / 70.0, 0, false);
  //opc.ledStrip(120, 60, width/2, height * 5 / 16, width / 70.0, 0, false); 
  //opc.ledStrip(180, 60, width/2, height * 7 / 16, width / 70.0, 0, false); 
  //opc.ledStrip(240, 60, width/2, height * 9 / 16, width / 70.0, 0, false); 
  //opc.ledStrip(300, 60, width/2, height * 11 / 16, width / 70.0, 0, false); 
  //opc.ledStrip(360, 60, width/2, height * 13 / 16, width / 70.0, 0, false); 
  //opc.ledStrip(420, 60, width/2, height * 15 / 16, width / 70.0, 0, false); 
  
  // create the GifAnimation object for playback
  loopingGif = new Gif(this, "image.gif");
  loopingGif.loop();
  
  delay(500);
      
  // Record current OPC output to local file
   opcrecorder = new OPCRecorder(this, "Z:\\ProOS\\rpi\\effects\\8x8revgrid\\" + file + ".opc");
   opcrecorder.setFrameRate(30);
   opc.enableOPCRecording(true);     
}

void draw() {
  background(0);
  image(loopingGif, 0, 0);
}
