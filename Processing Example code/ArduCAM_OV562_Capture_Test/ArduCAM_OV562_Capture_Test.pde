
import processing.serial.*;       // import the Processing serial library

Serial myPort;                    // The serial port

//NEW ADDITIONS
byte rawBytes[];
int sensorNum = 0;
PImage img;  
long picNum = 0;

void setup() {
  size(640, 480);

  // List all the available serial ports
  // if using Processing 2.1 or later, use Serial.printArray()
  println(Serial.list());


  // Change the 0 to the appropriate number of the serial port
  // that your microcontroller is attached to.
  myPort = new Serial(this, Serial.list()[0], 250000);

  // read bytes into a buffer until you get a linefeed (ASCII 10):
  myPort.bufferUntil('\n');

  // draw with smooth edges:
  smooth();

  img = createImage(320, 240, RGB);
  img.loadPixels();
  //frameRate(600);
  myPort.write(0x10);
}


void draw() {

  background(255);
  image(img, 0, 0, width, height-100);
  fill(0);
  text("Help:", 20, height-80);
  text("- Press key 1-7 to change image resolution", 30, height-60);
  text("- Press key S to capture a still photo", 30, height-40);
  text("- Press key C to enable/disable StopMotion capture", 30, height-20);
}


// serialEvent  method is run automatically by the Processing applet
// whenever the buffer reaches the  byte value set in the bufferUntil()
// method in the setup():

void serialEvent(Serial myPort) {
  while (myPort.available() > 0) {

    String incoming[];
    String myString = myPort.readStringUntil('\n');

    myString = trim(myString);
    incoming = split(myString, ',');

    if (incoming.length > 1) {
      if (incoming[0].equals("FifoLength:")) {
        //initialize raw data byte array to the size of the picture
        rawBytes = new byte[int(incoming[1])];
        println("Picture Size: "+incoming[1]+" bytes");
      } else if (incoming[0].equals("Image:")) {
        int x = 0;
        for (int i = 1; i < incoming.length; i++) {
          try {
            //add raw jpeg incoming bytes to byte array
            rawBytes[x]= (byte)int(incoming[i]);
            x++;
          }
          catch(RuntimeException e) {
            println(e.getMessage());
          }
        }
        try {
          //Save raw data to file
          String fname = "capture#"+picNum+"_"+day()+month()+year()+".jpg";
          saveBytes("data/capture/"+fname, rawBytes);

          // Open saved picture for local display
          img = loadImage("/data/capture/"+fname);
          picNum++;
        }
        catch(RuntimeException e) {
          println(e.getMessage());
        }
      } else if (incoming[0].equals("Ready:")) {
        myPort.write(0x10);
        println("Starting Capture");
      }
    } else {
      println(myString);
    }
  }
}

void keyPressed() {
  switch(key) {
  case 's':
    myPort.write(0x10);
    println("Starting Capture");
    break;

  case 'c':
    myPort.write(7);
    break;

  case '1':
    myPort.write(0);
    println("Set Image Resolution: 320x240");
    break;

  case '2':
    myPort.write(1);
    println("Set Image Resolution: 640x480");
    break;

  case '3':
    myPort.write(2);
    println("Set Image Resolution: 1024x768");
    break;

  case '4':
    myPort.write(3);
    println("Set Image Resolution: 1280x960");
    break;

  case '5':
    myPort.write(4);
    println("Set Image Resolution: 1600x1200");
    break;

  case '6':
    myPort.write(5);
    println("Set Image Resolution: 2048x1536");
    break;

  case '7':
    myPort.write(6);
    println("Set Image Resolution: 2592x1944");
    break;

  default:
    println("Unknown Command: "+key);
    break;
  }
}