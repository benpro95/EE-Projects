OPC opc;
OPCRecorder opcrecorder;
PImage im;
float jpgspeed = 0.045;

void setup()
{
  size(600,500);
    
  // Load a sample image
  im = loadImage("water.jpg");

  opc = new OPC(this, "ledgrid.home", 7890);
  //opc = new OPC(this, "127.0.0.1", 7890);
  
  // Record current OPC output to local file
   opcrecorder = new OPCRecorder(this, "V:\\water.opc");
   opcrecorder.setFrameRate(35);
   opc.enableOPCRecording(true);
   
   // Map 8x60 Neopixel Strips
   //opc.ledStrip(0, 60, width/2, height * 1 / 16, width / 70.0, 0, false); 
   //opc.ledStrip(60, 60, width/2, height * 3 / 16, width / 70.0, 0, false);
   //opc.ledStrip(120, 60, width/2, height * 5 / 16, width / 70.0, 0, false); 
   //opc.ledStrip(180, 60, width/2, height * 7 / 16, width / 70.0, 0, false); 
   //opc.ledStrip(240, 60, width/2, height * 9 / 16, width / 70.0, 0, false); 
   //opc.ledStrip(300, 60, width/2, height * 11 / 16, width / 70.0, 0, false); 
   //opc.ledStrip(360, 60, width/2, height * 13 / 16, width / 70.0, 0, false); 
   //opc.ledStrip(420, 60, width/2, height * 15 / 16, width / 70.0, 0, false);
   
 // 93-LED LED Fadecandy Ring Address
 // opc.ledRing(0, 1, width/2, height/2, width * 1, 0);
 // opc.ledRing(8, 8, width/2, height/2, width * 1 / 14, 0);
 // opc.ledRing(20, 12, width/2, height/2, width * 2 / 14, 0);
 // opc.ledRing(36, 16, width/2, height/2, width * 3 / 14, 0);
 // opc.ledRing(60, 24, width/2, height/2, width * 4 / 14, 0);
 // opc.ledRing(92, 32, width/2, height/2, width * 5 / 14, 0);

 // Map an 8x8 grid of LEDs (Non-reversed)
 // opc.ledGrid8x8(0, width/3, height/2, height / 16.0, 0, false);
 // Map an 8x8 grid of LEDs (Reversed, Zigzag)
    opc.ledGrid8x8(0, width/3, height/2, height / 16.0, 0 ,true);
  
}

void draw()
{
  // Scale the image so that it matches the width of the window
  int imHeight = im.height * width / im.width;

  // Scroll down slowly, and wrap around
  float speed = jpgspeed;
  float y = (millis() * -speed) % imHeight;
  
  // Use two copies of the image, so it seems to repeat infinitely  
  image(im, 0, y, width, imHeight);
  image(im, 0, y + imHeight, width, imHeight);
}

void keyReleased()
{
  // +/- used to adjust scrolling speed on the fly
  if (key == '+' || key == '=') {
    jpgspeed = jpgspeed + 0.01;
  } else if (key == '-' || key == '_') {
    jpgspeed = jpgspeed - 0.01;
  }
}
