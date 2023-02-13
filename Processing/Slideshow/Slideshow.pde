// Picture Slideshow Visualizer made by Ben Provenzano III - July 28th 2016

OPC opc;
int numFrames = 10;
int frame = 0; 
int start = 0; 
int end = 9;

PImage[] images = new PImage[numFrames];

void setup(){ 

// fullScreen();
frameRate(1);
size(800,600);
 
 for(int i=0; i<numFrames; i++) {
   String imageName = "pic" + ((i < 9) ? "" : "") + i + ".jpg";
   images[i] = loadImage(imageName);
 }

  opc = new OPC(this, "127.0.0.1", 7890);

   opc.ledStrip(0, 60, width/2, height * 1 / 16, width / 70.0, 0, true); 
   opc.ledStrip(60, 60, width/2, height * 3 / 16, width / 70.0, 0, true);
   opc.ledStrip(120, 60, width/2, height * 5 / 16, width / 70.0, 0, true); 
   opc.ledStrip(180, 60, width/2, height * 7 / 16, width / 70.0, 0, true); 
   opc.ledStrip(240, 60, width/2, height * 9 / 16, width / 70.0, 0, true); 
   opc.ledStrip(300, 60, width/2, height * 11 / 16, width / 70.0, 0, true); 
   opc.ledStrip(360, 60, width/2, height * 13 / 16, width / 70.0, 0, true); 
   opc.ledStrip(420, 60, width/2, height * 15 / 16, width / 70.0, 0, true); 
   
} 

void draw(){ 
 image(images[frame], 0, 0); 
 frame++; 
 if (frame > end){ 
   frame = start; 
 } 
} 

void keyPressed(){ 
 //play sequence 1 when "1" is pressed 
 //play sequence 2 when "2","3","4" is pressed 
 switch(key){ 
   case '0': 
   start = 0; 
   end = 0; 
   break; 
 case '1': 
   start = 1; 
   end = 1; 
   break; 
 case '2': 
   start = 2; 
   end = 2; 
   break; 
 case '3': 
   start = 3; 
   end = 3; 
   break; 
 case '4': 
   start = 4; 
   end = 4; 
   break; 
 case '5': 
   start = 5; 
   end = 5; 
   break;
 case '6': 
   start = 6; 
   end = 6; 
   break;
 case '7': 
   start = 7; 
   end = 7; 
   break;
 case '8': 
   start = 8; 
   end = 8; 
   break;
  case '9': 
   start = 9; 
   end = 9; 
   break;
   
 } 
} 