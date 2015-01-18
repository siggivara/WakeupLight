#include <LiquidCrystal.h>
#include <DS1307RTC.h>
#include <Time.h>
#include <TimeAlarms.h>
#include <Wire.h>


//  Pins:
//  0: 
//  1: TX
//  2: 
//  3: 
//  4: LCD D7
//  5: LCD D6
//  6: LCD D5
//  7: LCD D4
//  8: 
//  9: Button Left
//  10: Button Right
//  11: LCD Enable
//  12: LCD RS
//  13: Light Relay


// ######### Time ################
boolean alarmEnabled = false;
//time_t alarmTime;
int alarmHour = 0;
int alarmMinute = 0;
int alarmSecond = 0;

tmElements_t currentTime;
tmElements_t lastTime;

AlarmID_t myAlarmID = 0;  // this will be the ID for the first allocated alarm. We only have one
// ######## End time #############





// ######## Light ################
const int light = 13;
const int lightMaxSecondsOn = 15;
tmElements_t timeLightTurnedOn;
boolean lightOn = false;
// ######## End Light ############





// ######## LCD ##################
LiquidCrystal lcd(12, 11, 7, 6, 5, 4);  // initialize the library with the numbers of the interface pins
boolean cursorEnabled = false;
boolean isEditingDate = false;
//int cursorPos[] = {1, 0};               // [0] => column    [1] => row


typedef enum CursorPos {
  current_hour,
  current_minute,
  current_second,
  date_year,
  date_month,
  date_day,
  alarm_hour,
  alarm_minute,
  alarm_second,
  alarm_state,
  out_of_bounds
};

int currentCursorPos = current_hour;
// ######## End LCD ##############




// ######## Buttons ##############
const int buttonLeft = 9;
const int buttonRight = 10;
int buttonLeftLastState = 0;
int buttonRightLastState = 0;
// ######## End Buttons ##########



void setup() {
  // ######### Time ################
  setSyncProvider(RTC.get);
  myAlarmID = Alarm.alarmRepeat(alarmHour,alarmMinute,alarmSecond, alarm);      // Set alarm handler to be triggered every day
  //Alarm.timerRepeat(1, printTime);                  // Update lcd every second
  // ######## End time #############
  
  
  
  
  
  // ######## Light ################
  turnOff();  
  pinMode(light, OUTPUT);
  // ######## End Light ############
  
  
  
  
  
  // ######## LCD ##################
  lcd.begin(16, 2);  // set up the LCD's number of columns and rows
  printAlarmState();
  // ######## End LCD ##############





  // ######## Buttons ##############
  pinMode(buttonLeft, INPUT);
  pinMode(buttonRight, INPUT);
  // ######## End Buttons ##########

  
  
  
  
  // ######## Debug #################
  //Serial.begin(9600);
  // ######## End debug #############
}

void loop() {
  // ######### Time ################
  if (!cursorEnabled)    // pause time updating when changing time
    updateTime();

  Alarm.delay(0);        // Needed to check if any alarms have been triggered
  // ######## End time #############

  // ######## Buttons ##############
  readButtonsState();
  // ######## End Buttons ##########

}




  // ######### Time ################
 void updateTime() {
    if (RTC.read(currentTime)) {
      if (currentTime.Second != lastTime.Second) {
        printTime();
        //logtime();
        lastTime = currentTime;
      }
    }
 }
 
 
