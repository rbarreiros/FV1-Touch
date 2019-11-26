// https://www.facebook.com/1337fx/

// https://www.instagram.com/1337_fx/


#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include "XPT2046_Touchscreen.h"
#include <SPI.h>

#include "TypoHoop20pt.h"
#include "Typohoop10pt.h"
#include "uparrow.h";
#include "downarrow.h";
#include "bootlogo.h";
#include "digi.h";
#include "meter.h";

#define meterGrey 0x62c7
#define meterCream 0xd610
#define LTBLUE    0xB6DF
#define LTTEAL    0xBF5F
#define LTGREEN         0xBFF7
#define LTCYAN    0xC7FF
#define LTRED           0xFD34
#define LTMAGENTA       0xFD5F
#define LTYELLOW        0xFFF8
#define LTORANGE        0xFE73
#define LTPINK          0xFDDF
#define LTPURPLE  0xCCFF
#define LTGREY          0xE71C

#define BLUE            0x001F
#define TEAL    0x0438
#define GREEN           0x07E0
#define CYAN          0x07FF
#define RED           0xF800
#define MAGENTA       0xF81F
#define YELLOW        0xFFE0
#define ORANGE        0xFD20
#define PINK          0xF81F
#define PURPLE    0x801F
#define GREY        0xC618
#define WHITE         0xFFFF
#define BLACK         0x0000

#define DKBLUE        0x000D
#define DKTEAL    0x020C
#define DKGREEN       0x32A3
#define DKCYAN        0x03EF
#define DKRED         0x6000
#define DKMAGENTA       0x8008
#define DKYELLOW        0x8400
#define DKORANGE        0x8200
#define DKPINK          0x9009
#define DKPURPLE      0x4010
#define DKGREY        0x4A49

// For the STM32
#define TFT_DC PB9
#define TFT_CS PB8

#define CS_PIN  PB6

// Variables will change:
int buttonPushCounter = 1;   // counter for the number of button presses
int buttonState = 0;         // current state of the button
int lastButtonState = 0;     // previous state of the button
int reading = 0;
int meterOn = false;

volatile byte oldPnumber = 0;
volatile byte NewPnumber = 0;

double p1volts;
double p2volts;
double p3volts;
double p4volts;
double p5volts;

boolean graph_1 = true;
boolean graph_2 = true;
boolean graph_3 = true;
boolean graph_4 = true;
boolean graph_5 = true;

float ltx = 0;    // Saved x coord of bottom of needle
uint16_t osx = 80, osy = 80; // Saved x & y coords
uint32_t updateTime = 0;       // time for next update

int old_analog =  -999; // Value last displayed
int old_digital = -999; // Value last displayed


Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);
SPIClass mySPI(1); //Create an SPI instance on SPI1 port.
XPT2046_Touchscreen ts(CS_PIN); // Chip Select pin, SPI port


void setup() {
  disableDebugPorts();
  pinMode(PB12, OUTPUT); //S0
  pinMode(PB13, OUTPUT); //S1
  pinMode(PB14, OUTPUT); //S2
  pinMode(PB7, OUTPUT); //eeprom1
  pinMode(PB5, OUTPUT); //eeprom2

  pinMode(PA0, INPUT); // pot1
  pinMode(PA1, INPUT); // pot2
  pinMode(PA2, INPUT); // pot3
  pinMode(PA4, INPUT); // pot4 volume
  pinMode(PA3, INPUT); // pot5 mix
  // Analog meter input is PB0
  pinMode(PB3, INPUT); // fv1 clip

  pinMode(PB4, OUTPUT); //fv1 pin13, high ext, low int

  digitalWrite(PB7, LOW); //eprom1 on
  digitalWrite(PB5, HIGH); //eprom2 off
  digitalWrite(PB4, HIGH); //high ext, low int
  delay (100);
  digitalWrite(PB12, HIGH); //s0
  digitalWrite(PB13, HIGH); //s1
  digitalWrite(PB14, HIGH); //s2

  Serial.begin(9600);
  mySPI.begin();
  tft.begin();
  ts.begin();
  ts.setRotation(1);
  while (!Serial && (millis() <= 1000));
  tft.fillScreen(ILI9341_BLACK);
  tft.setRotation(0);

  // BOOTSCREEN

  tft.drawBitmap(0, 130, boot, 240, 44, RED);
  delay (100);
  tft.drawBitmap(0, 130, boot, 240, 44, BLUE);
  tft.drawBitmap(0, 130, boot, 240, 44, RED);
  delay (100);
  tft.drawBitmap(0, 130, boot, 240, 44, BLUE);
  tft.drawBitmap(0, 130, boot, 240, 44, RED);

  delay (1000);



  tft.fillScreen(ILI9341_BLACK);
  tft.setFont(&TypoHoopRegular20pt7b);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(1);
  tft.fillRect(0, 0, 240, 100, ILI9341_RED);
  tft.setCursor(67, 40);
  tft.print("Qubit");
  tft.setCursor(67, 85);
  tft.print("Delay");

  tft.setFont(&TypoHoopRegular10pt7b);
  tft.setTextColor(ILI9341_WHITE);
  tft.fillRect(8, 113, 139, 22, ILI9341_BLACK);
  tft.setCursor(10, 128);
  tft.print("Time");
  tft.fillRect(8, 156, 138, 22, ILI9341_BLACK);
  tft.setCursor(10, 171);
  tft.print("Modulation");
  tft.fillRect(8, 199, 139, 22, ILI9341_BLACK);
  tft.setCursor(10, 214);
  tft.print("Feedback");
  tft.setCursor(10, 257);
  tft.print("Volume");
  tft.setCursor(10, 300);
  tft.print("Mix");

  tft.fillRect(179, 182, 40, 45, ILI9341_WHITE);
  tft.fillRect(179, 260, 40, 45, ILI9341_WHITE);
  tft.drawBitmap(170, 170, upbutton, 64, 64, ILI9341_RED);
  tft.drawBitmap(170, 250, downbutton, 64, 64, ILI9341_RED);

  tft.setFont(&DSEG7_Classic_Mini_Bold_38);
  tft.fillRect(165, 105, 70, 50, ILI9341_BLACK);
  tft.setTextColor(ILI9341_GREEN);
  tft.setTextSize(1);
  tft.setCursor(185, 152);
  tft.print(buttonPushCounter);

  digitalWrite(PB12, LOW); //s0
  digitalWrite(PB13, LOW); //s1
  digitalWrite(PB14, LOW); //s2

  updateTime = millis(); // Next update time

}

