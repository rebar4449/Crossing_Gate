/***************************************************************************
  Dr.Duino Block Detection for Model Railroad Fans
  Written by: Guido Bonelli
  www.DrDuino.com

 This sketch demonstrations how to read the status of the IR sensor TCRT5000
 sensor. 

 When the sensor does detect that it is blocked, the signal state is changed from
 high to low, or low to high. 

 The Arduino then reads that state, and ouputs it to both the OLED and onboard
 LED's.

 You have the option of using the onboard Dr.Duino hardware to simulate the 
 sensors via SW1 & SW2 or using the block sensor hardware itself.

 In the event that the hardware is wired wrong, you'll be able to quickly isolate
 if the issue is with the hardware by simply moving the shunts around. 

 Note
 Arduino is a trademark of Arduino AG. Dr.Duino is not affiliated or endorsed by Arduino AG
 ***************************************************************************/
/*****************************************
  Built on the above base this is been modified
  to contorl a crossing signal on a double track
  Main line on the David R Bertz Railroad

  By Andrew Bertz Began Feb 14, 2021 Completed

  V1.1 - Starting by getting one track to work then will
  duplicate for the double track
  V1.2 - Added Double sensors to account for backing up 
  out of the block section without going all the way over
  the one sensor
 *****************************************/
 
//--------------Calling libraries needed to run-----------//
#include <DrDuino_Explorer.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_NeoPixel.h>

//------------------Object Instantiation-----------//
String     FwVersion        = "1.2.0";
DrDuino_Explorer Explorer;
Adafruit_NeoPixel strip(Explorer.NumOfLEDS, Explorer.AddressLED_Pin, NEO_GRB + NEO_KHZ800);

// General Variable Definitions 
int SensorT1E1 = LOW;         // variable for reading the pushbutton status
int SensorT1E2 = LOW;         // variable for reading the pushbutton status
int SensorT1W1 = LOW;         // variable for reading the pushbutton status
int SensorT1W2 = LOW;         // variable for reading the pushbutton status
int TrainInBlockT1 = LOW;       // variable for if there is a train in the block, LOW(UnBlocked) HIGH(BLOCKED)

//Variables for CrossingLED
unsigned long previousMillis = 0; //for CrossingLED Function, Global so it doesn't lose number
int ToggleFlashPattern = 0; //used to alternate the LEDs in the CrossingLED Function
int RRCrossingDelay   = 800; // 1000 = 1 second. 

//Set pins for Sensors
const int  SensorT1E1In   = 2;
const int  SensorT1E2In   = 3;
const int  SensorT1W1In   = 4;
const int  SensorT1W2In   = 5;

//Create a special variable for State of the Block
enum BLOCKSTATE
{
  ST_ENT_E1,
  ST_ENT_E2,
  ST_ENT_W1,
  ST_ENT_W2,
  ST_OCCUPIED,
  ST_EXT_E1,
  ST_EXT_E2,
  ST_EXT_W1,
  ST_EXT_W2,
  ST_EMPTY,
};
BLOCKSTATE CrossingStateT1; //This variable used for Track 1
BLOCKSTATE CrossingStateT2; //This variable used for Track 2

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
// and Setup variables
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Don't modify anything above this line, or you will be visited by the angry compiler gods :)

void setup() 
{
  Serial.begin(9600);
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) 
  { // Address 0x3C for 128x32
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  DrduinoSplashScreen(); // Display on the OLED
  delay(2000);

  //Start Assignable LEDs and turn them off
  strip.begin(); // Initialize NeoPixel strip object (REQUIRED)
  strip.show();  // Initialize all pixels to 'off'
  strip.setBrightness(42); // Set BRIGHTNESS to about 1/6 (max = 255)
  
  pinMode(SensorT1E1In,   INPUT_PULLUP);
  pinMode(SensorT1E2In,   INPUT_PULLUP);
  pinMode(SensorT1W1In,   INPUT_PULLUP);
  pinMode(SensorT1W2In,   INPUT_PULLUP);

  CrossingStateT1 = ST_EMPTY; //Set CrossingState to Empty
}

