/* Code written by Tom Cousins for the Homebrew GPS. */
/* Code released into the public domain under the Open Source GNU license*/
/* http://doayee.co.uk/homebrew-gps */

/* P.s. I know this code is shitty, I wrote it when I was 14. I should really update it with 
   seperate fuctions and stuff, but nah... Can't be bothered! */

#define OLED_DC 11     //Define pins for the OLED
#define OLED_CS 12
#define OLED_CLK 10
#define OLED_MOSI 9
#define OLED_RESET 13
#include <Wire.h>             //Include the relevant libraries
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
Adafruit_SSD1306 display(OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);  //Create object for the OLED
#define LOGO16_GLCD_HEIGHT 16  //Define the OLED width and height
#define LOGO16_GLCD_WIDTH  16 
#include <Adafruit_GPS.h>     //Include more libraried
#include <SoftwareSerial.h>
SoftwareSerial mySerial(3, 2);  //Set up a serial port on pins 3 and 2
#define GPSECHO  true           //Set the GPS echo on
Adafruit_GPS GPS(&mySerial);    //Initialise the GPS with the serial port set up previously
boolean usingInterrupt = false; //Initialse variables used in the rest of the program
boolean sleep = false;
boolean logging = false;
int mode = 0;
int page = 0;
int left = 6;
int mid = 5; 
int right = 4; 
int tzhour;
int tzday;
int fixflag = 0; 
int timezone = 0;
int dst = 1;
float maxspeed;
float maxalt;
boolean invert = false;

void setup() 
{
  if (dst == 1) //Check if it is daylight saving time
  {
    timezone--;
  }
  pinMode(7, OUTPUT);    //Wake the GPS up
  digitalWrite(7, HIGH); 
  pinMode(left, INPUT);  //Set up buttons
  pinMode(mid, INPUT);
  pinMode(right, INPUT);
  display.begin(SSD1306_SWITCHCAPVCC); //Begin the screen
  display.clearDisplay();                  //Print the loading message:
  display.setTextSize(1);                  //
  display.setTextColor(WHITE);             //
  display.setCursor(0,0);                  //
  display.println(" Where in the world?"); // 
  display.println("       V1.0.7");        //
  display.println("  A GPS project by");   // 
  display.println("    Tom Cousins");      //
  display.display();                       //
  delay(1000);                             // 
  display.clearDisplay();                  //
  GPS.begin(9600);           //Begin the GPS
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA); //Set the output type of data
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);    // Tell it to update once per second
  useInterrupt(true); 
  delay(1000);
}

SIGNAL(TIMER0_COMPA_vect) //Mandatory updating stuff
{
  char c = GPS.read();
  if (GPSECHO)
    if (c) UDR0 = c;  
}

void useInterrupt(boolean v) { //Mandatory updating stuff
  if (v) 
  {
    OCR0A = 0xAF;
    TIMSK0 |= _BV(OCIE0A);
    usingInterrupt = true;
  } 
  else 
  {
    TIMSK0 &= ~_BV(OCIE0A);
    usingInterrupt = false;
  }
}

uint32_t timer = millis();  //Mandatory updating stuff

