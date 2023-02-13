OPC opc;
OPCRecorder opcrecorder;
PImage im;


void setup()
{
  size(420, 420);
  // Load a sample image
  im = loadImage("pic.png");

  // Connect to the local instance of fcserver
  //opc = new OPC(this, "127.0.0.1", 7890);
  opc = new OPC(this, "ledgrid.home", 7890);
  
   // Record current OPC output to local file
   opcrecorder = new OPCRecorder(this, "V:\\spectrum.opc");
   opcrecorder.setFrameRate(40);
   opc.enableOPCRecording(true);

   //opc.ledStrip(0, 60, width * 1 / 16, height/2, width / 70.0, PI/2, false); 
   //opc.ledStrip(60, 60, width * 3 / 16, height/2, width / 70.0, PI/2, false);
   //opc.ledStrip(120, 60, width * 5 / 16, height/2, width / 70.0, PI/2, false); 
   //opc.ledStrip(180, 60, width * 7 / 16, height/2, width / 70.0, PI/2, false); 
   //opc.ledStrip(240, 60, width * 9 / 16, height/2, width / 70.0, PI/2, false); 
   //opc.ledStrip(300, 60, width * 11 / 16, height/2, width / 70.0, PI/2, false); 
   //opc.ledStrip(360, 60, width * 13 / 16, height/2, width / 70.0, PI/2, false); 
   //opc.ledStrip(420, 60, width * 15 / 16, height/2, width / 70.0, PI/2, false); 
   
  // 93-LED LED Fadecandy Ring Address
  //opc.ledRing(0, 1, width/2, height/2, width * 1, 0);
  //opc.ledRing(8, 8, width/2, height/2, width * 1 / 22, 0);
  //opc.ledRing(20, 12, width/2, height/2, width * 2 / 22, 0);
  //opc.ledRing(36, 16, width/2, height/2, width * 3 / 22, 0);
  //opc.ledRing(60, 24, width/2, height/2, width * 4 / 22, 0);
  //opc.ledRing(92, 32, width/2, height/2, width * 5 / 22, 0);
  
  // Map an 8x8 grid of LEDs (Non-reversed)
  // opc.ledGrid8x8(0, width/2, height/2, height / 8.0, 0, false);
  // Map an 8x8 grid of LEDs (Reversed, Zigzag)
     opc.ledGrid8x8(0, width/2, height/2, height / 8.0, 0 ,true);
 
}


void draw()
{
  // Scale the image so that it matches the width of the window
  int imHeight = im.height * width / im.width;

  // Scroll down slowly, and wrap around
  float speed = 0.025;
  float y = (millis() * -speed) % imHeight;
  
  // Use two copies of the image, so it seems to repeat infinitely  
  image(im, 0, y, width, imHeight);
  image(im, 0, y + imHeight, width, imHeight);
}
