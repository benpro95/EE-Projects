// Display Text on the 93-LED LED Fadecandy Ring by Ben Provenzano III
// v1

OPC opc;
PFont f;
PShader blur;
// Default scroll speed
float scroll = 0.090;

void setup()
{
  size(480, 360, P2D);

  // Horizontal blur
  blur = loadShader("blur.glsl");
  blur.set("blurSize", 65);
  blur.set("sigma", 8.0f);
  blur.set("horizontalPass", 1);

  // Connect to the local instance of fcserver
  opc = new OPC(this, "ledwall.local", 7890);
 // opc = new OPC(this, "127.0.0.1", 7890);

  opc.ledRing(0, 1, width/2, height/2, width * 1, PI);
  opc.ledRing(8, 8, width/2, height/2, width * 2 / 32, PI);
  opc.ledRing(20, 12, width/2, height/2, width * 4 / 32, PI);
  opc.ledRing(36, 16, width/2, height/2, width * 6 / 32, PI);
  opc.ledRing(60, 24, width/2, height/2, width * 8 / 32, PI);
  opc.ledRing(92, 32, width/2, height/2, width * 10 / 32, PI);
  
  // Create the font
  f = createFont("Futura", 280);
  textFont(f);
}

void scrollMessage(String s, float speed)
{
  int x = int( width + (millis() * -speed) % (textWidth(s) + width) );
  text(s, x, 280);  
}

void draw()
{
  background(0);
  
  // Text color
  fill(60, 255, 100);
  
  // Mirror text for projector
  translate(width/2,height * 1);
  scale(1, -1);
  
  scrollMessage("Have a goodnight!!", scroll);
  
  filter(blur);
}

void keyReleased()
{
  // +/- used to adjust scrolling speed on the fly
  if (key == '+' || key == '=') {
    scroll = scroll + 0.01;
  } else if (key == '-' || key == '_') {
    scroll = scroll - 0.01;
  }
}
