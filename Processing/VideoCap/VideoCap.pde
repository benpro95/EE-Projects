/**
*  Please note that the code for interfacing with Capture devices
*  will change in future releases of this library. This is just a
*  filler till something more permanent becomes available.
*
*  For use with the Raspberry Pi camera, make sure the camera is
*  enabled in the Raspberry Pi Configuration tool and add the line
*  "bcm2835_v4l2" (without quotation marks) to the file
*  /etc/modules. After a restart you should be able to see the
*  camera device as /dev/video0.
*/

OPC opc;
import gohai.glvideo.*;
GLCapture video;

void setup() {
size(640, 480, P2D);

{
  // Connect to the local instance of fcserver
  //opc = new OPC(this, "ledproject.local", 7890);
  opc = new OPC(this, "127.0.0.1", 7890);

  // 93-LED LED Fadecandy Ring Address
//  opc.ledRing(0, 1, width/2, height/2, width * 1, 0);
//  opc.ledRing(8, 8, width/2, height/2, width * 1 / 14, 0);
//  opc.ledRing(20, 12, width/2, height/2, width * 2 / 14, 0);
//  opc.ledRing(36, 16, width/2, height/2, width * 3 / 14, 0);
//  opc.ledRing(60, 24, width/2, height/2, width * 4 / 14, 0);
//  opc.ledRing(92, 32, width/2, height/2, width * 5 / 14, 0);

  // Map 420-LED strips 
opc.ledStrip(0, 60, width/2, height * 16 / 32, width / 100.0, 0, false); 
opc.ledStrip(60, 60, width/2, height * 17 / 32, width / 100.0, 0, false);
opc.ledStrip(120, 60, width/2, height * 18 / 32, width / 100.0, 0, false); 
opc.ledStrip(180, 60, width/2, height * 19 / 32, width / 100.0, 0, false); 
opc.ledStrip(240, 60, width/2, height * 20 / 32, width / 100.0, 0, false); 
opc.ledStrip(300, 60, width/2, height * 21 / 32, width / 100.0, 0, false); 
opc.ledStrip(360, 60, width/2, height * 22 / 32, width / 100.0, 0, false); 
opc.ledStrip(420, 60, width/2, height * 23 / 32, width / 100.0, 0, false);

}

String[] devices = GLCapture.list();
println("Devices:");
printArray(devices);
if (0 < devices.length) {
String[] configs = GLCapture.configs(devices[0]);
println("Configs:");
printArray(configs);
}

// this will use the first recognized camera by default
video = new GLCapture(this);

// you could be more specific also, e.g.
//video = new GLCapture(this, devices[0]);
//video = new GLCapture(this, devices[0], 640, 480, 25);
//video = new GLCapture(this, devices[0], configs[0]);

video.start();
}


void draw() {
background(0);
if (video.available()) {
video.read();
}
image(video, 0, 0, width, height);
}
