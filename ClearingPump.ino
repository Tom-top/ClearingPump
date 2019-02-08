////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////*Author_Thomas_TOPILKO_&&_Sophie_SKRIABINE*///////////////////////////////////////////
//////////////////////////////////////////////////////*Stable_Version*//////////////////////////////////////////////////////////          
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*Importing Libraries*/

#include <Adafruit_GFX.h> 
#include <Adafruit_PCD8544.h>
#include <ClickEncoder.h>
#include <TimerOne.h>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*Setting Pins*/

/*Motor Pins*/
const int motorStepPin = 6; /*Setting pin number for stepping : A4988 Pololu*/
const int motorDirPin = 7; /*Setting pin number for direction : A4988 Pololu*/

/*Nokia 5110 Pins*/
const int SCLKPin = 13;  /*Setting pin number for Serial clock out (SCLK) : 5110 Nokia*/
const int DNKPin = 11;  /*Setting pin number for Serial data out (DNK) : 5110 Nokia*/
const int DCPin = 10;  /*Setting pin number for Data/Command select (D/C) : 5110 Nokia*/
const int SCEPin = 12;  /*Setting pin number for Chip Enable (SCE) : 5110 Nokia*/
const int RSTPin = 8;  /*Setting pin number for Reset (RST) : 5110 Nokia*/
const int lightPin = 9;  /*Setting pin number for backlight : 5110 Nokia*/

/*Rotary encoder Pins*/
const int outAPin = A0; /*Setting pin number for OutA Pin : Rotary Encoder*/
const int outBPin = A1; /*Setting pin number for OutB Pin : Rotary Encoder*/
const int switchPin = A2; /*Setting pin number for Switch Pin : Rotary Encoder*/

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*Setting menu variables*/

/*Menu Variables*/
int menuItem = 1; /*Setting variable for selected items in the menu*/
int lastMenuItem = 1; /*Setting variable to check for previous item selected*/
int frame = 1; /*Setting variable for the current frame displayed*/
int page = 1; /*Setting variable for displayed page*/
int Yes_No = 1; /*Setting variable selection of Yes/No options in submenus*/

const String menuItem1 = "Set Vol/Speed"; /*Setting name for first menuitem*/
const String menuItem2 = "Fill"; /*Setting name for third menuitem*/
const String menuItem3 = "Calibrate"; /*Setting name for fourth menuitem*/
const String menuItem4 = "Backlight"; /*Setting name for fifth menuitem*/
const String menuItem5 = "Reset"; /*Setting name for sixth menuitem*/
String Yes = "YES"; /*Setting name for Yes submenu*/
String No = "NO"; /*Setting name for No submenu*/

boolean up = false; /*Setting boolean to check counter-clockwise turning of rotary encoder*/
boolean down = false; /*Setting boolean to check clockwise turning of rotary encoder*/
boolean middle = false; /*Setting boolean to check pressing of rotary encoder*/
boolean middlePressed = true; /*Setting boolean to launch functions if middle button is pressed*/

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*Setting internal variables*/

/*Motor*/
int counter = 0; /*Setting variable for step counts*/
boolean motorDirection = true; /*Setting boolean for Stepper motor direction*/
boolean motorRunning = false; /*Setting boolean to check Stepper motor status*/
boolean motorState = false; /*Setting boolean to check Stepper motor state HIGH/LOW*/
int calibrationPercent = 0; /*Setting variable for calibration percentage*/
int calibrationLinePercent = 0;
int calibrationItem = 0; /*Setting variable for calibration item selection*/
float calibrationValue = 0.; /*Setting variable for calibration final volume*/
int cntFill = 10000;
int cntCalibration = 30;
int cntSet = 0;
int millisFill = 10; /*Setting variable for the intervals between steps (ms)*/
int millisCalibration = 10; /*Setting variable for the intervals between steps (ms)*/
int millisSet = 0;

