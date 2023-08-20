#include <M5StickCPlus.h>
#include <RTClib.h>

int counter = 0;
bool showClock = false;  // flag to decide if the clock or counter is displayed
int currentHour = 0;
int currentMinute = 0;
//int currentState = 0;  // 0 for hours, 1 for minutes

enum DisplayState {
  COUNTER,
  CLOCK,
  BATTERY
};
DisplayState currentState = COUNTER; // Start with the counter screen

void setRTC() {
    RTC_TimeTypeDef timeStruct;
    timeStruct.Hours = currentHour;
    timeStruct.Minutes = currentMinute;
    timeStruct.Seconds = 0;
    M5.Rtc.SetTime(&timeStruct);  // Pass the address of timeStruct to SetTime
}

void updateDisplay();  // Forward declaration of updateDisplay function

uint16_t RGB24toRGB565(uint32_t rgb24) { //used to setup hex colors
  uint8_t r = (rgb24 & 0xFF0000) >> 16;
  uint8_t g = (rgb24 & 0x00FF00) >> 8;
  uint8_t b = rgb24 & 0x0000FF;

  return ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3);
}

void displayTimeSetting() {
    M5.Lcd.fillScreen(TFT_BLACK);
    M5.Lcd.setTextSize(4);
    M5.Lcd.setCursor(20, 50);

    // Print the hour
    if (currentState == 0) {
        M5.Lcd.setTextColor(TFT_RED);  // Highlighting the hour when it's selected
    } else {
        M5.Lcd.setTextColor(TFT_WHITE);
    }
    M5.Lcd.print(currentHour);

    // Reset text color for the colon
    M5.Lcd.setTextColor(TFT_WHITE);
    M5.Lcd.print(":");

    // Print the minute
    if (currentState == 1) {
        M5.Lcd.setTextColor(TFT_RED);  // Highlighting the minute when it's selected
    } else {
        M5.Lcd.setTextColor(TFT_WHITE);
    }
    M5.Lcd.print(currentMinute);
}

void setup() {
  M5.begin();
  M5.IMU.Init();
  uint16_t rPurple = RGB24toRGB565(0x9300ff);
  M5.Lcd.fillScreen(rPurple);
  M5.Lcd.setTextColor(TFT_WHITE);
  M5.Rtc.begin();  // Initialize the RTC

  updateDisplay(); // Initial display update
}

void loop() {
  M5.update();  // Update button states

  // Button A (top) to increment counter and adjust time
if (M5.BtnA.wasPressed()) {
    if (showClock) {
      if (currentState == CLOCK) {
    if (currentHour < 23) {
        currentHour++;
    } else {
        currentHour = 0;
    }
} else if (currentState == COUNTER) {
    if (currentMinute < 59) {
        currentMinute++;
    } else {
        currentMinute = 0;
    }
}
      setRTC();  // <-- Update the RTC with the adjusted time
      displayTimeSetting();
    } else {
      counter++;
      delay(200);  // Debounce delay
    }
    updateDisplay();
}

  // Button B (bottom) to decrement counter and toggle time setting state
  if (M5.BtnB.wasPressed()) {
    if (showClock) {
      if (currentState == CLOCK) {
    currentState = COUNTER;
} else if (currentState == COUNTER) {
    currentState = CLOCK;
}
      displayTimeSetting();
    } else {
      counter--;
      delay(200);  // Debounce delay
    }
    updateDisplay();
  }
  
  // Shake detection for switching between clock and counter
  float ax, ay, az;
  M5.IMU.getAccelData(&ax, &ay, &az);
  float magnitude = sqrt(ax * ax + ay * ay + az * az);

  float shakeThreshold = 4; // Adjust this value based on sensitivity preference
  if (magnitude > shakeThreshold) {
  currentState = (currentState == CLOCK) ? COUNTER : CLOCK;
  delay(500); // Prevent repeated toggles from continuous shaking
  updateDisplay();
}

  // Check simultaneous button press
  if (M5.BtnA.isPressed() && M5.BtnB.isPressed()) {
    currentState = BATTERY;
    updateDisplay();
    delay(500);  // Debounce delay for this condition
  }

  delay(10);
}

void updateDisplay() {
  uint16_t rPurple = RGB24toRGB565(0x9300ff);
  M5.Lcd.fillScreen(rPurple);

  switch(currentState) {
    case COUNTER:
      displayCounter();
      break;
    case CLOCK:
      displayClock();
      break;
    case BATTERY:
      displayBattery();
      break;
  }
}

void displayBattery() {
  M5.Lcd.setRotation(1);  // Landscape mode
  M5.Lcd.setTextSize(4);
  M5.Lcd.setCursor(0, 5);
  M5.Lcd.println("Battery:");

  float batteryVoltage = M5.Axp.GetBatVoltage();
  
  M5.Lcd.setTextSize(26);
  M5.Lcd.setCursor(30, 80);
  M5.Lcd.print(batteryVoltage);
  M5.Lcd.print("V");
}

void displayCounter() {
  M5.Lcd.setRotation(0);  // Revert the rotation to default (portrait mode)
  M5.Lcd.setTextSize(4);
  M5.Lcd.setCursor(0, 5);
  M5.Lcd.println("Quest Pts:");

  M5.Lcd.setTextSize(48);
  M5.Lcd.setCursor(30, 120);
  M5.Lcd.println(counter);
}

void displayClock() {
  M5.Lcd.setRotation(1);  // Rotate the display by 90 degrees
  M5.Lcd.setTextSize(4);
  M5.Lcd.setCursor(0, 5);
  M5.Lcd.println("Time:");

  RTC_TimeTypeDef nowTime;
  M5.Rtc.GetTime(&nowTime);  // Get the current time

  char timeStr[10];
  sprintf(timeStr, "%02d:%02d", nowTime.Hours, nowTime.Minutes); // Access the hours and minutes from the structure

  M5.Lcd.setTextSize(26);
  M5.Lcd.setCursor(30, 80);
  M5.Lcd.println(timeStr);
}