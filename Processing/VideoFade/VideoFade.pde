// Display H.264 Video on the Fadecandy by Ben Provenzano III
// v2.0

OPC opc;
OPCRecorder opcrecorder;
import processing.video.*;
Movie movie;

// Movie name and output name
String file = "tunnel";

void setup()
{
   background(0);
   frameRate(30);

   // Load and play the video in a loop
   movie = new Movie(this, file + ".mp4");
   movie.loop();

   // Connect to the local instance of fcserver
   opc = new OPC(this, "ledwall.home", 7890);
   //opc = new OPC(this, "127.0.0.1", 7890);

   // Map 420-LED strips
  // size(620, 420); 
  // opc.ledStrip(0, 60, width/2, height * 5 / 32, width / 70.0, 0, false); 
  // opc.ledStrip(60, 60, width/2, height * 7 / 32, width / 70.0, 0, false);
  // opc.ledStrip(120, 60, width/2, height * 9 / 32, width / 70.0, 0, false); 
  // opc.ledStrip(180, 60, width/2, height * 11 / 32, width / 70.0, 0, false); 
  // opc.ledStrip(240, 60, width/2, height * 13 / 32, width / 70.0, 0, false); 
  // opc.ledStrip(300, 60, width/2, height * 15 / 32, width / 70.0, 0, false); 
  // opc.ledStrip(360, 60, width/2, height * 17 / 32, width / 70.0, 0, false); 
  // opc.ledStrip(420, 60, width/2, height * 19 / 32, width / 70.0, 0, false); 
  
   // 93-LED LED Fadecandy Ring Address
   size(620, 420);
   opc.ledRing(0, 1, width/2, height/2, width * 1, 0);
   opc.ledRing(8, 8, width/2, height/2, width * 1 / 15, 0);
   opc.ledRing(20, 12, width/2, height/2, width * 2 / 15, 0);
   opc.ledRing(36, 16, width/2, height/2, width * 3 / 15, 0);
   opc.ledRing(60, 24, width/2, height/2, width * 4 / 15, 0);
   opc.ledRing(92, 32, width/2, height/2, width * 5 / 15, 0);
  
  //size(580, 320);
  // Map an 8x8 grid of LEDs (Non-reversed)
  //opc.ledGrid8x8(0, width/2, height/2, height / 10.0, 0 ,false);
  
  // Map an 8x8 grid of LEDs (Reversed, Zigzag)
  //opc.ledGrid8x8(0, width/2, height/2, height / 10.0, 0 ,true);
  
  delay(500);
      
  // Record current OPC output to local file
   opcrecorder = new OPCRecorder(this, "B:\\Shared\\" + file + ".opc");
   opcrecorder.setFrameRate(30);
   opc.enableOPCRecording(true);     
      
}

void movieEvent(Movie m) {
  m.read();
}

void draw() 
{
  if (movie.available() == true) {
    movie.read(); 
  }
  
  // Mirror the video
  //translate(width/32,height/1);
  //scale(1, -1);
  
  image(movie, 0, 0, width, height);
}