/*LCD Backlight*/
boolean LCDBackLight = true; /*Setting boolean for Nokia 5110 backlight On/Off*/
int LCDContrast = 55; /*Setting variable for Nokia 5110 contrast*/
int LCDLineContrastPercent = 0;
int lightItem = 0; /*Setting variable for light item selection*/
boolean LCDContrastSelection = false; /*Setting boolean for light contrast selection*/
Adafruit_PCD8544 display = Adafruit_PCD8544(SCLKPin, DNKPin, DCPin, SCEPin, RSTPin); /*Initializing Nokia 5110*/

/*Pumping variables*/
int volumeVar = 0; /*Setting variable for the volume to be pumped (mL)*/
float speedVar = 0.; /*Setting variable for the speed to be used (mL/s)*/
int valueItem = 0; /*Setting variable for the item to be selected vol/speed*/

/*Rotary encoder variables*/
ClickEncoder *encoder; /*Initializing rotatry encoder*/
int16_t encoderLast, encoderValue; /*Setting variables for last/current position of the Rotary encoder*/

/*Timing variables*/
unsigned long curMillis; /*Setting variable for the current running time of the Arduino (ms)*/
unsigned long prevStepMillis = 0; /*Setting variable for the previous step that occured (ms)*/

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*Setup*/

void setup() {
  
  pinMode(motorStepPin,OUTPUT); /*Set motor pin mode for steps*/
  pinMode(motorDirPin,OUTPUT); /*Set motor pin mode for direction*/
  pinMode(lightPin,OUTPUT); /*Set light pin mode for LCD backlight*/
  TurnBackLightOn(); /*Runing the BackLight function to turn the light on*/

  digitalWrite(motorDirPin,LOW); /*Set motor direction to clockwise*/
  
  encoder = new ClickEncoder(outBPin, outAPin, switchPin); /*Setting the encoder pins*/
  encoder->setAccelerationEnabled(true); /*Function that allows acceleration of stepper indentation (true or false)*/

  display.begin(); /*Setting LCD*/
  display.clearDisplay(); /*Clear the LCD screen*/
  display.setContrast(LCDContrast); /*Run Contrast function to set contrast of the screen*/

  Timer1.initialize(1000);
  Timer1.attachInterrupt(timerIsr); 
  
  encoderLast = encoder->getValue();   
}

