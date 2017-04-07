
#include <Wire.h>
#include <ArduCAM.h>
#include <SPI.h>
#include "memorysaver.h"

//This code can only work on OV5642_MINI_5MP_PLUS platform.
#if !(defined (OV5642_MINI_5MP_PLUS))
#error Please select the hardware platform and camera module in the ../libraries/ArduCAM/memorysaver.h file
#endif
#define   FRAMES_NUM    0x00

// set pin 4 as the slave select for SPI:
const int CS1 = 4;
bool CAM1_EXIST = false;
bool stopMotion = false;

ArduCAM myCAM1(OV5642, CS1);
long int streamStartTime;

void setup() {
  // put your setup code here, to run once:
  uint8_t vid, pid;
  uint8_t temp;
  Wire.begin();
  Serial.begin(250000); //921600
  Serial.println(F("ArduCAM Start!"));
  // set the CS output:
  pinMode(CS1, OUTPUT);

  // initialize SPI:
  SPI.begin();
  //Check if the 4 ArduCAM Mini 5MP PLus Cameras' SPI bus is OK
  while (1) {
    myCAM1.write_reg(ARDUCHIP_TEST1, 0x55);
    temp = myCAM1.read_reg(ARDUCHIP_TEST1);
    if (temp != 0x55)
    {
      Serial.println(F("SPI1 interface Error!"));
    } else {
      CAM1_EXIST = true;
      Serial.println(F("SPI1 interface OK."));
    }

    if (!(CAM1_EXIST)) {
      delay(1000); continue;
    } else
      break;
  }

  while (1) {
    //Check if the camera module type is OV5642
    myCAM1.rdSensorReg16_8(OV5642_CHIPID_HIGH, &vid);
    myCAM1.rdSensorReg16_8(OV5642_CHIPID_LOW, &pid);
    if ((vid != 0x56) || (pid != 0x42)) {
      Serial.println(F("Can't find OV5642 module!"));
      delay(1000); continue;
    } else {
      Serial.println(F("OV5642 detected.")); break;
    }
  }

  //Change to JPEG capture mode and initialize the OV5640 module
  myCAM1.set_format(JPEG);
  myCAM1.InitCAM();
  myCAM1.write_reg(ARDUCHIP_TIM, VSYNC_LEVEL_MASK);   //VSYNC is active HIGH
  myCAM1.clear_fifo_flag();
  myCAM1.write_reg(ARDUCHIP_FRAMES, FRAMES_NUM);

  //myCAM1.OV5642_set_JPEG_size(OV5642_640x480); delay(1000);
  myCAM1.OV5642_set_JPEG_size(OV5642_320x240); delay(1000);
  delay(1000);
  myCAM1.clear_fifo_flag();
  Serial.println("Ready:,1");
}

void loop() {

  if (CAM1_EXIST && stopMotion) {
    streamStartTime = millis();
    myCAMSendToSerial(myCAM1);
    double fps = ((millis() - streamStartTime) / 1000);
    Serial.println("fps: " + String(1 / fps ));
  }
}


