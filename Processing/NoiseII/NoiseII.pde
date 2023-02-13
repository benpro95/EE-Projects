OPC opc;
OPCRecorder opcrecorder;
float dx, dy;

void setup()
{
  //size(420, 300, P3D);

  // Connect to the local instance of fcserver. You can change this line to connect to another computer's fcserver
  //opc = new OPC(this, "127.0.0.1", 7890);
  opc = new OPC(this, "ledgrid.home", 7890);

  // Record current OPC output to local file
   opcrecorder = new OPCRecorder(this, "V:\\noise2.opc");
   opcrecorder.setFrameRate(35);
   opc.enableOPCRecording(true);

   // Map 420-LED strips 
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
  // opc.ledRing(8, 8, width/2, height/2, width * 1 / 16, 0);
  // opc.ledRing(20, 12, width/2, height/2, width * 2 / 16, 0);
  // opc.ledRing(36, 16, width/2, height/2, width * 3 / 16, 0);
  // opc.ledRing(60, 24, width/2, height/2, width * 4 / 16, 0);
  // opc.ledRing(92, 32, width/2, height/2, width * 5 / 16, 0);

  // Map an 8x8 grid of LEDs
  size(320, 320, P3D);
  // Map an 8x8 grid of LEDs (Non-reversed)
  //opc.ledGrid8x8(0, width/2, height/2, height / 8.0, 0 ,false);
  // Map an 8x8 grid of LEDs (Reversed, Zigzag)
  opc.ledGrid8x8(0, width/2, height/2, height / 8.0, 0 ,true);

  // Make the status LED quiet
  opc.setStatusLed(false);
  
  colorMode(HSB, 90);
}

float noiseScale=0.1;

float fractalNoise(float x, float y, float z) {
  float r = 0;
  float amp = 1.0;
  for (int octave = 0; octave < 4; octave++) {
    r += noise(x, y, z) * amp;
    amp /= 2;
    x *= 2;
    y *= 2;
    z *= 2;
  }
  return r;
}

void draw() {
  long now = millis();
  float speed = 0.05;
  float angle = sin(now * 0.001);
  float z = now * 0.00008;
  float hue = now * 0.01;
  float scale = 0.005;

  dx += cos(angle) * speed;
  dy += sin(angle) * speed;

  loadPixels();
  for (int x=0; x < width; x++) {
    for (int y=0; y < height; y++) {
     
      float n = fractalNoise(dx + x*scale, dy + y*scale, z) - 0.75;
      float m = fractalNoise(dx + x*scale, dy + y*scale, z + 10.0) - 0.75;

      color c = color(
         (hue + 80.0 * m) % 100.0,
         100 - 100 * constrain(pow(3.0 * n, 3.5), 0, 0.9),
         100 * constrain(pow(3.0 * n, 1.5), 0, 0.9)
         );
      
      pixels[x + width*y] = c;
    }
  }
  updatePixels();
}