void loop() 
{
   //Below Should commented out once we are using sensor
  SensorT1E1 = digitalRead(Explorer.SW1);
  SensorT1E2 = digitalRead(Explorer.SW2);
  SensorT1W2 = digitalRead(Explorer.SW3);
  SensorT1W1 = digitalRead(Explorer.SW4);

  //SensorT1E1 = digitalRead(SensorE1In);
  //SensorT1E2 = digitalRead(SensorE2In);
  //SensorT1W1 = digitalRead(SensorW1In);
  //SensorT1W2 = digitalRead(SensorW2In);


  switch (CrossingStateT1) //Switch used as a Multiple If Stmt set
  {
    case ST_EMPTY:
      CrossingEmpty(SensorT1E1,SensorT1E2,SensorT1W1,SensorT1W2);
      break;
    case ST_ENT_E1:
      CrossingENT_E1(SensorT1E1,SensorT1E2,SensorT1W1,SensorT1W2);
      break;
    case ST_ENT_E2:
      CrossingENT_E2(SensorT1E1,SensorT1E2,SensorT1W1,SensorT1W2);
      break;
    case ST_ENT_W1:
      CrossingENT_W1(SensorT1E1,SensorT1E2,SensorT1W1,SensorT1W2);
      break;
    case ST_ENT_W2:
      CrossingENT_W2(SensorT1E1,SensorT1E2,SensorT1W1,SensorT1W2);
      break;
    case ST_OCCUPIED:
      CrossingOccupied(SensorT1E1,SensorT1E2,SensorT1W1,SensorT1W2);
      break;
    case ST_EXT_E1:
      CrossingExt_E1(SensorT1E1,SensorT1E2,SensorT1W1,SensorT1W2);
      break;
    case ST_EXT_E2:
      CrossingExt_E2(SensorT1E1,SensorT1E2,SensorT1W1,SensorT1W2);
      break;
    case ST_EXT_W1:
      CrossingExt_W1(SensorT1E1,SensorT1E2,SensorT1W1,SensorT1W2);
      break;
    case ST_EXT_W2:
      CrossingExt_W2(SensorT1E1,SensorT1E2,SensorT1W1,SensorT1W2);
      break;
  }
  
  // Tell the user the status of the sensors on the OLED
  BlockingSensorT1Status(SensorT1E1,SensorT1E2,SensorT1W1,SensorT1W2,CrossingStateT1);
}

/***************************************************************************
*---------------State Functions-----------------------
*  Funtions to be performed at various crossing states
*  
*                              USAGE
*  Crossing______(SensorE1,SensorE2,SensorW1,SensorW2);
***************************************************************************/
void CrossingEmpty(int SensorT1E1, int SensorE2, int SensorW1, int SensorW2)
{
  //Do this:
  CrossingLED(LOW); //Turn off Crossing Lights
  //Then Watch for Sensor changes
  if(SensorE1 == LOW) //When front of train hits SensorE1
  {
    // change state to ENT_E1
    CrossingStateT1 = ST_ENT_E1;
  }
  else if(SensorW1 == LOW) //When front of train hits SensorW1
  {
    // change state to ENT_W1
    CrossingStateT1 = ST_ENT_W1;
  }
}

void CrossingENT_E1(int SensorE1, int SensorE2, int SensorW1, int SensorW2)
{
  //Do this:
  CrossingLED(HIGH);
  //Then Watch for Sensor changes
  if(SensorE1 == LOW && SensorE2 == LOW) //When train Covers both SensorE1 and SensorE2
  {
    // change state to Ent_E2
    CrossingStateT1 = ST_ENT_E2;
  }
  else if(SensorE1 == HIGH && SensorE2 == HIGH) //If train backs up and clears SensorE1 
  {
    // change state to Empty
    CrossingStateT1 = ST_EMPTY;
  }
}

void CrossingENT_E2(int SensorE1, int SensorE2, int SensorW1, int SensorW2)
{
  //Do this:
  CrossingLED(HIGH);
  //Then Watch for Sensor changes
  if(SensorE1 == HIGH && SensorE2 == LOW) //When end of train clears SensorE1 and Still on SensorE2
  {
    // change state to Occupied
    CrossingStateT1 = ST_OCCUPIED;
  }
  else if(SensorE1 == LOW && SensorE2 == HIGH) //If train backs up and clears SensorE2 but still on SensorE1
  {
    // change state to EXT_E1
    CrossingStateT1 = ST_EXT_E1;
  }
}

void CrossingENT_W1(int SensorE1, int SensorE2, int SensorW1, int SensorW2)
{
  //Do this:
  CrossingLED(HIGH);
  //Then Watch for Sensor changes
  if(SensorW1 == LOW && SensorW2 == LOW) //When Train covers SensorW1 and SensorW2
  {
    // change state to Ent_W2
    CrossingStateT1 = ST_ENT_W2;
  }
  else if(SensorW1 == HIGH && SensorW2 == HIGH) //If trains backs up and clears SensorW1
  {
    // change state to Empty
    CrossingStateT1 = ST_EMPTY;
  }
}