void myCAMSendToSerial(ArduCAM myCAM) {
  char str[8];
  byte buf[5];

  static int i = 0;
  static int k = 0;
  uint8_t temp = 0, temp_last = 0;
  uint32_t length = 0;
  bool is_header = false;

  myCAM.flush_fifo(); //Flush the FIFO
  myCAM.clear_fifo_flag(); //Clear the capture done flag
  myCAM.start_capture();//Start capture

  while (!myCAM.get_bit(ARDUCHIP_TRIG , CAP_DONE_MASK));

  length = myCAM.read_fifo_length();
  Serial.print(F("FifoLength:,"));
  Serial.print(length, DEC);
  Serial.println(",");

  if (length >= MAX_FIFO_SIZE) //8M
  {
    Serial.println(F("Over size."));
    return ;
  }
  if (length == 0 ) //0 kb
  {
    Serial.println(F("Size is 0."));
    return ;
  }

  myCAM.CS_LOW();
  myCAM.set_fifo_burst();

  Serial.print("Image:,");

  while ( length-- )
  {
    temp_last = temp;
    temp =  SPI.transfer(0x00);
    //Read JPEG data from FIFO
    if ( (temp == 0xD9) && (temp_last == 0xFF) ) //If find the end ,break while,
    {
      buf[i++] = temp;  //save the last  0XD9
      //Write the remain bytes in the buffer
      myCAM.CS_HIGH();

      for (int i = 0; i < sizeof(buf); i++) {
        Serial.print(buf[i]); Serial.print(",");
      }

      Serial.println();
      Serial.println(F("Image transfer OK."));
      is_header = false;
      i = 0;
    }
    if (is_header == true)
    {
      //Write image data to buffer if not full
      if (i < 5) {
        buf[i++] = temp;
      } else
      {
        //Stream 5 bytes of raw image data to serial
        myCAM.CS_HIGH();

        for (int i = 0; i < sizeof(buf); i++) {
          Serial.print(buf[i]); Serial.print(",");
        }

        i = 0;
        buf[i++] = temp;
        myCAM.CS_LOW();
        myCAM.set_fifo_burst();

      }
    }
    else if ((temp == 0xD8) & (temp_last == 0xFF))
    {
      is_header = true;
      buf[i++] = temp_last;
      buf[i++] = temp;
    }

  }
}


void serialEvent() {
  if (Serial.available() > 0) {
    uint8_t temp = 0xff, temp_last = 0;
    uint8_t start_capture = 0;
    temp = Serial.read();
    switch (temp)
    {
      case 0:
        temp = 0xff;

        myCAM1.OV5642_set_JPEG_size(OV5642_320x240);
        Serial.println(F("OV5642_320x240")); delay(1000);
        myCAM1.clear_fifo_flag();
        break;
      case 1:
        temp = 0xff;

        myCAM1.OV5642_set_JPEG_size(OV5642_640x480);
        Serial.println(F("OV5642_640x480")); delay(1000);
        myCAM1.clear_fifo_flag();
        break;
      case 2:
        temp = 0xff;

        myCAM1.OV5642_set_JPEG_size(OV5642_1024x768);
        Serial.println(F("OV5642_1024x768")); delay(1000);
        myCAM1.clear_fifo_flag();
        break;
      case 3:
        temp = 0xff;

        myCAM1.OV5642_set_JPEG_size(OV5642_1280x960);
        Serial.println(F("OV5642_1280x960")); delay(1000);
        myCAM1.clear_fifo_flag();
        break;
      case 4:

        temp = 0xff;
        myCAM1.OV5642_set_JPEG_size(OV5642_1600x1200);
        Serial.println(F("OV5642_1600x1200")); delay(1000);
        myCAM1.clear_fifo_flag();
        break;
      case 5:
        temp = 0xff;

        myCAM1.OV5642_set_JPEG_size(OV5642_2048x1536);
        Serial.println(F("OV5642_2048x1536")); delay(1000);
        myCAM1.clear_fifo_flag();
        break;
      case 6:
        temp = 0xff;

        myCAM1.OV5642_set_JPEG_size(OV5642_2592x1944);
        Serial.println(F("OV5642_2592x1944")); delay(1000);
        myCAM1.clear_fifo_flag();
        break;
      case 7:
        {
          if (stopMotion)
            stopMotion = false;
          else
            stopMotion = true;
          Serial.println("Stop Motion Enabled: " + String(stopMotion));
        }
        break;

      case 0x10:
        if (CAM1_EXIST) {
          streamStartTime = millis();
          myCAMSendToSerial(myCAM1);
          double fps = ((millis() - streamStartTime) / 1000);
          Serial.println("Total Time: " + String(fps));
        }
        break;
      default:
        break;
    }
  }
}

