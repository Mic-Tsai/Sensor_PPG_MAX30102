// ###################################################################################
// # Project: Heart Beat Plotting
// # Engineer:  Mic.Tsai
// # Date:  28 April 2020
// # Objective: Dev.board
// # Usage: ESP8266
// ###################################################################################

#include <Wire.h>
#include "MAX30105.h"

MAX30105 particleSensor;

// # LCD install
#include <Adafruit_SSD1306.h>
#define OLED_Address 0x3C // 0x3C device address of I2C OLED. Few other OLED has 0x3D
Adafruit_SSD1306 oled(128, 64); // create our screen object setting resolution to 128x64

// # Plot 變數宣告區
int a=0;
int lasta=0;
int lastb=0;
int LastTime=0;
int ThisTime;
bool BPMTiming=false;
bool BeatComplete=false;
int BPM=0;

// # 上限下限
#define UpperThreshold 520  
#define LowerThreshold 450

// # Bias
int LevelSea = 0;

// # 平均心跳
const byte RATE_SIZE = 36; //Increase this for more averaging. 18 is good.
byte rates[RATE_SIZE]; //Array of heart rates
byte rateSpot = 0;
long lastBeat = 0; //Time at which the last beat occurred

float beatsPerMinute;
int beatAvg;

int CHECK = 0;
byte TestledBrightness = 50; //Options: 0=Off to 255=50mA //0x1F

void setup()
{
  Serial.begin(9600);

  //Display show...
  // initialize with the I2C addr 0x3C
  Serial.print("Initializing...Display");
  if(!oled.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
  Serial.println(F("SSD1306 allocation failed"));
  for(;;);
  }
    
  // Clear the buffer.
  oled.clearDisplay();
  oled.setTextSize(2);

    // Display "8WA - Current Senser"
    oled.setTextSize(2); oled.setTextColor(WHITE); oled.setCursor(0,0); oled.println("HeartRate");
    oled.setTextSize(1); oled.setCursor(0,20); oled.println("PPG Senser v1");
    oled.display();
    delay(1000);

    // Display "By Mic.Tsai - BU8"
    //display.clearDisplay();
    oled.setTextSize(1); oled.setTextColor(WHITE); 
    oled.setCursor(60,40); oled.println("Engineer:");
    oled.setCursor(75,50); oled.println("Mic.Tsai");
    oled.display();
    delay(1000);
    oled.clearDisplay();   // Clear the buffer.
    oled.display();

  Serial.println("OK!");

  // Initialize sensor
  Serial.println("Initializing...MAX30102");
  particleSensor.begin(Wire, I2C_SPEED_FAST);

  //Setup to sense a nice looking saw tooth on the plotter
  byte ledBrightness = 255; //Options: 0=Off to 255=50mA //0x1F
  byte sampleAverage = 32; //Options: 1, 2, 4, 8, 16, 32
  byte ledMode = 2; //Options: 1 = Red only, 2 = Red + IR, 3 = Red + IR + Green
  int sampleRate = 3200; //Options: 50, 100, 200, 400, 800, 1000, 1600, 3200
  int pulseWidth = 411; //Options: 69, 118, 215, 411
  int adcRange = 16384; //Options: 2048, 4096, 8192, 16384

  particleSensor.setup(ledBrightness, sampleAverage, ledMode, sampleRate, pulseWidth, adcRange); //Configure sensor with these settings
  particleSensor.enableDIETEMPRDY(); //Enable the temp ready interrupt. This is required.
}

void loop()
{
    int value = 0;
    int valueOrignal=particleSensor.getRed();
    float temperature = particleSensor.readTemperature();

    LevelSea = (LevelSea + valueOrignal) / (2) ;
    value = valueOrignal-LevelSea + 500 ;
//    Serial.println(value); //Send raw data to plotter
    Serial.println(valueOrignal); //Send raw data to plotter

    if(a>127)
    {
      oled.clearDisplay();
      a=0;
      lasta=a;
    }
    
    ThisTime=millis();
    oled.setTextColor(WHITE);

    int b=80-(value/8);

    oled.writeLine(lasta,lastb,a,b,WHITE);
    lastb=b;
    lasta=a;
    
    if(value<LowerThreshold)
    {
      if(BeatComplete)
      {
      BPM=ThisTime-LastTime;
      BPM=int(60/(float(BPM)/1000));
      BPMTiming=false;
      BeatComplete=false;
      tone(0,1000,250);
      }
      if(BPMTiming==false)
      {
        LastTime=millis();
        BPMTiming=true;
      }
    }
    if((value>UpperThreshold)&(BPMTiming))
      BeatComplete=true;

    beatsPerMinute = BPM;

    if (beatsPerMinute < 130 && beatsPerMinute >= 40)
    {
      rates[rateSpot++] = (byte)beatsPerMinute; //Store this reading in the array
      rateSpot %= RATE_SIZE; //Wrap variable

      //Take average of readings
      beatAvg = 0;
      for (byte x = 0 ; x < RATE_SIZE ; x++)
        beatAvg += rates[x];
        beatAvg /= RATE_SIZE;     
    }
    
    oled.writeFillRect(0,40,128,26,BLACK);
    oled.setTextSize(1); 
    oled.setCursor(0,45); oled.print(BPM);
    oled.setTextSize(1); oled.setCursor(45,40);oled.print("Avg");
    oled.setTextSize(2); oled.setCursor(32,50);
    oled.print(" BPM:"); oled.print(beatAvg);
    a++;
    oled.setTextSize(1);oled.setCursor(0,55);oled.print(temperature,1);oled.print("C");
    oled.display();
}
