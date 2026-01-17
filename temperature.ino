//Interface the DHT11 Temp & Humidity sensor and display humidity and temperature
//in Fahrenheit on a 16x2 character LCD with mode switching button

#include <dht_nonblocking.h>
#include <LiquidCrystal.h>

#define DHT_SENSOR_TYPE DHT_TYPE_11
#define DHT_SENSOR_PIN 8
#define BUTTON_PIN 9
#define BUTTON2_PIN 10

DHT_nonblocking dht_sensor(DHT_SENSOR_PIN, DHT_SENSOR_TYPE);
const int RS = 2, EN = 3, D4 = 4, D5 = 5, D6 = 6, D7 = 7;
LiquidCrystal lcd(RS,EN,D4,D5,D6,D7);   //set Uno pins that are connected to LCD, 4-bit mode

int displayMode = 0;  // 0 = Temp F, 1 = Temp C, 2 = Elapsed Time, 3 = Clock
bool lastButtonState = HIGH;
bool buttonState = HIGH;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;
unsigned long startTime = 0;

// Button 2 variables for time setting
bool lastButton2State = HIGH;
bool button2State = HIGH;
unsigned long lastDebounceTime2 = 0;
bool settingMode = false;  // true when setting clock time
int settingField = 0;  // 0 = hours, 1 = minutes, 2 = seconds

// Clock time variables
int clockHours = 12;
int clockMinutes = 0;
int clockSeconds = 0;
unsigned long lastClockUpdate = 0;

// Sensor data variables (persistent across loops)
float temperature = 0.0;
float humidity = 0.0;

void setup() {
  pinMode(BUTTON_PIN, INPUT_PULLUP);  //button connected to pin 9 with internal pull-up
  pinMode(BUTTON2_PIN, INPUT_PULLUP);  //button 2 connected to pin 10 with internal pull-up
  
  lcd.begin(16,2);    //set 16 columns and 2 rows of 16x2 LCD
  lcd.clear();        //clear any initial garbage on display
  lcd.setCursor(0,0);
  lcd.print("Initializing...");
  delay(2000);        //give DHT sensor time to stabilize
  
  startTime = millis();  //record start time for elapsed time display
  lastClockUpdate = millis();  //initialize clock update time
}