void loop()

{

  const uint32_t analogReadInterval = 1;
  if (millis() - updateTime >= analogReadInterval) {
    updateTime += analogReadInterval;
    int raw = analogRead(PB0);
    Serial.print("Raw: "), Serial.println(raw);
    reading = map(raw, 0, 4195, 0, 100);
    if (meterOn == true)
      plotNeedle(reading, 0); // Draw needle
  }

  p1volts = analogRead(PA0);
  p2volts = analogRead(PA1);
  p3volts = analogRead(PA2);
  p4volts = analogRead(PA4);
  p5volts = analogRead(PA3);
  DrawBarChartP1(tft, 10, 134, 140, 7, 0, 4195 , 409, p1volts, 0, 0, PINK, DKRED, BLACK, WHITE, BLACK, "POT1", graph_1);
  DrawBarChartP1(tft, 10, 177, 140, 7, 0, 4195 , 409, p2volts, 0, 0, GREEN, DKGREY, BLACK, WHITE, BLACK, "POT2", graph_2);
  DrawBarChartP1(tft, 10, 220, 140, 7, 0, 4195 , 409, p3volts, 0, 0, CYAN, DKBLUE, BLACK, WHITE, BLACK, "POT3", graph_3);
  DrawBarChartP1(tft, 10, 263, 140, 7, 0, 4195 , 409, p4volts, 0, 0, RED, DKGREEN, BLACK, WHITE, BLACK, "POT4", graph_4);
  DrawBarChartP1(tft, 10, 306, 140, 7, 0, 4195 , 409, p5volts, 0, 0, YELLOW, DKORANGE, BLACK, WHITE, BLACK, "POT5", graph_5);
}
//Draw Pot Values
void DrawBarChartP1(Adafruit_ILI9341 & d, double x , double y , double w, double h , double loval , double hival , double inc , double curval ,  int dig , int dec, unsigned int barcolor, unsigned int voidcolor, unsigned int bordercolor, unsigned int textcolor, unsigned int backcolor, String label, boolean & redraw)
{

  double stepval, range;
  double mx, level;
  double i, data;

  if (redraw == true) {
    redraw = false;
    d.drawRect(x , y , w, h, bordercolor);
    stepval =  inc * (double (w) / (double (hival - loval))) - .001;
    for (i = 0; i <= w; i += stepval) {
    }
  }
  level = (w * (((curval - loval) / (hival - loval))));
  d.fillRect(x + level + 1, y + 1, w - level - 2, h - 2,  voidcolor);
  d.fillRect(x + 1, y + 1 , level - 1,  h - 2, barcolor);
  int sensorValue = analogRead(PB0);
  //Serial.println(sensorValue);

  //TOUCHSCREEN

  if (ts.touched()) {
    TS_Point p = ts.getPoint();

    // Button Code

    if (buttonState != lastButtonState)   // compare the buttonState to its previous state

      // save the current state as the last state, for next time through the loop
      lastButtonState = buttonState;


    // Ive had some ili9341 screens with flipped touch pixels,
    // change screen type if up & down & meter buttons dont work, in 3 places below

    if ((p.x > 1350) && (p.x < 2020) && (p.y > 340) && (p.y < 1450)) // Screen type 1
      //if ((p.x < 2762) && (p.x > 2104) && (p.y > 428) && (p.y < 1500)) // Screen type 2
      buttonPushCounter++;
    if (buttonPushCounter >= 22)
      buttonPushCounter = 1;
    meterOn = false;
    delay(200);
    {
      if ((p.x > 480) && (p.x < 1100) && (p.y > 340) && (p.y < 1450)) // Screen type 1
        //if ((p.x < 3598) && (p.x > 2998) && (p.y > 428) && (p.y < 1500)) // Screen type 2
        buttonPushCounter--;
      if (buttonPushCounter <= 0)
        buttonPushCounter = 21;
      meterOn = false;
      {
        if (buttonPushCounter <= 9)
        {
          tft.fillRect(165, 105, 70, 50, ILI9341_BLACK);
          tft.setTextColor(ILI9341_GREEN);
          tft.setTextSize(1);
          tft.setCursor(185, 152);
          tft.setFont(&DSEG7_Classic_Mini_Bold_38);
          tft.print(buttonPushCounter);
        }

        if (buttonPushCounter >= 10)
        {
          tft.fillRect(165, 105, 70, 50, ILI9341_BLACK);
          tft.setTextColor(ILI9341_GREEN);
          tft.setTextSize(1);
          tft.setCursor(165, 152);
          tft.setFont(&DSEG7_Classic_Mini_Bold_38);
          tft.print(buttonPushCounter);
        }

        {
          switch (buttonPushCounter)
          {
            case 1:
              tft.setFont(&TypoHoopRegular20pt7b);
              tft.setTextColor(ILI9341_WHITE);
              tft.setTextSize(1);
              tft.fillRect(0, 0, 240, 100, ILI9341_RED);
              tft.setCursor(67, 40);
              tft.print("Qubit");
              tft.setCursor(67, 85);
              tft.print("Delay");

              tft.setFont(&TypoHoopRegular10pt7b);
              tft.setTextColor(ILI9341_WHITE);
              tft.fillRect(8, 113, 139, 22, ILI9341_BLACK);
              tft.setCursor(10, 128);
              tft.print("Time");
              tft.fillRect(8, 156, 138, 22, ILI9341_BLACK);
              tft.setCursor(10, 171);
              tft.print("Modulation");
              tft.fillRect(8, 199, 139, 22, ILI9341_BLACK);
              tft.setCursor(10, 214);
              tft.print("Feedback");

              digitalWrite(PB4, HIGH); //high ext, low int
              digitalWrite(PB7, LOW); //eprom1 on
              digitalWrite(PB5, HIGH); //eprom2 off
              digitalWrite(PB12, LOW); //s0
              digitalWrite(PB13, LOW); //s1
              digitalWrite(PB14, LOW); //s2
              delay(50);
              break;

            case 2:
              tft.setFont(&TypoHoopRegular20pt7b);
              tft.setTextColor(ILI9341_WHITE);
              tft.setTextSize(1);
              tft.fillRect(0, 0, 240, 100, ILI9341_RED);
              tft.setCursor(10, 40);
              tft.print("Supernova");
              tft.setCursor(67, 85);
              tft.print("Delay");

              tft.setFont(&TypoHoopRegular10pt7b);
              tft.setTextColor(ILI9341_WHITE);
              tft.fillRect(8, 113, 139, 22, ILI9341_BLACK);
              tft.setCursor(10, 128);
              tft.print("Time");
              tft.fillRect(8, 156, 138, 22, ILI9341_BLACK);
              tft.setCursor(10, 171);
              tft.print("Filter");
              tft.fillRect(8, 199, 139, 22, ILI9341_BLACK);
              tft.setCursor(10, 214);
              tft.print("Feedback");

              digitalWrite(PB7, LOW); //eprom1 on
              digitalWrite(PB5, HIGH); //eprom2 off
              digitalWrite(PB12, LOW); //s0
              digitalWrite(PB13, LOW); //s1
              digitalWrite(PB14, HIGH); //s2
              delay(50);
              break;

            case 3:
              tft.setFont(&TypoHoopRegular20pt7b);
              tft.setTextColor(ILI9341_WHITE);
              tft.setTextSize(1);
              tft.fillRect(0, 0, 240, 100, ILI9341_RED);
              tft.setCursor(10, 43);
              tft.print("Modulated");
              tft.setCursor(67, 85);
              tft.print("Delay");


              tft.setFont(&TypoHoopRegular10pt7b);
              tft.setTextColor(ILI9341_WHITE);
              tft.fillRect(8, 113, 139, 22, ILI9341_BLACK);
              tft.setCursor(10, 128);
              tft.print("Time");
              tft.fillRect(8, 156, 138, 22, ILI9341_BLACK);
              tft.setCursor(10, 171);
              tft.print("Feedback");
              tft.fillRect(8, 199, 139, 22, ILI9341_BLACK);
              tft.setCursor(10, 214);
              tft.print("Modulation");


              digitalWrite(PB7, LOW); //eprom1 on
              digitalWrite(PB5, HIGH); //eprom2 off
              digitalWrite(PB12, LOW); //s0
              digitalWrite(PB13, HIGH); //s1
              digitalWrite(PB14, LOW); //s2
              delay(50);
              break;

            case 4:
              tft.setFont(&TypoHoopRegular20pt7b);
              tft.setTextColor(ILI9341_WHITE);
              tft.setTextSize(1);
              tft.fillRect(0, 0, 240, 100, ILI9341_RED);
              tft.setCursor(75, 40);
              tft.print("Echo");
              tft.setCursor(50, 85);
              tft.print("Reverb");

              tft.setFont(&TypoHoopRegular10pt7b);
              tft.setTextColor(ILI9341_WHITE);
              tft.fillRect(8, 113, 139, 22, ILI9341_BLACK);
              tft.setCursor(10, 128);
              tft.print("Reverb Level");
              tft.fillRect(8, 156, 138, 22, ILI9341_BLACK);
              tft.setCursor(10, 171);
              tft.print("Delay Time");
              tft.fillRect(8, 199, 139, 22, ILI9341_BLACK);
              tft.setCursor(10, 214);
              tft.print("Echo Level");

              digitalWrite(PB7, LOW); //eprom1 on
              digitalWrite(PB5, HIGH); //eprom2 off
              digitalWrite(PB12, LOW); //s0
              digitalWrite(PB13, HIGH); //s1
              digitalWrite(PB14, HIGH); //s2
              delay(50);
              break;

            case 5:
              tft.setFont(&TypoHoopRegular20pt7b);
              tft.setTextColor(ILI9341_WHITE);
              tft.setTextSize(1);
              tft.fillRect(0, 0, 240, 100, ILI9341_RED);
              tft.setCursor(30, 40);
              tft.print("Shimmer");
              tft.setCursor(50, 85);
              tft.print("Reverb");

              tft.setFont(&TypoHoopRegular10pt7b);
              tft.setTextColor(ILI9341_WHITE);
              tft.fillRect(8, 113, 139, 22, ILI9341_BLACK);
              tft.setCursor(10, 128);
              tft.print("Dwell");
              tft.fillRect(8, 156, 138, 22, ILI9341_BLACK);
              tft.setCursor(10, 171);
              tft.print("Pitch");
              tft.fillRect(8, 199, 139, 22, ILI9341_BLACK);
              tft.setCursor(10, 214);
              tft.print("Blend");

              digitalWrite(PB7, LOW); //eprom1 on
              digitalWrite(PB5, HIGH); //eprom2 off
              digitalWrite(PB12, HIGH); //s0
              digitalWrite(PB13, LOW); //s1
              digitalWrite(PB14, LOW); //s2
              delay(50);
              break;

            case 6:
              tft.setFont(&TypoHoopRegular20pt7b);
              tft.setTextColor(ILI9341_WHITE);
              tft.setTextSize(1);
              tft.fillRect(0, 0, 240, 100, ILI9341_RED);
              tft.setCursor(25, 40);
              tft.print("Sample &");
              tft.setCursor(80, 85);
              tft.print("Hold");

              tft.setFont(&TypoHoopRegular10pt7b);
              tft.setTextColor(ILI9341_WHITE);
              tft.fillRect(8, 113, 139, 22, ILI9341_BLACK);
              tft.setCursor(10, 128);
              tft.print("Time");
              tft.fillRect(8, 156, 138, 22, ILI9341_BLACK);
              tft.setCursor(10, 171);
              tft.print("Feedback");
              tft.fillRect(8, 199, 139, 22, ILI9341_BLACK);
              tft.setCursor(10, 214);
              tft.print("Modulation");

              digitalWrite(PB7, LOW); //eprom1 on
              digitalWrite(PB5, HIGH); //eprom2 off
              digitalWrite(PB12, HIGH); //s0
              digitalWrite(PB13, LOW); //s1
              digitalWrite(PB14, HIGH); //s2
              delay(50);
              break;

            case 7:
              tft.setFont(&TypoHoopRegular20pt7b);
              tft.setTextColor(ILI9341_WHITE);
              tft.setTextSize(1);
              tft.fillRect(0, 0, 240, 100, ILI9341_RED);
              tft.setCursor(10, 40);
              tft.print("Modulated");
              tft.setCursor(50, 85);
              tft.print("Reverb");

              tft.setFont(&TypoHoopRegular10pt7b);
              tft.setTextColor(ILI9341_WHITE);
              tft.fillRect(8, 113, 139, 22, ILI9341_BLACK);
              tft.setCursor(10, 128);
              tft.print("Dwell");
              tft.fillRect(8, 156, 138, 22, ILI9341_BLACK);
              tft.setCursor(10, 171);
              tft.print("Depth");
              tft.fillRect(8, 199, 139, 22, ILI9341_BLACK);
              tft.setCursor(10, 214);
              tft.print("Rate");

              digitalWrite(PB7, LOW); //eprom1 on
              digitalWrite(PB5, HIGH); //eprom2 off
              digitalWrite(PB12, HIGH); //s0
              digitalWrite(PB13, HIGH); //s1
              digitalWrite(PB14, LOW); //s2
              delay(50);
              break;

            case 8:
              tft.setFont(&TypoHoopRegular20pt7b);
              tft.setTextColor(ILI9341_WHITE);
              tft.setTextSize(1);
              tft.fillRect(0, 0, 240, 100, ILI9341_RED);
              tft.setCursor(30, 40);
              tft.print("Glitch Bit");
              tft.setCursor(67, 85);
              tft.print("Delay");

              tft.setFont(&TypoHoopRegular10pt7b);
              tft.setTextColor(ILI9341_WHITE);
              tft.fillRect(8, 113, 139, 22, ILI9341_BLACK);
              tft.setCursor(10, 128);
              tft.print("Delay 1");
              tft.fillRect(8, 156, 138, 22, ILI9341_BLACK);
              tft.setCursor(10, 171);
              tft.print("Delay 2");
              tft.fillRect(8, 199, 139, 22, ILI9341_BLACK);
              tft.setCursor(10, 214);
              tft.print("Feedback");

              digitalWrite(PB7, LOW); //eprom1 on
              digitalWrite(PB5, HIGH); //eprom2 off
              digitalWrite(PB12, HIGH); //s0
              digitalWrite(PB13, HIGH); //s1
              digitalWrite(PB14, HIGH); //s2
              delay(50);
              break;

            case 9:
              tft.setFont(&TypoHoopRegular20pt7b);
              tft.setTextColor(ILI9341_WHITE);
              tft.setTextSize(1);
              tft.fillRect(0, 0, 240, 100, ILI9341_RED);
              tft.setCursor(15, 40);
              tft.print("Daydream");
              tft.setCursor(67, 85);
              tft.print("Delay");

              tft.setFont(&TypoHoopRegular10pt7b);
              tft.setTextColor(ILI9341_WHITE);
              tft.fillRect(8, 113, 139, 22, ILI9341_BLACK);
              tft.setCursor(10, 128);
              tft.print("Time");
              tft.fillRect(8, 156, 138, 22, ILI9341_BLACK);
              tft.setCursor(10, 171);
              tft.print("Feedback");
              tft.fillRect(8, 199, 139, 22, ILI9341_BLACK);
              tft.setCursor(10, 214);
              tft.print("Filter");

              digitalWrite(PB7, HIGH); //eprom1 off
              digitalWrite(PB5, LOW); //eprom2 on
              digitalWrite(PB12, LOW); //s0
              digitalWrite(PB13, LOW); //s1
              digitalWrite(PB14, LOW); //s2
              delay(50);
              break;

            case 10:
              tft.setFont(&TypoHoopRegular20pt7b);
              tft.setTextColor(ILI9341_WHITE);
              tft.setTextSize(1);
              tft.fillRect(0, 0, 240, 100, ILI9341_RED);
              tft.setCursor(30, 40);
              tft.print("Starfield");
              tft.setCursor(67, 85);
              tft.print("Delay");

              tft.setFont(&TypoHoopRegular10pt7b);
              tft.setTextColor(ILI9341_WHITE);
              tft.fillRect(8, 113, 139, 22, ILI9341_BLACK);
              tft.setCursor(10, 128);
              tft.print("Time");
              tft.fillRect(8, 156, 138, 22, ILI9341_BLACK);
              tft.setCursor(10, 171);
              tft.print("Feedback");
              tft.fillRect(8, 199, 139, 22, ILI9341_BLACK);
              tft.setCursor(10, 214);
              tft.print("Phaser");

              digitalWrite(PB7, HIGH); //eprom1 off
              digitalWrite(PB5, LOW); //eprom2 on
              digitalWrite(PB12, LOW); //s0
              digitalWrite(PB13, LOW); //s1
              digitalWrite(PB14, HIGH); //s2
              delay(50);
              break;

            case 11:
              tft.setFont(&TypoHoopRegular20pt7b);
              tft.setTextColor(ILI9341_WHITE);
              tft.setTextSize(1);
              tft.fillRect(0, 0, 240, 100, ILI9341_RED);
              tft.setCursor(20, 40);
              tft.print("Dual Pitch");
              tft.setCursor(51, 85);
              tft.print("Shifter");

              tft.setFont(&TypoHoopRegular10pt7b);
              tft.setTextColor(ILI9341_WHITE);
              tft.fillRect(8, 113, 139, 22, ILI9341_BLACK);
              tft.setCursor(10, 128);
              tft.print("Pitch 1");
              tft.fillRect(8, 156, 138, 22, ILI9341_BLACK);
              tft.setCursor(10, 171);
              tft.print("1-Mix-2");
              tft.fillRect(8, 199, 139, 22, ILI9341_BLACK);
              tft.setCursor(10, 214);
              tft.print("Pitch 2");

              digitalWrite(PB7, HIGH); //eprom1 off
              digitalWrite(PB5, LOW); //eprom2 on
              digitalWrite(PB12, LOW); //s0
              digitalWrite(PB13, HIGH); //s1
              digitalWrite(PB14, LOW); //s2
              delay(50);
              break;

            case 12:
              tft.setFont(&TypoHoopRegular20pt7b);
              tft.setTextColor(ILI9341_WHITE);
              tft.setTextSize(1);
              tft.fillRect(0, 0, 240, 100, ILI9341_RED);
              tft.setCursor(62, 40);
              tft.print("Triple");
              tft.setCursor(67, 85);
              tft.print("Delay");

              tft.setFont(&TypoHoopRegular10pt7b);
              tft.setTextColor(ILI9341_WHITE);
              tft.fillRect(8, 113, 139, 22, ILI9341_BLACK);
              tft.setCursor(10, 128);
              tft.print("Delay 1");
              tft.fillRect(8, 156, 138, 22, ILI9341_BLACK);
              tft.setCursor(10, 171);
              tft.print("Delay 2");
              tft.fillRect(8, 199, 139, 22, ILI9341_BLACK);
              tft.setCursor(10, 214);
              tft.print("Delay 3");

              digitalWrite(PB7, HIGH); //eprom1 off
              digitalWrite(PB5, LOW); //eprom2 on
              digitalWrite(PB12, LOW); //s0
              digitalWrite(PB13, HIGH); //s1
              digitalWrite(PB14, HIGH); //s2
              delay(50);
              break;

            case 13:
              tft.setFont(&TypoHoopRegular20pt7b);
              tft.setTextColor(ILI9341_WHITE);
              tft.setTextSize(1);
              tft.fillRect(0, 0, 240, 100, ILI9341_RED);
              tft.setCursor(35, 40);
              tft.print("Reverse");
              tft.setCursor(67, 85);
              tft.print("Delay");

              tft.setFont(&TypoHoopRegular10pt7b);
              tft.setTextColor(ILI9341_WHITE);
              tft.fillRect(8, 113, 139, 22, ILI9341_BLACK);
              tft.setCursor(10, 128);
              tft.print("Sample");
              tft.fillRect(8, 156, 138, 22, ILI9341_BLACK);
              tft.setCursor(10, 171);
              tft.print("Feedback");
              tft.fillRect(8, 199, 139, 22, ILI9341_BLACK);
              tft.setCursor(10, 214);
              tft.print("Delay");

              digitalWrite(PB7, HIGH); //eprom1 off
              digitalWrite(PB5, LOW); //eprom2 on
              digitalWrite(PB12, HIGH); //s0
              digitalWrite(PB13, LOW); //s1
              digitalWrite(PB14, LOW); //s2
              delay(50);
              break;

            case 14:
              tft.setFont(&TypoHoopRegular20pt7b);
              tft.setTextColor(ILI9341_WHITE);
              tft.setTextSize(1);
              tft.fillRect(0, 0, 240, 100, ILI9341_RED);
              tft.setCursor(78, 40);
              tft.print("Wah");
              tft.setCursor(50, 85);
              tft.print("Reverb");

              tft.setFont(&TypoHoopRegular10pt7b);
              tft.setTextColor(ILI9341_WHITE);
              tft.fillRect(8, 113, 139, 22, ILI9341_BLACK);
              tft.setCursor(10, 128);
              tft.print("Reverb");
              tft.fillRect(8, 156, 138, 22, ILI9341_BLACK);
              tft.setCursor(10, 171);
              tft.print("Rate");
              tft.fillRect(8, 199, 139, 22, ILI9341_BLACK);
              tft.setCursor(10, 214);
              tft.print("Wah");

              digitalWrite(PB7, HIGH); //eprom1 off
              digitalWrite(PB5, LOW); //eprom2 on
              digitalWrite(PB12, HIGH); //s0
              digitalWrite(PB13, LOW); //s1
              digitalWrite(PB14, HIGH); //s2
              delay(50);
              break;

            case 15:
              tft.setFont(&TypoHoopRegular20pt7b);
              tft.setTextColor(ILI9341_WHITE);
              tft.setTextSize(1);
              tft.fillRect(0, 0, 240, 100, ILI9341_RED);
              tft.setCursor(45, 40);
              tft.print("Vibrato");
              tft.setCursor(50, 85);
              tft.print("Reverb");

              tft.setFont(&TypoHoopRegular10pt7b);
              tft.setTextColor(ILI9341_WHITE);
              tft.fillRect(8, 113, 139, 22, ILI9341_BLACK);
              tft.setCursor(10, 128);
              tft.print("Reverb");
              tft.fillRect(8, 156, 138, 22, ILI9341_BLACK);
              tft.setCursor(10, 171);
              tft.print("Rate");
              tft.fillRect(8, 199, 139, 22, ILI9341_BLACK);
              tft.setCursor(10, 214);
              tft.print("Vibrato");

              digitalWrite(PB7, HIGH); //eprom1 off
              digitalWrite(PB5, LOW); //eprom2 on
              digitalWrite(PB12, HIGH); //s0
              digitalWrite(PB13, HIGH); //s1
              digitalWrite(PB14, LOW); //s2
              delay(50);
              break;


            case 16:
              tft.setFont(&TypoHoopRegular20pt7b);
              tft.setTextColor(ILI9341_WHITE);
              tft.setTextSize(1);
              tft.fillRect(0, 0, 240, 100, ILI9341_RED);
              tft.setCursor(48, 40);
              tft.print("Phaser");
              tft.setCursor(50, 85);
              tft.print("Reverb");

              tft.setFont(&TypoHoopRegular10pt7b);
              tft.setTextColor(ILI9341_WHITE);
              tft.fillRect(8, 113, 139, 22, ILI9341_BLACK);
              tft.setCursor(10, 128);
              tft.print("Reverb");
              tft.fillRect(8, 156, 138, 22, ILI9341_BLACK);
              tft.setCursor(10, 171);
              tft.print("Rate");
              tft.fillRect(8, 199, 139, 22, ILI9341_BLACK);
              tft.setCursor(10, 214);
              tft.print("Phaser");

              digitalWrite(PB4, HIGH); //high ext, low int
              digitalWrite(PB7, HIGH); //eprom1 off
              digitalWrite(PB5, LOW); //eprom2 on
              digitalWrite(PB12, HIGH); //s0
              digitalWrite(PB13, HIGH); //s1
              digitalWrite(PB14, HIGH); //s2
              delay(50);
              break;



            case 17:
              tft.setFont(&TypoHoopRegular20pt7b);
              tft.setTextColor(ILI9341_WHITE);
              tft.setTextSize(1);
              tft.fillRect(0, 0, 240, 100, ILI9341_RED);
              tft.setCursor(45, 40);
              tft.print("Chorus");
              tft.setCursor(50, 85);
              tft.print("Reverb");

              tft.setFont(&TypoHoopRegular10pt7b);
              tft.setTextColor(ILI9341_WHITE);
              tft.fillRect(8, 113, 139, 22, ILI9341_BLACK);
              tft.setCursor(10, 128);
              tft.print("Reverb");
              tft.fillRect(8, 156, 138, 22, ILI9341_BLACK);
              tft.setCursor(10, 171);
              tft.print("Rate");
              tft.fillRect(8, 199, 139, 22, ILI9341_BLACK);
              tft.setCursor(10, 214);
              tft.print("Chorus");

              digitalWrite(PB4, LOW); //high ext, low int
              digitalWrite(PB7, HIGH); //eprom1 off
              digitalWrite(PB5, LOW); //eprom2 on
              digitalWrite(PB12, LOW); //s0
              digitalWrite(PB13, LOW); //s1
              digitalWrite(PB14, LOW); //s2
              delay(50);
              break;


            case 18:
              tft.setFont(&TypoHoopRegular20pt7b);
              tft.setTextColor(ILI9341_WHITE);
              tft.setTextSize(1);
              tft.fillRect(0, 0, 240, 100, ILI9341_RED);
              tft.setCursor(45, 40);
              tft.print("Flanger");
              tft.setCursor(50, 85);
              tft.print("Reverb");

              tft.setFont(&TypoHoopRegular10pt7b);
              tft.setTextColor(ILI9341_WHITE);
              tft.fillRect(8, 113, 139, 22, ILI9341_BLACK);
              tft.setCursor(10, 128);
              tft.print("Reverb");
              tft.fillRect(8, 156, 138, 22, ILI9341_BLACK);
              tft.setCursor(10, 171);
              tft.print("Rate");
              tft.fillRect(8, 199, 139, 22, ILI9341_BLACK);
              tft.setCursor(10, 214);
              tft.print("Flanger");

              digitalWrite(PB7, HIGH); //eprom1 off
              digitalWrite(PB5, LOW); //eprom2 on
              digitalWrite(PB12, LOW); //s0
              digitalWrite(PB13, LOW); //s1
              digitalWrite(PB14, HIGH); //s2
              delay(50);
              break;


            case 19:
              tft.setFont(&TypoHoopRegular20pt7b);
              tft.setTextColor(ILI9341_WHITE);
              tft.setTextSize(1);
              tft.fillRect(0, 0, 240, 100, ILI9341_RED);
              tft.setCursor(35, 40);
              tft.print("Tremolo");
              tft.setCursor(50, 85);
              tft.print("Reverb");

              tft.setFont(&TypoHoopRegular10pt7b);
              tft.setTextColor(ILI9341_WHITE);
              tft.fillRect(8, 113, 139, 22, ILI9341_BLACK);
              tft.setCursor(10, 128);
              tft.print("Reverb");
              tft.fillRect(8, 156, 138, 22, ILI9341_BLACK);
              tft.setCursor(10, 171);
              tft.print("Rate");
              tft.fillRect(8, 199, 139, 22, ILI9341_BLACK);
              tft.setCursor(10, 214);
              tft.print("Tremolo");

              digitalWrite(PB7, HIGH); //eprom1 off
              digitalWrite(PB5, LOW); //eprom2 on
              digitalWrite(PB12, LOW); //s0
              digitalWrite(PB13, HIGH); //s1
              digitalWrite(PB14, LOW); //s2
              delay(50);
              break;


            case 20:
              tft.setFont(&TypoHoopRegular20pt7b);
              tft.setTextColor(ILI9341_WHITE);
              tft.setTextSize(1);
              tft.fillRect(0, 0, 240, 100, ILI9341_RED);
              tft.setCursor(32, 40);
              tft.print("FV-1 Rom");
              tft.setCursor(32, 85);
              tft.print("Reverb 1");

              tft.setFont(&TypoHoopRegular10pt7b);
              tft.setTextColor(ILI9341_WHITE);
              tft.fillRect(8, 113, 139, 22, ILI9341_BLACK);
              tft.setCursor(10, 128);
              tft.print("Size");
              tft.fillRect(8, 156, 138, 22, ILI9341_BLACK);
              tft.setCursor(10, 171);
              tft.print("HF Filter");
              tft.fillRect(8, 199, 139, 22, ILI9341_BLACK);
              tft.setCursor(10, 214);
              tft.print("LF Filter");

              digitalWrite(PB7, HIGH); //eprom1 off
              digitalWrite(PB5, LOW); //eprom2 on
              digitalWrite(PB12, HIGH); //s0
              digitalWrite(PB13, HIGH); //s1
              digitalWrite(PB14, LOW); //s2
              delay(50);
              break;


            case 21:
              tft.setFont(&TypoHoopRegular20pt7b);
              tft.setTextColor(ILI9341_WHITE);
              tft.setTextSize(1);
              tft.fillRect(0, 0, 240, 100, ILI9341_RED);
              tft.setCursor(32, 40);
              tft.print("FV-1 Rom");
              tft.setCursor(32, 85);
              tft.print("Reverb 2");

              tft.setFont(&TypoHoopRegular10pt7b);
              tft.setTextColor(ILI9341_WHITE);
              tft.fillRect(8, 113, 139, 22, ILI9341_BLACK);
              tft.setCursor(10, 128);
              tft.print("Size");
              tft.fillRect(8, 156, 138, 22, ILI9341_BLACK);
              tft.setCursor(10, 171);
              tft.print("HF Filter");
              tft.fillRect(8, 199, 139, 22, ILI9341_BLACK);
              tft.setCursor(10, 214);
              tft.print("LF Filter");

              digitalWrite(PB4, LOW); //high ext, low int
              digitalWrite(PB7, HIGH); //eprom1 off
              digitalWrite(PB5, LOW); //eprom2 on
              digitalWrite(PB12, HIGH); //s0
              digitalWrite(PB13, HIGH); //s1
              digitalWrite(PB14, HIGH); //s2
              delay(50);
              break;

          }
        }
      }
    }

    // Meter Code -------------------------------------

    if ((p.x > 2880) && (p.x < 3840) && (p.y > 313) && (p.y < 3680)) // Screen type 1
      //if ((p.x < 1300) && (p.x > 257) && (p.y > 381) && (p.y < 3844)) // Screen type 2
    {
      analogMeter(); // Draw analogue meter
      meterOn = true;

    }
  }
}

