OPC opc;
OPCRecorder opcrecorder;
PShader blur;
PGraphics src;
PGraphics pass1, pass2;

void setup()
{
  size(600, 320, P2D);

  blur = loadShader("blur.glsl");
  blur.set("blurSize", height / 8);
  blur.set("sigma", 1.0f);  
  
  src = createGraphics(width, height, P3D); 
  
  pass1 = createGraphics(width, height, P2D);
  pass1.noSmooth();  
  
  pass2 = createGraphics(width, height, P2D);
  pass2.noSmooth();

  // Connect to the local instance of fcserver. You can change this line to connect to another computer's fcserver
  opc = new OPC(this, "ledwall.home", 7890);
  //opc = new OPC(this, "127.0.0.1", 7890);
 
  // Record current OPC output to local file
  opcrecorder = new OPCRecorder(this, "V:\\blur.opc");
  opcrecorder.setFrameRate(50);
  opc.enableOPCRecording(true);
  
  // 93-LED LED Fadecandy Ring Address
  // opc.ledRing(0, 1, width/2, height/2, width * 1, 0);
  // opc.ledRing(8, 8, width/2, height/2, width * 1 / 22, 0);
  // opc.ledRing(20, 12, width/2, height/2, width * 2 / 22, 0);
  // opc.ledRing(36, 16, width/2, height/2, width * 3 / 22, 0);
  // opc.ledRing(60, 24, width/2, height/2, width * 4 / 22, 0);
  // opc.ledRing(92, 32, width/2, height/2, width * 5 / 22, 0);

  // 8x60 Neopixel Strips Address
  // float spacing = height / 14.0;
  // opc.ledStrip(0, 60, width/2, height * 1 / 16, width / 70.0, 0, true); 
  // opc.ledStrip(60, 60, width/2, height * 3 / 16, width / 70.0, 0, true);
  // opc.ledStrip(120, 60, width/2, height * 5 / 16, width / 70.0, 0, true); 
  // opc.ledStrip(180, 60, width/2, height * 7 / 16, width / 70.0, 0, true); 
  // opc.ledStrip(240, 60, width/2, height * 9 / 16, width / 70.0, 0, true); 
  // opc.ledStrip(300, 60, width/2, height * 11 / 16, width / 70.0, 0, true); 
  // opc.ledStrip(360, 60, width/2, height * 13 / 16, width / 70.0, 0, true); 
  // opc.ledStrip(420, 60, width/2, height * 15 / 16, width / 70.0, 0, true);
  
  // Map an 8x8 grid of LEDs (Non-reversed)
  // opc.ledGrid8x8(0, width/2, height/2, height / 8.0, 0, false);
  // Map an 8x8 grid of LEDs (Reversed, Zigzag)
  opc.ledGrid8x8(0, width/2, height/2, height / 8.0, 0 ,true);

  // Make the status LED quiet
  opc.setStatusLed(false);
}

void draw()
{
  float t = millis() * 0.001;
  randomSeed(0);
  
  src.beginDraw();
  src.noStroke();
  src.background(0, 0, 255);
  src.fill(255, 150);
  src.blendMode(NORMAL);

  src.directionalLight(255, 255, 255, -1, 0, 0.4);
  src.directionalLight(50, 50, 50, -1, 0, 0.2);
  src.directionalLight(50, 50, 50, -1, 0, 0);
  src.directionalLight(255, 0, 0, 1, 0, 0);

  // Lots of rotating cubes
  for (int i = 0; i < 80; i++) {
    src.pushMatrix();

    // This part is the chaos demon.
    src.translate(map(noise(random(1000), t * 0.07), 0, 1, -width, width*2),
      map(noise(random(1000), t * 0.07), 0, 1, -height, height*2), 0);

    // Progression of time
    src.rotateY(t * 0.4 + randomGaussian());
    src.rotateX(t * 0.122222 + randomGaussian());

    // But of course.
    src.box(height * abs(0.2 + 0.2 * randomGaussian()));
    src.popMatrix();
  }

  // Brighten if we care
  /*
  src.noLights();
  src.blendMode(ADD);
  src.fill(40, 0, 0);
  src.rect(0, 0, width, height);
  */

  // Separable blur filter
  src.endDraw();

  blur.set("horizontalPass", 0);
  pass1.beginDraw();            
  pass1.shader(blur);  
  pass1.image(src, 0, 0);
  pass1.endDraw();
  
  blur.set("horizontalPass", 1);
  pass2.beginDraw();            
  pass2.shader(blur);  
  pass2.image(pass1, 0, 0);
  pass2.endDraw();    
        
  image(pass2, 0, 0);
}
