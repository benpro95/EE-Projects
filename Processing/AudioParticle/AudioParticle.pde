// Some real-time FFT! This visualizes music in the frequency domain using a
// polar-coordinate particle system. Particle size and radial distance are modulated
// using a filtered FFT. Color is sampled from an image.

import ddf.minim.analysis.*;
import ddf.minim.*;

OPC opc;
PImage dot;
PImage colors;
Minim minim;
AudioInput in;
FFT fft;
float[] fftFilter;
int buffer_size = 1024;  // also sets FFT size (frequency resolution)
float sample_rate = 44100;

float spin = 0.003;
float radiansPerBucket = radians(2);
float decay = 0.95;
float opacity = 40;
float minSize = 0.15;
float sizeScale = 0.5;

void setup()
{
  size(320, 240, P3D);

  minim = new Minim(this); 

  // Small buffer size!
 // in = minim.getLineIn();
   in = minim.getLineIn(Minim.MONO,buffer_size,sample_rate);
 
  fft = new FFT(in.bufferSize(), in.sampleRate());
  fftFilter = new float[fft.specSize()];

  dot = loadImage("dot.png");
  colors = loadImage("colors.png");

  // Connect to the network instance of fcserver
   //opc = new OPC(this, "ledwall.local", 7890);
  // Connect to the local instance of fcserver
   opc = new OPC(this, "automate.home", 7890);

  // Map 420-LED strips 
   opc.ledStrip(0, 60, width/2, height * 1 / 16, width / 70.0, 0, false); 
   opc.ledStrip(60, 60, width/2, height * 3 / 16, width / 70.0, 0, false);
   opc.ledStrip(120, 60, width/2, height * 5 / 16, width / 70.0, 0, false); 
   opc.ledStrip(180, 60, width/2, height * 7 / 16, width / 70.0, 0, false); 
   opc.ledStrip(240, 60, width/2, height * 9 / 16, width / 70.0, 0, false); 
   opc.ledStrip(300, 60, width/2, height * 11 / 16, width / 70.0, 0, false); 
   opc.ledStrip(360, 60, width/2, height * 13 / 16, width / 70.0, 0, false); 
   opc.ledStrip(420, 60, width/2, height * 15 / 16, width / 70.0, 0, false); 
}

void draw()
{
  background(0);

  fft.forward(in.mix);
  for (int i = 0; i < fftFilter.length; i++) {
    fftFilter[i] = max(fftFilter[i] * decay, log(1 + fft.getBand(i)));
  }
 
  for (int i = 0; i < fftFilter.length; i += 3) {   
    color rgb = colors.get(int(map(i, 0, fftFilter.length-1, 0, colors.width-1)), colors.height/2);
    tint(rgb, fftFilter[i] * opacity);
    blendMode(ADD);
 
    float size = height * (minSize + sizeScale * fftFilter[i]);
    PVector center = new PVector(width * (fftFilter[i] * 0.2), 0);
    center.rotate(millis() * spin + i * radiansPerBucket);
    center.add(new PVector(width * 0.5, height * 0.5));
 
    image(dot, center.x - size/2, center.y - size/2, size, size);
  }
}
