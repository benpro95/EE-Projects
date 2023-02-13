OPC opc;
int numFrames = 20;
int frame = 0; 
int start = 0; 
int end = 19;

PImage[] images = new PImage[numFrames];

void setup(){ 

// fullScreen();
 frameRate(0.75);
size(800,600);
 
 for(int i=0; i<numFrames; i++) {
   String imageName = "pic" + ((i < 20) ? "" : "") + i + ".jpg";
   images[i] = loadImage(imageName);
 }

  opc = new OPC(this, "127.0.0.1", 7890);

   opc.ledStrip(0, 60, width/2, height * 1 / 16, width / 70.0, 0, false); 
   opc.ledStrip(60, 60, width/2, height * 3 / 16, width / 70.0, 0, false);
   opc.ledStrip(120, 60, width/2, height * 5 / 16, width / 70.0, 0, false); 
   opc.ledStrip(180, 60, width/2, height * 7 / 16, width / 70.0, 0, false); 
   opc.ledStrip(240, 60, width/2, height * 9 / 16, width / 70.0, 0, false); 
   opc.ledStrip(300, 60, width/2, height * 11 / 16, width / 70.0, 0, false); 
   opc.ledStrip(360, 60, width/2, height * 13 / 16, width / 70.0, 0, false); 
   opc.ledStrip(420, 60, width/2, height * 15 / 16, width / 70.0, 0, false); 
   
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
  case 'A': 
   start = 10; 
   end = 10; 
   break;
   case 'B': 
   start = 11; 
   end = 11; 
   break; 
   case 'C': 
   start = 12; 
   end = 12;
   break;
   case 'D': 
   start = 13; 
   end = 13; 
   break; 
   case 'E': 
   start = 14; 
   end = 14; 
   break; 
   case 'F': 
   start = 15; 
   end = 15; 
      case 'G': 
   start = 16; 
   end = 16; 
   break; 
      case 'H': 
   start = 17; 
   end = 17; 
   break; 
      case 'I': 
   start = 18; 
   end = 18; 
   break; 
      case 'J': 
   start = 19; 
   end = 19; 
   break; 
   
   
 } 
} 