/* void checkAlarm() {
   if (hour(alarmTime) == hour(currentTime) && minute(alarmTime) == minute(currentTime) && second(alarmTime) == second(currentTime)) {
     timeLightTurnedOn = currentTime;
     //Serial.println("Alarm went off");
     turnOn();
   } 
 }
*/
 void changeAlarmState() {
   alarmEnabled = !alarmEnabled; 
   //Serial.print("Changing alarm state to: ");
   //Serial.println(alarmEnabled);
   printAlarmState();
  }
  
  
  void alarm() {
    if (alarmEnabled) {
      timeLightTurnedOn = currentTime;
     //Serial.println("Alarm went off");
     turnOn();
     
     // Safety measure. Turn off light after 15 minutes
     Alarm.timerOnce(lightMaxSecondsOn, turnOff);
    }
  }
  
  void changeAlarmTime(int h, int m, int s) {
     Alarm.write(myAlarmID , AlarmHMS(h,m,s));
  }
  // ######## End time #############
  
  
  
  
  
  // ######## Light ################
  void turnOn() {
    //Serial.println("Light On");
    lightOn = true;
    digitalWrite(light, LOW);
  }
  
  void turnOff() {
   //Serial.println("Light Off");
    lightOn = false;
    digitalWrite(light, HIGH);
  }
  // ######## End Light ############
  
  
  
  
  
  // ######## LCD ##################
  void printTime() {
    printCurrentTime();
    printAlarmTime();
    printAlarmState();
  }
  
  void printCurrentTime() {
    lcd.setCursor(0, 0);        // Set cursor at start of first line
    lcd.print(String(zeroit(currentTime.Hour) + ":" + zeroit(currentTime.Minute) + ":" + zeroit(currentTime.Second) + "   " + zeroit(currentTime.Day) + "/" + zeroit(currentTime.Month) + "-" + tmYearToCalendar(currentTime.Year)));
    positionCursor();
  }
  
  void printAlarmTime() {
    lcd.setCursor(0, 1);
    if (isEditingDate)
      lcd.print("                              ");
    else
      lcd.print(String(zeroit(alarmHour) + ":" + zeroit(alarmMinute) + ":" + zeroit(alarmSecond)));
    positionCursor();
  }
  
  void printAlarmState() {
     lcd.setCursor(11, 1);
     lcd.print(isEditingDate ? "     " : (alarmEnabled ? String("On ") : String("Off")));
     positionCursor();
  }
  
  void positionCursor() {
    switch (currentCursorPos) {
          case current_hour :
            lcd.setCursor(1, 0);
            break;
          
          case current_minute :
            lcd.setCursor(4, 0);
            break;
            
          case current_second :
            lcd.setCursor(7, 0);
            break;
            
          case date_day :
            lcd.setCursor(12, 0);
            break;
            
          case date_month :
            lcd.setCursor(15, 0);
            break;
            
          case date_year:
            lcd.setCursor(20, 0);
          break;
            
          case alarm_hour :
            lcd.setCursor(1, 1);
            break;
            
          case alarm_minute :
            lcd.setCursor(4, 1);
            break;
            
          case alarm_second :
            lcd.setCursor(7, 1);
            break;
            
          case alarm_state :
            lcd.setCursor(11, 1);
            break;
            
          case out_of_bounds :
            disableCursor();
            currentCursorPos = current_hour;  // reset the position of the cursor
            return;
            break;
     } 
  }
  // ######## End LCD ##############




  // ######## Buttons ##############
  int buttonHighCount = 0;
  void readButtonsState() {
    delay(100);
    int buttonLeftState = digitalRead(buttonLeft);
    int buttonRightState = digitalRead(buttonRight);
    
    
   
    
    
    // Both buttons
    if ((buttonLeftState == HIGH && !buttonLeftLastState) && (buttonRightState == HIGH && !buttonRightLastState)) {
      else if ((buttonLeftState == LOW && buttonLeftLastState) && (buttonRightState == LOW && buttonRightLastState)) {
      //Serial.println("Both buttons pressed");
      
      if (lightOn) {    // Means we should turn off the light
        turnOff();
        return;
      }
      
      currentCursorPos++;    // Select next cursor position
      
      if (currentCursorPos == date_day) {
          isEditingDate = true;
          for (int i=0; i<11; i++)
            lcd.scrollDisplayLeft();
      }
        
      else if (currentCursorPos == alarm_hour) {
          isEditingDate = false;
          for (int i=0; i<11; i++)
            lcd.scrollDisplayRight();
      }
      
      if (cursorEnabled) {
        positionCursor();
      }
      else {
        enableCursor(); 
      }
    }
    //else if ((buttonLeftState == LOW && buttonLeftLastState) && (buttonRightState == LOW && buttonRightLastState)) {Serial.println("Both buttons released"); }
     

    // Left button
    else if (buttonLeftState == HIGH && !buttonLeftLastState) {     // Button pressed
      //Serial.println("left pressed");
      if (lightOn)    // Means we should turn off the light
        turnOff();
      else  
        leftButtonHandler();
    } 
    //else if (buttonLeftState == LOW && buttonLeftLastState){        // Button released  Serial.println("left released"); }  
    
    
    // Right button
    else if (buttonRightState == HIGH && !buttonRightLastState) {  // Button pressed
      //Serial.println("right pressed");
      if (lightOn)    // Means we should turn off the light
        turnOff();
      else
        rightButtonHandler();
    } 
    
    //else if (buttonRightState == LOW && buttonRightLastState){     // Button released Serial.println("right released");    }
    
    // Remember last state of buttons
    buttonLeftLastState = buttonLeftState;
    buttonRightLastState = buttonRightState;
    
    printTime();              // Update lcd display
  }
  
  void rightButtonHandler() {
    if (cursorEnabled) {          // If cursor is enabled. Handle the button press
      switch (currentCursorPos) {
        case current_hour: // hour              
            (currentTime.Hour == 23) ? 
              currentTime.Hour = 0 :                // Prevent changing date when going from 23 to 00
              currentTime.Hour += 1;                // Cursor at clock hour. Add one hour to currentTime
              break;
       
        case alarm_hour:                              
          (alarmHour == 23) ?                       // Cursor at alarm hour hour. Add one hour to alarmTime  
              alarmHour = 0 :                       // Prevent changing date when going from 23 to 00
              alarmHour += 1;                        
          break;
           
        case current_minute:  // minute            
          (currentTime.Minute == 59) ?
            currentTime.Minute = 0 :               // Rollover
            currentTime.Minute += 1;               // Cursor at clock minute. Add one minute to currentTime
          break;
            
        case alarm_minute:
          (alarmMinute == 59) ?           
            alarmMinute = 0 :                      // Rollover
            alarmMinute += 1;                      // Cursor at alarm minute. Add one minute to alarmTime
          break;
            
        case current_second:  // second
          (currentTime.Second == 59) ?             // Cursor at clock second. Add one second to currentTime
            currentTime.Second = 0 :               // Rollover
            currentTime.Second += 1;    
          break;
          
        case alarm_second :                        // Cursor at clock second. Add one second to currentTime
          (alarmSecond == 59) ?
            alarmSecond = 0 :                      // Rollover
            alarmSecond += 1;                      // Cursor at alarm second. Add one second to alarmTime
          break;
        
        case date_day:
          {
            time_t timeInSeconds = makeTime(currentTime);    // Convert to time_t (seconds)
            timeInSeconds += SECS_PER_DAY;                   // Add one day worth of seconds
            breakTime(timeInSeconds, currentTime);           // Convert back to tmElements_t
          }
          break;
        
        case date_month:
          (currentTime.Month == 12) ?
            currentTime.Month = 1 :
            currentTime.Month += 1;
          break;
       
       case date_year :
          currentTime.Year += 1;      
          break;
        
        case alarm_state:
          changeAlarmState();
          break;
      }  
    }
  }
  
  void leftButtonHandler() {
    if (cursorEnabled) {          // If cursor is enabled. Handle the button press
      switch (currentCursorPos) {
        case current_hour: // hour              
            (currentTime.Hour == 0) ? 
              currentTime.Hour = 23 :                // Prevent changing date when going from 23 to 00
              currentTime.Hour -= 1;                // Cursor at clock hour. Add one hour to currentTime
              break;
       
        case alarm_hour:                              
          (alarmHour == 0) ?                       // Cursor at alarm hour hour. Add one hour to alarmTime  
              alarmHour = 23 :                       // Prevent changing date when going from 23 to 00
              alarmHour -= 1;                        
          break;
           
        case current_minute:  // minute            
          (currentTime.Minute == 0) ?
            currentTime.Minute = 59 :               // Rollover
            currentTime.Minute -= 1;               // Cursor at clock minute. Add one minute to currentTime
          break;
            
        case alarm_minute:
          (alarmMinute == 0) ?           
            alarmMinute = 59 :                      // Rollover
            alarmMinute -= 1;                      // Cursor at alarm minute. Add one minute to alarmTime
          break;
            
        case current_second:  // second
          (currentTime.Second == 0) ?             // Cursor at clock second. Add one second to currentTime
            currentTime.Second = 59 :               // Rollover
            currentTime.Second -= 1;    
          break;
          
        case alarm_second :                        // Cursor at clock second. Add one second to currentTime
          (alarmSecond == 0) ?
            alarmSecond = 59 :                      // Rollover
            alarmSecond -= 1;                      // Cursor at alarm second. Add one second to alarmTime
          break;
        
        case date_day :
          {
            time_t timeInSeconds = makeTime(currentTime);    // Convert to time_t (seconds)
            timeInSeconds -= SECS_PER_DAY;                   // Remove one day worth of seconds
            breakTime(timeInSeconds, currentTime);           // Convert back to tmElements_t
          }
          break;
        
        case date_month :
          (currentTime.Month == 0) ?
            currentTime.Month = 12 :
            currentTime.Month -= 1;
          break;
       
       case date_year :
          currentTime.Year -= 1;      
          break;
        
        case alarm_state:
          changeAlarmState();
          break;
      }
    }
  }
  
  void enableCursor() {
    //Serial.println("Enabling cursor");
    cursorEnabled = true;
    lcd.setCursor(1, 0);
    currentCursorPos = current_hour;
    lcd.blink();
  }
  
  void disableCursor() {
    //Serial.println("Disabling cursor");
    cursorEnabled = false;
    lcd.noBlink();
    RTC.write(currentTime);      // After editing time, set current time as the edited time
  }
  // ######## End Buttons ##########

  
  
  
  // ######## Helpers #################
  String zeroit(int value)
  {
    if (value < 10)
      return String("0" + String(value, DEC));
      
    return String(value, DEC);
  }
  // ######## End helpers #############
  
  
  
  
  // ######## Debug #################
  /*void logtime()
  {
     Serial.print("  Time = ");
     Serial.print(zeroit(currentTime.Hour));
     Serial.print(":");
     Serial.print(zeroit(currentTime.Minute));
     Serial.print(":");
     Serial.println(zeroit(currentTime.Second));
     return;
  }*/
  // ######## End debug #############