void CrossingENT_W2(int SensorE1, int SensorE2, int SensorW1, int SensorW2)
{
  //Do this:
  CrossingLED(HIGH);
  //Then Watch for Sensor changes
  if(SensorW1 == HIGH && SensorW2 == LOW) //When end of train clears SensorW1 and Still on SensorW2
  {
    // change state to Occupied
    CrossingStateT1 = ST_OCCUPIED;
  }
  else if(SensorW1 == LOW && SensorW2 == HIGH) //if train backs up and clears SensorE2 but still on SensorW1
  {
    // change state to EXT_W1
    CrossingStateT1 = ST_EXT_W1;
  }
}

void CrossingOccupied(int SensorE1, int SensorE2, int SensorW1, int SensorW2)
{
  //Do This
  CrossingLED(HIGH); //to keep the lights flashing
  //Watch for Sensor changes
  if(SensorE2 == LOW) //When front of train hits sensorE2
  {
    // change state to EXT_E2
    CrossingStateT1 = ST_EXT_E2;
  }
  else if(SensorW2 == LOW) //When fornt of train hits second sensorW2
  {
    // change state to EXT_W2
    CrossingStateT1 = ST_EXT_W2;
  }
}

void CrossingExt_E1(int SensorE1, int SensorE2, int SensorW1, int SensorW2)
{
  //Do This
  CrossingLED(HIGH); //to keep the lights flashing
  //Watch for Sensor changes
  if(SensorE1 == HIGH && SensorE2 == HIGH) //When end of train clears SensorE1
  {
    // change state to Empty
    CrossingStateT1 = ST_EMPTY;
  }
  else if(SensorE1 == HIGH && SensorE2 == LOW) // to account for a direction change
  {
    //Change State to Ext_E2
    CrossingStateT1 = ST_EXT_E2;
  }
}

void CrossingExt_E2(int SensorE1, int SensorE2, int SensorW1, int SensorW2)
{
  //Do This
  CrossingLED(HIGH); //to keep the lights flashing
  //Watch for Sensor changes
  if(SensorE1 == LOW && SensorE2 == LOW) //When Train covers both Sensors Leaving the Block
  {
    // change state to Ext_E1
    CrossingStateT1 = ST_EXT_E1;
  }
  else if(SensorE1 == HIGH && SensorE2 == HIGH) //to account for direction change
    // Change state to Occupied
    CrossingStateT1 = ST_OCCUPIED;
}

void CrossingExt_W1(int SensorE1, int SensorE2, int SensorW1, int SensorW2)
{
  //Do This
  CrossingLED(HIGH); //to keep the lights flashing
  //Watch for Sensor changes
  if(SensorW1 == HIGH && SensorW2 == HIGH) //When end of train clears SensorW1
  {
    // change state to Empty
    CrossingStateT1 = ST_EMPTY;
  }
  else if(SensorW1 == HIGH && SensorW2 ==LOW) //to account for a direction change
  {
    // Change state to EXT_W2
    CrossingStateT1 = ST_EXT_W2;
  }
}

void CrossingExt_W2(int SensorE1, int SensorE2, int SensorW1, int SensorW2)
{
  //Do This
  CrossingLED(HIGH); //to keep the lights flashing
  //Watch for Sensor changes
  if(SensorW1 == LOW && SensorW2 == LOW) //When end of train clears SensorE2
  {
    // change state to EXT_W1
    CrossingStateT1 = ST_EXT_W1;
  }
  else if(SensorW1 == HIGH && SensorW2 == HIGH) //to account for direction change
  {
    // Change state to Occupied
    CrossingStateT1 = ST_OCCUPIED;
    }
}

/***************************************************************************
*---------------Crossing Flash Function-----------------------
*  Flash the LEDs for the Crossing Gate
*  
*                              USAGE
*  CrossingLED(State);// State LOW = Off, HIGH = On
***************************************************************************/