void analogMeter()
{

  tft.setTextColor(ILI9341_BLACK);  // Text colour
  tft.fillRect(0, 0, 240, 100, meterCream);

  // draw meter background
  int h = 53, w = 240, row, col, buffidx = 0;
  for (row = 0; row < h; row++) { // For each scanline...
    for (col = 0; col < w; col++) { // For each pixel...
      //To read from Flash Memory, pgm_read_XXX is required.
      //Since image is stored as uint16_t, pgm_read_word is used as it uses 16bit address
      tft.drawPixel(col, row, pgm_read_word(vumeter + buffidx));
      buffidx++;
    } // end pixel
  }

}

void plotNeedle(int value, byte ms_delay)
{
  if (digitalRead (PB3) == LOW)
    tft.fillCircle(215, 80, 5, 0xc801);
  if (digitalRead (PB3) == HIGH)
    tft.fillCircle(215, 80, 5, meterCream);
  //tft.setTextColor(meterGrey, meterCream);
  //tft.setFont(&FreeMono12pt7b);
  //char buf[8]; dtostrf(value, 4, 0, buf);
  //tft.setCursor(5, 80);
  // tft.print(buf);

  if (value < -10) value = -10; // Limit value to emulate needle end stops
  if (value > 110) value = 110;

  // Move the needle util new value reached
  while (!(value == old_analog)) {
    if (old_analog < value) old_analog++;
    else old_analog--;

    if (ms_delay == 0) old_analog = value; // Update immediately if delay is 0

    float sdeg = map(old_analog, -10, 110, -150, -30); // Map value to angle
    // Calcualte tip of needle coords
    float sx = cos(sdeg * 0.0174532925);
    float sy = sin(sdeg * 0.0174532925);

    // Calculate x delta of needle start (does not start at pivot point)
    float tx = tan((sdeg + 90) * 0.0174532925);

    // Erase old needle image
    tft.drawLine(120 + 20 * ltx - 1, 119 - 20, osx - 1, osy, meterCream); //119
    tft.drawLine(120 + 20 * ltx, 119 - 20, osx, osy, meterCream);
    tft.drawLine(120 + 20 * ltx + 1, 119 - 20, osx + 1, osy, meterCream);

    // Re-plot text under needle
    tft.setTextColor(meterGrey);
    tft.setCursor(87, 83);
    tft.print("1337FX");

    // Store new needle end coords for next erase
    ltx = tx;
    osx = sx * 118 + 120; // 188 + 120
    osy = sy * 38 + 85; // 98 + 140 , 38 + 85

    // Draw the needle in the new postion, magenta makes needle a bit bolder
    // draws 3 lines to thicken needle
    tft.drawLine(120 + 20 * ltx - 1, 119 - 20, osx - 1, osy, 0xc801);
    tft.drawLine(120 + 20 * ltx, 119 - 20, osx, osy, 0xc801);
    tft.drawLine(120 + 20 * ltx + 1, 119 - 20, osx + 1, osy, 0xc801);

    // Slow needle down slightly as it approaches new postion
    // if (abs(old_analog - value) < 10) ms_delay += ms_delay / 5;

    // Wait before next update
    //  delay(ms_delay);
  }
}