void loop() {
  
  curMillis = millis();

  ClickEncoder::Button b = encoder->getButton(); /*Checks if Encoder central button is pressed*/
  
  if (b != ClickEncoder::Open) 
  {
    switch (b) 
    {
      case ClickEncoder::Clicked:
      middle=true;
      break;
    }
  }

  if (motorRunning == false)
  {
    ReadRotaryEncoder();
    DrawMenu();
    Up();
    Down();
    Middle();
  }

  if (motorRunning == true)
  {
    Middle();

//    if (menuItem == 1)
//    {
//       RunningPump(millisSet,cntSet);
//    }
    
    if (menuItem == 2)
    {
     RunningPump(millisFill,cntFill);
    }

    if (menuItem == 3)
    {
       RunningPump(millisCalibration,cntCalibration);
       calibrationPercent = (counter/cntCalibration)*100;
       DrawMenu();
    }
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*Functions*/

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*Functions for Rotarty encoder*/

void timerIsr() 
{
  encoder->service();
}

void ReadRotaryEncoder()
{
  encoderValue += encoder->getValue();
  if (encoderValue/2 > encoderLast) 
  {
    encoderLast = encoderValue/2;
    down = true;
    delay(150);
  }
  else if (encoderValue/2 < encoderLast) 
  {
    encoderLast = encoderValue/2;
    up = true;
    delay(150);
  }
}

void Up()
{
  // If the Rotary Encoder is turned Counter-Clockwise
  if (up)
  {

  up = false;

    // If the user was on page 1
    if (page == 1)
    {
      
      // If the user was on the second frame, second item, change frame to previous
      if(menuItem == 2 && frame == 2)
      {
          frame--;
      }
      
      // If the user was on the third frame, third item, change frame to previous
      else if(menuItem == 3 && frame == 3)
      {
        frame--;
      }
      
      lastMenuItem = menuItem; 
      menuItem--; 

      // If the user was on the first menu item and goes up, stays in the same position
      if(menuItem == 0)
      {
        menuItem++;
      }
    }

    // If the user was on page 2
    else if (page == 2)
    {

      // If the user was on the first menu item (vol/speed)
      if (menuItem == 1)
      {

        // If the user selected the volume value, decreases it
        if (volumeVar > 0 && valueItem == 0)
        {
          volumeVar--;
        }

        // If the user selected the speed value, decreases it
        else if (speedVar > 0 && valueItem == 1)
        {
          speedVar-=0.01;
        }
      }

      // If the user was on the fourth menu item (light)
      else if (menuItem == 4)
      {
        
        // If the user turns the rotary encoder counter-clockwise change lightItem
        if (lightItem > 0 && LCDContrastSelection == false)
        {
          lightItem--;
        }

        // If the user selected the contrast modification submenu changes the contrast of the screen
        else if (lightItem == 1 && LCDContrastSelection == true)
        {
          LCDContrast--;
          display.setContrast(LCDContrast);
        }
      }
    }

    // If the user was on page 3
    else if (page == 3)
    {
      
      // If the user selected menu item 1 and is highlighting NO goes to YES
      if (menuItem == 1 && Yes_No == 2)
      {
        Yes_No--;
      }

      // If the user selected menu item 3
      else if (menuItem == 3 && calibrationItem == 0 && calibrationValue > 0.9)
      {
        calibrationValue--;
      }
      
      else if (menuItem == 3 && calibrationItem == 1 && calibrationValue > 0.09)
      {
        calibrationValue-=0.1;
      }
      
      else if (menuItem == 3 && calibrationItem == 2 && calibrationValue > 0.009)
      {
        calibrationValue-=0.01;
      }
    }  
  }
}

////////////////////////////////////////////////////////////////
/* Setting the function that executes when rotary encoder is turned clockwise */

void Down()
{
  if (down) // The Rotary Encoder is turned Clockwise
  {

  down = false;

    // If the user was on page 1
    if (page == 1)
    {

      // If the user was on the first frame, third item, change frame to next
      if(menuItem == 3 && frame == 1)
      {
          frame++;
      }

      // If the user was on the second frame, fourth item, change frame to next
      else if(menuItem == 4 && frame == 2)
      {
        frame++;
      }
      
      lastMenuItem = menuItem;
      menuItem++; 

      // If the user was on the last menu item and goes down, stays in the same position
      if(menuItem == 6)
      {
        menuItem--;
      }
    }

    // If the user was on page 2
    else if (page == 2)
    {

      // If the user was on the first menu item (vol/speed)
      if (menuItem==1 && volumeVar<100)
      {

        // If the user selected the volume value, increases it
        if (volumeVar < 100 && valueItem == 0)
        {
          volumeVar++;
        }

        // If the user selected the speed value, increases it
        else if (speedVar < 1.4 && valueItem == 1)
        {
          speedVar+=0.01;
        }
      }

      // If the user was on the fourth menu item (light)
      else if (menuItem == 4)
      {
        
        // If the user turns the rotary encoder clockwise change lightItem
        if (lightItem < 2 && LCDContrastSelection == false)
        {
          lightItem++;
        }

        // If the user selected the contrast modification submenu changes the contrast of the screen
        else if (lightItem == 1 && LCDContrastSelection == true)
        {
          LCDContrast++;
          display.setContrast(LCDContrast);
        }
      }
    }

    // If the user was on page 3
    else if (page == 3)
    {

      // If the user selected menu item 1 and is highlighting YES goes to NO
      if (menuItem == 1 && Yes_No==1)
      {
        Yes_No++;
      }

      // If the user selected menu item 3
      else if (menuItem == 3 && calibrationItem == 0)
      {
        calibrationValue+=1;
      }

      
      else if (menuItem == 3 && calibrationItem == 1)
      {
        calibrationValue+=0.1;
      }

      
      else if (menuItem == 3 && calibrationItem == 2)
      {
        calibrationValue+=0.01;
      }
      
    }
  }
}

////////////////////////////////////////////////////////////////
/* Setting the function that executes when rotary encoder is pressed */

void Middle()
{
  // The Rotary Encoder is Pressed
  if (middle)
  {
    
  middle = false;

    // If on the first page
    if (page == 1)
    {
      // If first menu item was selected
      if (menuItem == 1)
      {
        page = 2;
        valueItem = 0;
      }

      // If second menu item was selected
      else if (menuItem == 2)
      {
        ResetMotorVars();
      }

      // If third menu item was selected
      else if(menuItem == 3)
      {
        page = 2;
        ResetMotorVars();
      }
      
      // If fourth menu item was selected
      else if (menuItem == 4)
      {
        page = 2;
        lightItem = 0;
      }

      // If fith menu item was selected
      else if (menuItem == 5)
      {
        ResetDefaults();
      }
    }

    // If on the second page
    else if (page == 2)
    {
      // If first menu item was selected
      if (menuItem == 1)
      {
        if (valueItem < 2)
        {
          valueItem+=1;
        }
        else if (valueItem == 2)
        {
          page = 3;
        }
      }

      // If third menu item was selected
      else if (menuItem == 3)
      {
        if (not motorRunning) 
        {
          page = 3;
        }
      }

      // If forth menu item was selected
      else if (menuItem == 4)
      {
        
        if (lightItem == 0)
        {
          SwitchState(LCDBackLight);
        }

        else if (lightItem == 1)
        {
          SwitchState(LCDContrastSelection);
        }

        else if (lightItem == 2)
        {
          page = 1;
        }
      }
    }

    else if (page == 3)
    {
      if (menuItem == 1)
      {
        
        if (Yes_No == 1)
        {
          ResetMotorVars();
        }
        
        else if (Yes_No == 2)
        {
          page = 1;
        }
      }

      else if (menuItem == 3)
      {
        if (calibrationItem < 2)
        {
         calibrationItem += 1;
        }
        else if (calibrationItem == 2)
        {
          page = 1;
        }
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*Functions for the motor*/

void SwitchState(boolean &bol)
{
  if (bol == true) 
  {
    bol = false;
  }
  else if (bol == false) 
  {
    bol = true;
  }
  return bol;
}

void ResetMotorVars ()
{
  prevStepMillis = 0;
  SwitchState(motorRunning);
  motorState = false;
  counter=0;
}

void RunningPump(int freq,int cnt) 
{
  if (counter < cnt)
  {
    if (curMillis - prevStepMillis >= freq)
    {
      prevStepMillis = curMillis;
  
      if (motorState == false)
      {
        digitalWrite(motorStepPin, HIGH);
        SwitchState(motorState);
      }

      else if (motorState == true)
      {
        digitalWrite(motorStepPin, LOW);
        SwitchState(motorState);
        counter+=1;
      }
    }
  }

  else 
  {
    prevStepMillis = 0;
    SwitchState(motorRunning);
    motorState = false;
    counter=0;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*Functions for backLight control*/

/*Turn the backLight on*/
void TurnBackLightOn ()
{
  LCDBackLight = true;
  digitalWrite(lightPin,HIGH);
};

/*Turn the backLight off*/
void TurnBackLightOff ()
{
  LCDBackLight = false;
  digitalWrite(lightPin,LOW);
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*Functions for the menu*/

/*Draw the menu*/
void DrawMenu()
{
  /*****************************************/
  //If on the first page
  if (page==1) 
  {    
    display.setTextSize(1);
    display.clearDisplay();
    display.setTextColor(BLACK, WHITE);
    display.setCursor(7, 0);
    display.print("TOPPUMP 1.0");
    display.drawFastHLine(0,10,83,BLACK);

    //If on the first frame
    if(menuItem==1 && frame ==1)
    {   
      DisplayMenuItem(menuItem1, 15,true);
      DisplayMenuItem(menuItem2, 25,false);
      DisplayMenuItem(menuItem3, 35,false);
    }
    else if(menuItem == 2 && frame == 1)
    {
      DisplayMenuItem(menuItem1, 15,false);
      DisplayMenuItem(menuItem2, 25,true);
      DisplayMenuItem(menuItem3, 35,false);
    }
    else if(menuItem == 3 && frame == 1)
    {
      DisplayMenuItem(menuItem1, 15,false);
      DisplayMenuItem(menuItem2, 25,false);
      DisplayMenuItem(menuItem3, 35,true);
    }

    //If on the second frame
    else if(menuItem == 2 && frame == 2)
    {
      DisplayMenuItem(menuItem2, 15,true);
      DisplayMenuItem(menuItem3, 25,false);
      DisplayMenuItem(menuItem4, 35,false);
    }
    else if(menuItem == 3 && frame == 2)
    {
      DisplayMenuItem(menuItem2, 15,false);
      DisplayMenuItem(menuItem3, 25,true);
      DisplayMenuItem(menuItem4, 35,false);
    }
    else if(menuItem == 4 && frame == 2)
    {
      DisplayMenuItem(menuItem2, 15,false);
      DisplayMenuItem(menuItem3, 25,false);
      DisplayMenuItem(menuItem4, 35,true);
    }

    //If on the third frame
    else if(menuItem == 3 && frame == 3)
    {
      DisplayMenuItem(menuItem3, 15,true);
      DisplayMenuItem(menuItem4, 25,false);
      DisplayMenuItem(menuItem5, 35,false);
    }
    else if(menuItem == 4 && frame == 3)
    {
      DisplayMenuItem(menuItem3, 15,false);
      DisplayMenuItem(menuItem4, 25,true);
      DisplayMenuItem(menuItem5, 35,false);
    }
    else if(menuItem == 5 && frame == 3)
    {
      DisplayMenuItem(menuItem3, 15,false);
      DisplayMenuItem(menuItem4, 25,false);
      DisplayMenuItem(menuItem5, 35,true);
    }
  }
  
  /*****************************************/
  //If on the second page
  else if (page == 2)
  {
    //If first item was selected
    if (menuItem == 1)
    {
      DisplayVolSpeedMenu(menuItem1, volumeVar, speedVar);
    }
    
    else if (menuItem == 3)
    {
      DisplayCalibrationMenu(menuItem3);
    }

    else if (menuItem == 4)
    {
      DisplayBackLightMenu(menuItem4,LCDContrast);
    }
  }

  //If on the third page
  else if (page == 3)
  {
    if (menuItem == 1)
    {
      display.setTextSize(1);
      display.clearDisplay();
      display.setTextColor(BLACK, WHITE);
      display.setCursor(4, 0);
      display.print("Are you ready");
      display.setCursor(7, 10);
      display.print("to proceed ?");
      display.drawFastHLine(0,20,83,BLACK);
      display.setTextSize(2);
      display.setCursor(45, 30);
      display.print("/"); 
      if (menuItem == 1 && Yes_No == 1)
      { 
        DisplayYesNoMenu(Yes,No, 30,true,false);
      }
      else if (menuItem == 1 && Yes_No == 2)
      { 
        DisplayYesNoMenu(Yes,No, 30,false,true);
      }
    }

    else if (menuItem == 3)
    {
      DisplayInputCalibrationMenu(menuItem3);
    }
  }
  
  display.display();
}

/*Draw a menu item*/
void DisplayMenuItem(String item, int position, boolean selected)
{
  if(selected)
  {
    display.setTextColor(WHITE, BLACK);
  }
  else
  {
    display.setTextColor(BLACK, WHITE);
  }
  display.setCursor(0, position);
  display.print(">"+item);
}

/*Draw the volume speed selection menu*/
void DisplayVolSpeedMenu(String menuItem, int volumeVar, float speedVar)
{
  display.setTextSize(1);
  display.clearDisplay();
  display.setTextColor(BLACK, WHITE);
  display.setCursor(3, 0);
  display.print(menuItem);
  display.drawFastHLine(0,10,83,BLACK);

  display.setCursor(1, 15);
  display.print("Vol:");
  display.setCursor(1, 25);
  display.print(volumeVar);
  display.setCursor(14, 25);
  display.print("mL");

  display.setCursor(34, 15);
  display.print("Speed:");
  display.setCursor(34, 25);
  display.print(speedVar);
  display.setCursor(62, 25);
  display.print("mL/s");

  display.display();
}

/*Draw the YES/NO selection menu*/
void DisplayYesNoMenu(String item1,String item2, int position, boolean selected1, boolean selected2)
{
  if (selected1)
  {
    display.setTextColor(WHITE, BLACK);
    display.setTextSize(2);
    display.setCursor(5, position);
    display.print(item1);
    display.setTextColor(BLACK, WHITE);
    display.setCursor(50, position);
    display.print(item2);
  }
  else if (selected2)
  {
    display.setTextColor(BLACK, WHITE);
    display.setTextSize(2);
    display.setCursor(5, position);
    display.print(item1);
    display.setTextColor(WHITE, BLACK);
    display.setCursor(50, position);
    display.print(item2);
  }
}

/*Draw Calibration menu*/
void DisplayCalibrationMenu(String menuItem)
{
  display.setTextSize(1);
  display.clearDisplay();
  display.setTextColor(BLACK, WHITE);
  display.setCursor(15, 0);
  display.print(menuItem);
  display.drawFastHLine(0,10,83,BLACK);

  display.setCursor(5, 15);
  display.print("Calib : ");
  display.setCursor(50, 15);
  display.print(calibrationPercent);
  display.setCursor(70, 15);
  display.print("%");

  calibrationLinePercent = (calibrationPercent/100) * 83;

  display.drawFastHLine(0,20,calibrationLinePercent,BLACK);
  display.drawFastHLine(0,21,calibrationLinePercent,BLACK);
  display.drawFastHLine(0,22,calibrationLinePercent,BLACK);
}

/*Draw Input Calibration menu*/
void DisplayInputCalibrationMenu(String menuItem)
{
  display.setTextSize(1);
  display.clearDisplay();
  display.setTextColor(BLACK, WHITE);
  display.setCursor(5, 0);
  display.print("Input volume pumped :");
  display.setCursor(5, 30);
  display.print(calibrationValue);
  display.setCursor(20, 30);
  display.print("mL"); 
}

/*Draw the backlight menu*/
void DisplayBackLightMenu(String menuItem, int LCDContrast)
{
  display.setTextSize(1);
  display.clearDisplay();
  display.setTextColor(BLACK, WHITE);
  display.setCursor(3, 0);
  display.print(menuItem);
  display.drawFastHLine(0,10,83,BLACK);

  display.setCursor(1, 15);
  display.print("Light:");
  
  if (LCDBackLight == true)
  {
    display.setCursor(20, 30);
    display.print("ON");
  }

  else if (LCDBackLight == false)
  {
    display.setCursor(20, 30);
    display.print("OFF");
  }

  display.setCursor(40, 15);
  display.print("Contrast:");
  LCDLineContrastPercent = (LCDContrast/255)*13;
  display.drawFastHLine(40,30,LCDLineContrastPercent,BLACK);

  display.display();
}

/*Reset most of the internal variables to default*/
void ResetDefaults()
{
  LCDContrast = 55;
  volumeVar = 0;
  speedVar = 0;
  TurnBackLightOn();
}