void loop() 
{
  if (digitalRead(right) == HIGH && sleep == false) //If 'Mode Up' button is pressed, advance mode one
  {
    while(digitalRead(right) == HIGH);
    mode++;
    if(mode > 5) //Scroll back to begining
    {
      mode = 0; 
    }

  }
  
  if (digitalRead(left) == HIGH && sleep == false) //If 'Mode Down' button is pressed, de-advance (is that a word?) the mode one
  {
    while(digitalRead(left) == HIGH);
    mode--;
    if(mode < 0) //Scroll to the end
    {
      mode = 5; 
    }
  }
  
  if (digitalRead(mid) == HIGH && sleep == false && mode != 5)     //If 'Menu' button is pressed and the mode is not the logging mode, and the GPS is awake
  {
    delay(20);
    while(digitalRead(mid) == HIGH);
    page = 0;
    while(digitalRead(left) == LOW && digitalRead(right) == LOW) //While they haven't chosen an option
    {
      display.invertDisplay(invert);         //Display menu
      display.clearDisplay();
      display.setTextSize(1);
      display.setCursor(0,0);
      display.println("        Menu");
      display.println("     Top: Sleep");
      display.println("   Middle: Invert");
      display.println("    Bottom: Exit");
      display.display();
      if(digitalRead(mid) == HIGH)       //If they invert the text display, do that.
      {
        invert = !invert;
        while(digitalRead(mid) == HIGH);
      }
    }
    if(digitalRead(left) == HIGH)    //If they send it to sleep
    {
      display.invertDisplay(invert);  //Display the "Goodnight" message
      display.clearDisplay();
      display.setCursor(0,8);
      display.setTextSize(2);
      display.println(" Goodnight");
      display.display();
      delay(500);
      display.invertDisplay(false);
      display.clearDisplay();
      display.display();
      sleep = true;  //Sleep
    }   
    else if(digitalRead(right) == HIGH) //Exit the menu
    {
      while(digitalRead(right) == HIGH);
      sleep = false;
    }
  }
  
  if (digitalRead(mid) == HIGH && mode == 5) //If the middle button is pressed and the mode is the logging mode
  {
    if (GPS.LOCUS_StartLogger())      //If the logging started succesfully
    {
      display.invertDisplay(invert);  //Print the logging succesful message
      display.clearDisplay();
      display.setTextSize(2);
      display.setCursor(0,0);
      display.println("  Logging");
      display.println(" Succesful");
      display.display();
      delay(1000);
      logging = true; //Set the logging to true
    }
    else
    {
      logging = false;
      display.invertDisplay(invert); //Display the logging unsuccessful message
      display.clearDisplay();
      display.setTextSize(2);
      display.setCursor(0,0);
      display.println("  Logging");
      display.println("Unsuccesful");
      display.display();
      delay(1000);
    }
  }

  if (sleep == true)
  {
    digitalWrite(7, LOW);            
    if(digitalRead(left) == HIGH) //Scan the wake up button
    {
      display.invertDisplay(invert); 
      digitalWrite(7, HIGH);  //Wake the GPS up
      display.clearDisplay(); //Display awake message
      display.setCursor(0,0);
      display.setTextSize(2);
      display.println("    I'm");
      display.println("   Awake!");
      display.display();
      delay(500);
      mode = 0; //Set mode to default
      sleep = false; //Set sleep to false
    }
  }
  else
  {

    if (! usingInterrupt) {                     //Updating stuff
      char c = GPS.read();
      if (GPSECHO)
        if (c) UDR0 = c;
    }

    if (GPS.newNMEAreceived()) {

      if (!GPS.parse(GPS.lastNMEA()))
        return;
    }

    if (timer > millis())  timer = millis();

    if ((GPS.speed * 1.150779) > maxspeed) { //Update maxspeed
      maxspeed = GPS.speed * 1.150779;
    }

    if (GPS.altitude > maxalt) { //Update maxheight
      maxalt = GPS.altitude;
    }

    if (millis() - timer > 500) {
      timer = millis(); // reset the timer
    }
    if (mode == 0)                             //Display Default Mode Text
    {
      display.invertDisplay(invert);
      display.clearDisplay();
      display.fillRect(0, 0, 127, 8, BLACK);
      display.setCursor(0, 0);
      display.setTextSize(1);
      
      if (GPS.hour >= 0 && GPS.hour < timezone) 
      {
        tzhour = GPS.hour + 20;
      }
      else 
      {
        tzhour = GPS.hour - timezone;
      }
      
      if (tzhour < 10) 
      {
        display.print("0");
      }
      
      display.print(tzhour, DEC); 
      display.print(':');
      
      if (GPS.minute < 10) 
      {
        display.print("0");
      }
      
      display.print(GPS.minute, DEC); 
      display.print(':');
      
      if (GPS.seconds < 10) 
      {
        display.print("0");
      }
      
      display.print(GPS.seconds, DEC);    

      display.print("   ");
     
      if (tzhour >= 20) 
      {
        tzday = GPS.day - 1;
      }
      else 
      {
        tzday = GPS.day;
      }
      
      if (tzday < 10) 
      {
        display.print("0");
      }
      
      display.print(tzday, DEC);
      display.print("/");
      if (GPS.month < 10)  
      {
        display.print("0");
      }
      display.print(GPS.month, DEC);
      display.print("/");
      display.print("20");
      display.print(GPS.year, DEC);

      if (GPS.fix == 0) //If it has no fix
      {
        display.fillRect(0, 16, 127, 16, BLACK);
        display.setTextSize(1);
        display.setCursor(0, 16);
        display.println("Harassing Satellites");
        display.println("      For a fix!");
      }
      
      if (GPS.fix) //If it has a fix
      {
        fixflag = 1;                            //Display all the info
        display.fillRect(0, 8, 128, 24, BLACK);
        display.setCursor(0, 8);
        display.print("Lat:");
        display.print(GPS.lat);
        display.print(GPS.latitude, 4);
        display.setCursor(0, 16);
        display.print("Lon:");
        display.print(GPS.lon);
        display.print(GPS.longitude, 4);
        display.setCursor(0, 24);
        display.print("Alt:");
        display.print(GPS.altitude);
        display.setCursor(92, 24);
        if (GPS.fix == 1) 
        {
          display.print("Fix:");
          display.print(GPS.satellites);
        }
        if (GPS.fix == 0) 
        {
          display.setTextColor(BLACK, WHITE);
          display.print("NO FIX");
          display.setTextColor(WHITE);
        }
      }
    }

    if (mode == 1) //Display mode 2 text
    {
      display.clearDisplay();
      display.setCursor(0, 0);
      if (GPS.fix)  //If it has a fix
      {
        display.invertDisplay(invert);
        display.setTextSize(1);
        display.setCursor(0, 0);
        display.print("Lat:");
        display.print(GPS.lat);
        display.print(GPS.latitude, 4);
        display.setCursor(0, 8);
        display.print("Lon:");
        display.print(GPS.lon);
        display.print(GPS.longitude, 4);
        display.setCursor(0, 16);
        display.print("Speed(MPH):");
        display.print(GPS.speed * 1.150779); //*converts to MPH
        display.setCursor(0, 24);
        display.print("Bearing:");
        display.print(GPS.angle);
        display.setCursor(90, 24);
        display.print("Fix:");
        display.print(GPS.satellites);
      }
      else
      { //If it has no fix
        display.invertDisplay(invert);
        display.setTextSize(1);
        display.setCursor(0, 0);
        display.print("Lat:");
        display.print(" N/A");
        display.setCursor(0, 8);
        display.print("Lon:");
        display.print(" N/A");
        display.setCursor(0, 16);
        display.print("Speed(MPH):");
        display.print(" N/A");
        display.setCursor(0, 24);
        display.print("Bearing:");
        display.print(" N/A");
        display.setCursor(90, 24);
        display.print("Fix:");
        display.print("0");
      }
    }
    
    if (mode == 2) //Display speed and bearing
    {
      display.clearDisplay();
      display.setCursor(0, 0);
      if(GPS.fix) //If it has a fix
      {
        display.invertDisplay(invert);
        display.clearDisplay();
        display.setTextSize(2);
        display.setCursor(0,0);
        display.print(GPS.speed * 1.150779);
        display.print(" MPH");
        display.setCursor(0, 16);
        display.print(GPS.angle);
        display.print(" DEG");
      }
      else
      {  //You get the point by now...
        display.invertDisplay(invert);
        display.clearDisplay();
        display.setTextSize(2);
        display.setCursor(0,0);
        display.print("N/A ");
        display.print(" MPH");
        display.setCursor(0, 16);
        display.print("N/A ");
        display.print(" DEG");
      }
    }
    
    if (mode == 3) //Display time and date
    {
      display.invertDisplay(invert);
      display.clearDisplay();
      display.setCursor(0, 0);
      display.setTextSize(2);
      
      if (GPS.hour >= 0 && GPS.hour < timezone) {
        tzhour = GPS.hour + 20;
      }
      else 
      {
        tzhour = GPS.hour - timezone;
      }
      
      if (tzhour < 10) 
      {
        display.print("0");
      }
      
      display.print(tzhour, DEC); 
      display.print(':');
      
      if (GPS.minute < 10) 
      {
        display.print("0");
      }
      
      display.print(GPS.minute, DEC); 
      display.print(':');
      
      if (GPS.seconds < 10) 
      {
        display.print("0");
      }
      
      display.println(GPS.seconds, DEC); 
      
      
      if (tzhour >= 20) 
      {
        tzday = GPS.day - 1;
      }
      else 
      {
        tzday = GPS.day;
      }
      
      if (tzday < 10) 
      {
        display.print("0");
      }
      
      display.print(tzday, DEC);
      display.print("/");
      if (GPS.month < 10) 
      {
        display.print("0");
      }
      
      display.print(GPS.month, DEC);
      display.print("/");
      
      display.print("20");
      display.print(GPS.year, DEC);
    }
    
    if (mode == 4) //Display Time BIG!
    {
      display.invertDisplay(invert);
      display.clearDisplay();
      display.setCursor(4, 2);
      display.setTextSize(4);
      
      if (GPS.hour >= 0 && GPS.hour < timezone)
      {
        tzhour = GPS.hour + 20;
      }
      else 
      {
        tzhour = GPS.hour - timezone;
      }
      
      if (tzhour < 10) 
      {
        display.print("0");
      }
      
      display.print(tzhour, DEC); 
      display.print(':');
      
      if (GPS.minute < 10) 
      {
        display.print("0");
      }
      display.print(GPS.minute, DEC);
    }
    
    if (mode == 5) //Logging mode
    {
      display.invertDisplay(invert);
      display.clearDisplay();
      display.setCursor(0, 8);
      display.setTextSize(2);
      if(logging) //If it is logging
      {
        display.println("Logging...");
      }
      else //If not:
      {
        display.setTextSize(1);
        display.setCursor(0, 8);
        display.println("    Push Middle");
        display.setCursor(0, 16);
        display.println("  To start logging");
      }
    }
    display.display(); //Update Screen
  }
}







