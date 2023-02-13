// Ben Provenzano III's Audio RGB Visualizer SpectroGraph!! for LED Projector
 
import ddf.minim.analysis.*;
import ddf.minim.*;
OPC opc; 
Minim minim;
AudioInput in;
FFT fft;
float[] peaks;
int hVal;
int peak_hold_time = 1;  // how long before peak decays
int[] peak_age;  // tracks how long peak has been stable, before decaying
// how wide each 'peak' band is, in fft bins
int binsperband = 30;
int peaksize; // how many individual peak bands we have (dep. binsperband)
float gain = 50; // in dB
float dB_scale = 12;  // pixels per dB
int buffer_size = 1024;  // also sets FFT size (frequency resolution)
float sample_rate = 44100;
int spectrum_height = 320; // determines range of dB shown
int legend_height = 20;
int spectrum_width = 280; // determines how much of spectrum we see
int legend_width = -15;
PImage im;

void setup()
{
  frameRate(60);
  size(280, 280);
     minim = new Minim(this);
  in = minim.getLineIn(Minim.MONO,buffer_size,sample_rate);
  // create an FFT object that has a time-domain buffer 
  // the same size as line-in's sample buffer
  fft = new FFT(in.bufferSize(), in.sampleRate());
  // Tapered window important for log-domain display
  fft.window(FFT.HAMMING);

  // initialize peak-hold structures
  peaksize = 1+Math.round(fft.specSize()/binsperband);
  peaks = new float[peaksize];
  peak_age = new int[peaksize];
  
  // Connect to the local instance of fcserver
  //opc = new OPC(this, "127.0.0.1", 7890);
  opc = new OPC(this, "ledwall.home", 7890);

  // 93-LED LED Fadecandy Ring Address
  opc.ledRing(0, 1, width/2, height/2, width * 1, 0);
  opc.ledRing(8, 8, width/2, height/2, width * 1 / 16, 0);
  opc.ledRing(20, 12, width/2, height/2, width * 2 / 16, 0);
  opc.ledRing(36, 16, width/2, height/2, width * 3 / 16, 0);
  opc.ledRing(60, 24, width/2, height/2, width * 4 / 16, 0);
  opc.ledRing(92, 32, width/2, height/2, width * 5 / 16, 0);
  
  // Map 420-LED strips 
  // opc.ledStrip(420, 60, width * 1 / 16, height/2, width / 70.0, PI/2, true); 
  // opc.ledStrip(360, 60, width * 3 / 16, height/2, width / 70.0, PI/2, true);
  // opc.ledStrip(300, 60, width * 5 / 16, height/2, width / 70.0, PI/2, true); 
  // opc.ledStrip(240, 60, width * 7 / 16, height/2, width / 70.0, PI/2, true); 
  // opc.ledStrip(180, 60, width * 9 / 16, height/2, width / 70.0, PI/2, true); 
  // opc.ledStrip(120, 60, width * 11 / 16, height/2, width / 70.0, PI/2, true); 
  // opc.ledStrip(60, 60, width * 13 / 16, height/2, width / 70.0, PI/2, true); 
  // opc.ledStrip(0, 60, width * 15 / 16, height/2, width / 70.0, PI/2, true); 

   hVal = 0;
   im = loadImage("pic.jpg"); 
}


void draw()

{
  
  background(0); 
  
  
  // Scale the image so that it matches the width of the window
  int imHeight = im.height;

  // Scroll down slowly, and wrap around
  float speed = 0.008;
  float x = (millis() * -speed) % imHeight;
  
  // Use two copies of the image, so it seems to repeat infinitely  

  image(im, x, 0, imHeight, width );
  image(im, x + imHeight, 0, imHeight, width);

 
  // perform a forward FFT on the samples in input buffer
  fft.forward(in.mix);
  
  // draw peak bars
  noStroke();
  colorMode(HSB);
  fill(hVal, 255, 255);
  for(int i = 0; i < peaksize; ++i) { 
    int thisy = spectrum_height - Math.round(peaks[i]);
    rect(legend_width+binsperband*i, thisy, binsperband, spectrum_height-thisy);
    // update decays
    if (peak_age[i] < peak_hold_time) {
      ++peak_age[i];
    } else {
      peaks[i] -= 8; // Bars Speed (lower = slower)
      if (peaks[i] < 0) { peaks[i] = 0; }
    } 

 
  }

  // now draw current spectrum 
  noStroke();
  noFill();
  for(int i = 0; i < spectrum_width; i++)  {
    // draw the line for frequency band i using dB scale
    float val = dB_scale*(20*((float)Math.log10(fft.getBand(i))) + gain);
    if (fft.getBand(i) == 0) {   val = -200;   }  // avoid log(0)
    int y = spectrum_height - Math.round(val);
    if (y > spectrum_height) { y = spectrum_height; }
    line(legend_width+i, spectrum_height, legend_width+i, y);
    // update the peak record
    // which peak bin are we in?
    int peaksi = i/binsperband;
    if (val > peaks[peaksi]) {
      peaks[peaksi] = val;
      // reset peak age counter
      peak_age[peaksi] = 0;
    }
  }
{
}
 hVal +=  1;
 if( hVal > 255)
 {
     hVal = 0;
 }

}

void keyReleased()
{
  // +/- used to adjust gain on the fly
  if (key == '+' || key == '=') {
    gain = gain + 5.0;
  } else if (key == '-' || key == '_') {
    gain = gain - 5.0;
  }
}

 
void stop()
{
  // always close Minim audio classes when you finish with them
  in.close();
  minim.stop();
 
  super.stop();
}
