OPC opc;
OPCRecorder opcrecorder;
PImage dot1, dot2;

void setup()
{
  frameRate(50);

  dot1 = loadImage("greenDot.png");
  dot2 = loadImage("purpleDot.png");

  // Connect to the local instance of fcserver. You can change this line to connect to another computer's fcserver
  opc = new OPC(this, "ledgrid.home", 7890);
  //opc = new OPC(this, "ledwall.home", 7890);
  //opc = new OPC(this, "127.0.0.1", 7890);
  
   // Record current OPC output to local file
   opcrecorder = new OPCRecorder(this, "V:\\orbits.opc");
   opcrecorder.setFrameRate(50);
   opc.enableOPCRecording(true);

  // 8x60 Neopixel Strips Address
 // size(420, 320);
//  opc.ledStrip(0, 60, width/2, height * 1 / 16, width / 70.0, 0, true); 
//  opc.ledStrip(60, 60, width/2, height * 3 / 16, width / 70.0, 0, true);
//  opc.ledStrip(120, 60, width/2, height * 5 / 16, width / 70.0, 0, true); 
//  opc.ledStrip(180, 60, width/2, height * 7 / 16, width / 70.0, 0, true); 
//  opc.ledStrip(240, 60, width/2, height * 9 / 16, width / 70.0, 0, true); 
//  opc.ledStrip(300, 60, width/2, height * 11 / 16, width / 70.0, 0, true); 
//  opc.ledStrip(360, 60, width/2, height * 13 / 16, width / 70.0, 0, true); 
//  opc.ledStrip(420, 60, width/2, height * 15 / 16, width / 70.0, 0, true);

  // 93-LED LED Fadecandy Ring Address
  //size(420, 420);
  //opc.ledRing(0, 1, width/2, height/2, width * 1, 0);
  //opc.ledRing(8, 8, width/2, height/2, width * 1 / 12, 0);
  //opc.ledRing(20, 12, width/2, height/2, width * 2 / 12, 0);
  //opc.ledRing(36, 16, width/2, height/2, width * 3 / 12, 0);
  //opc.ledRing(60, 24, width/2, height/2, width * 4 / 12, 0);
  //opc.ledRing(92, 32, width/2, height/2, width * 5 / 12, 0);
  
  size(360, 360);
  // Map an 8x8 grid of LEDs (Non-reversed)
  //opc.ledGrid8x8(0, width/2, height/2, height / 8.0, 0 ,false);
  // Map an 8x8 grid of LEDs (Reversed, Zigzag)
  opc.ledGrid8x8(0, width/2, height/2, height / 8.0, 0 ,true);

}

float px, py;

void draw()
{
  background(0);
  blendMode(ADD);
  
  // Smooth out the mouse location
  px += (50 - px) * 0.1;
  py += (200 - py) * 0.1;

  float a = millis() * 0.001;
  float r = py * 0.5;
  float dotSize = r * 4;  

  float dx = width/2 + cos(a) * r;
  float dy = height/2 + sin(a) * r;
  
  // Draw it centered at the mouse location
  image(dot1, dx - dotSize/2, dy - dotSize/2, dotSize, dotSize);

  // Another dot, mirrored around the center
  image(dot2, width - dx - dotSize/2, height - dy - dotSize/2, dotSize, dotSize);
}