void CrossingLED(int State)
{
    unsigned long currentMillis = millis(); //Sets current Millis

    if(State == HIGH)
    {
      if(currentMillis - previousMillis >= RRCrossingDelay)
      {
        if(ToggleFlashPattern == 1) //Code from dr.Duino RR Lighting example
          {    
            strip.setPixelColor(0, strip.Color(255,0,0));
            strip.setPixelColor(1, strip.Color(255,0,0));
            strip.setPixelColor(2, strip.Color(255,0,0));
            strip.setPixelColor(3, strip.Color(255,0,0));
            strip.setPixelColor(4, strip.Color(0,0,0));
            strip.setPixelColor(5, strip.Color(0,0,0));
            strip.setPixelColor(6, strip.Color(0,0,0));
            strip.setPixelColor(7, strip.Color(0,0,0));
          }
          else
          {
            strip.setPixelColor(0, strip.Color(0,0,0));
            strip.setPixelColor(1, strip.Color(0,0,0));
            strip.setPixelColor(2, strip.Color(0,0,0));
            strip.setPixelColor(3, strip.Color(0,0,0));
            strip.setPixelColor(4, strip.Color(255,0,0));
            strip.setPixelColor(5, strip.Color(255,0,0));
            strip.setPixelColor(6, strip.Color(255,0,0));
            strip.setPixelColor(7, strip.Color(255,0,0));
          }
        ToggleFlashPattern = !ToggleFlashPattern; 
        strip.show(); // Show the lights
        previousMillis = currentMillis;
      }
    }
    else
    {
      strip.clear(); //Fill the NeoPixel Strip with off/black
      strip.show(); //Show All Black from .Clear
    }
}


/***************************************************************************
*---------------BlockingSensorStatus Function-----------------------
*  Tell the user using the OLED what the current status of the blocking 
*  sensors are.
*                              USAGE
*  BlockingSensorStatus(SensorE1, SensorE2);// Nothing special
***************************************************************************/
void BlockingSensorStatus(int SensorE1Status, int SensorE2Status, int SensorW1Status, int SensorW2Status, BLOCKSTATE CrossingSt)
{
  display.clearDisplay();
  display.setTextSize(1);      // Normal 1:1 pixel scale
  display.setTextColor(WHITE); // Draw white text
  display.setCursor(0, 0);     // Start at top-left cornerer
  display.cp437(true);         // Use full 256 char 'Code Page 437' font
  
  display.print(("S1:"));
  display.setTextSize(1);      
 
  if (SensorE1Status == 1)
  {
    display.print("C, ");    
  }
  else
  {
    display.print("B, "); 
  }
  display.print(("S2:"));
  if (SensorE2Status == 1)
  {
    display.print("C, ");    
  }
  else
  {
    display.print("B, "); 
  }
  display.print(("S3:"));
  if (SensorW1Status == 1)
  {
    display.print("C, ");    
  }
  else
  {
    display.print("B, "); 
  }
  display.print(("S4:"));
  if (SensorW2Status == 1)
  {
    display.println("C, ");    
  }
  else
  {
    display.println("B, "); 
  }
  display.print(("State:"));
    switch (CrossingSt) //Switch used as a Multiple If Stmt set
    {
      case ST_EMPTY:
        display.println("Empty");
        break;
      case ST_ENT_E1:
        display.println("Entering E1");
        break;
      case ST_ENT_E2:
        display.println("Entering E2");
        break;
      case ST_ENT_W1:
        display.println("Entering W1");
        break;
      case ST_ENT_W2:
        display.println("Entering W2");
        break;
      case ST_OCCUPIED:
        display.println("Occupied");
        break;
      case ST_EXT_E1:
        display.println("Exiting E1");
        break;
      case ST_EXT_E2:
        display.println("Exiting E2");
        break;
      case ST_EXT_W1:
        display.println("Exiting W1");
        break;
      case ST_EXT_W2:
        display.println("Exiting W2");
        break;
    }
    
  display.display();      
  delay(100);  //?? not sure what this is for and need to consider removing
}
/***************************************************************************
*---------------DrduinoSplashScreen Function-----------------------
*  Displays Dr.Duino Splash screen. WHich also contains the firmware verison. 
*                              USAGE
*  DrduinoSplashScreen();// Nothing special
***************************************************************************/
void DrduinoSplashScreen()
{
  display.clearDisplay();
  display.setTextSize(2);      // Normal 1:1 pixel scale
  display.setTextColor(WHITE); // Draw white text
  display.cp437(true);         // Use full 256 char 'Code Page 437' font
  display.println(("DR.DUINO"));
  display.setTextSize(1);      
  display.println(("RR Crossing"));
  display.print(( "Version "));
  display.println(( FwVersion));
  display.display();      
  delay(100);
}