void loop() {
  // Read button 1 with debouncing
  bool reading = digitalRead(BUTTON_PIN);
  
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }
  
  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != buttonState) {
      buttonState = reading;
      
      // Button pressed (LOW because of pull-up)
      if (buttonState == LOW) {
        if (settingMode) {
          // In setting mode, button 1 increments the current field only
          if (settingField == 0) {
            clockHours = (clockHours + 1) % 24;
          } else if (settingField == 1) {
            clockMinutes = (clockMinutes + 1) % 60;
          } else if (settingField == 2) {
            clockSeconds = (clockSeconds + 1) % 60;
          }
        } else {
          // Normal mode - cycle through display modes
          displayMode = (displayMode + 1) % 4;  // Cycle through 0, 1, 2, 3
          lcd.clear();
        }
      }
    }
  }
  
  lastButtonState = reading;
  
  // Read button 2 with debouncing (time setting button)
  bool reading2 = digitalRead(BUTTON2_PIN);
  
  if (reading2 != lastButton2State) {
    lastDebounceTime2 = millis();
  }
  
  if ((millis() - lastDebounceTime2) > debounceDelay) {
    if (reading2 != button2State) {
      button2State = reading2;
      
      // Button 2 pressed (LOW because of pull-up)
      if (button2State == LOW) {
        if (displayMode == 3) {  // Only works in clock mode
          if (settingMode) {
            // In setting mode, button 2 advances to next field
            settingField++;
            if (settingField > 2) {
              // After seconds, save and exit setting mode
              settingMode = false;
              settingField = 0;
              lastClockUpdate = millis();  // Reset clock update time
              lcd.clear();
            }
          } else {
            // Enter setting mode
            settingMode = true;
            settingField = 0;  // Start with hours
            lcd.clear();
          }
        }
      }
    }
  }
  
  lastButton2State = reading2;
  
  // Keep calling measure() - it will return true when data is ready
  if(dht_sensor.measure(&temperature, &humidity)) {
    
    if(displayMode == 0) {
      // Display Temperature in Fahrenheit
      lcd.clear();
      float tempF = (temperature * 9.0 / 5.0) + 32.0;
      
      lcd.setCursor(0,0); 
      lcd.print("Temp: ");
      lcd.print(tempF, 1);
      lcd.write(0xDF);
      lcd.print("F");
      
      lcd.setCursor(0,1);
      lcd.print("Humidity: ");
      lcd.print(humidity, 1);
      lcd.print("%");
      
    } else if(displayMode == 1) {
      // Display Temperature in Celsius
      lcd.clear();
      
      lcd.setCursor(0,0); 
      lcd.print("Temp: ");
      lcd.print(temperature, 1);
      lcd.write(0xDF);
      lcd.print("C");
      
      lcd.setCursor(0,1);
      lcd.print("Humidity: ");
      lcd.print(humidity, 1);
      lcd.print("%");
    }
  }
  
  // Display elapsed time (always updated)
  if(displayMode == 2) {
    unsigned long elapsedSeconds = (millis() - startTime) / 1000;
    unsigned long hours = elapsedSeconds / 3600;
    unsigned long minutes = (elapsedSeconds % 3600) / 60;
    unsigned long seconds = elapsedSeconds % 60;
    
    lcd.setCursor(0,0);
    lcd.print("Elapsed Time:   ");
    lcd.setCursor(0,1);
    
    if(hours < 10) lcd.print("0");
    lcd.print(hours);
    lcd.print(":");
    if(minutes < 10) lcd.print("0");
    lcd.print(minutes);
    lcd.print(":");
    if(seconds < 10) lcd.print("0");
    lcd.print(seconds);
    
    delay(100);  // Small delay for time display to prevent flickering
  }
  
  // Display clock mode
  if(displayMode == 3) {
    // Update clock time if not in setting mode
    if (!settingMode) {
      unsigned long currentMillis = millis();
      if (currentMillis - lastClockUpdate >= 1000) {
        lastClockUpdate = currentMillis;
        clockSeconds++;
        if (clockSeconds >= 60) {
          clockSeconds = 0;
          clockMinutes++;
          if (clockMinutes >= 60) {
            clockMinutes = 0;
            clockHours++;
            if (clockHours >= 24) {
              clockHours = 0;
            }
          }
        }
      }
    }
   

    lcd.setCursor(0,0);
    if (settingMode) {
      lcd.print("SET  :");
    } else {
      lcd.print("CLOCK:");
    }
    
    lcd.setCursor(6,0);
    
    // Display hours (blink if setting)
    if (settingMode && settingField == 0) {
      if ((millis() / 500) % 2 == 0) {
        lcd.print("  ");
      } else {
        if(clockHours < 10) lcd.print("0");
        lcd.print(clockHours);
      }
    } else {
      if(clockHours < 10) lcd.print("0");
      lcd.print(clockHours);
    }
    
    lcd.print(":");
    
    // Display minutes (blink if setting)
    if (settingMode && settingField == 1) {
      if ((millis() / 500) % 2 == 0) {
        lcd.print("  ");
      } else {
        if(clockMinutes < 10) lcd.print("0");
        lcd.print(clockMinutes);
      }
    } else {
      if(clockMinutes < 10) lcd.print("0");
      lcd.print(clockMinutes);
    }
    
    lcd.print(":");
    
    // Display seconds (blink if setting)
    if (settingMode && settingField == 2) {
      if ((millis() / 500) % 2 == 0) {
        lcd.print("  ");
      } else {
        if(clockSeconds < 10) lcd.print("0");
        lcd.print(clockSeconds);
      }
    } else {
      if(clockSeconds < 10) lcd.print("0");
      lcd.print(clockSeconds);
    }
    if(dht_sensor.measure(&temperature, &humidity)) {
    // Display temperature and humidity on line 2
    float tempF = (temperature * 9.0 / 5.0) + 32.0;
    
    lcd.setCursor(0,1);
    lcd.print("                ");  // Clear line 2
    lcd.setCursor(0,1);
    lcd.print(tempF, 1);
    lcd.write(0xDF);
    lcd.print("F");
    lcd.print(" ");
    lcd.print(humidity, 1);
    lcd.print("%");
    
    delay(100);  // Small delay to prevent flickering
  }}
}