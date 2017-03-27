#include <EEPROM.h>

#define  STICK_X 9
#define STICK_Y 8

const int rowCount = 5;  // Number of row pins
const int colCount = 4;  // Number of column pins
const int thumbButton = 1;
const int joystickButton = 21;
int rowPins[rowCount] = {6, 5, 4, 3, 2};  // Matrix row pin numbers
int colPins[colCount] = {10, 9, 8, 7};  // Matrix column pin numbers
int Xstick;
int Ystick;
int deadzone;
int upperBound;
int lowerBound;
bool isInCalibration;
bool isCalibrationButtonPressed;
unsigned long calibrationButtonLastPressedTimeStamp;
unsigned long calibrationButtonPressedDuration;
unsigned long calibrationWriteLastTimeStamp;

int keys[rowCount][colCount] = {
  {1,2, 3, 4},
  {5, 6, 7, 8},
  {9, 10, 11, 12},
  {13, 14,15, 16},
  {17, 18, 19, 20}
};

void setup() {
  resetColumns();  // Setup column pins
  pinMode(thumbButton, INPUT_PULLUP);
  pinMode(joystickButton, INPUT_PULLUP);
  
  pinMode(14, INPUT_PULLUP);
  pinMode(15, INPUT_PULLUP);
  pinMode(16, INPUT_PULLUP);
  pinMode(17, INPUT_PULLUP);
  pinMode(18, INPUT_PULLUP);
  
  Joystick.useManualSend(true);
  
  setDeadzone();
  setBounds();
  isInCalibration = false;
  isCalibrationButtonPressed = false;
  calibrationButtonLastPressedTimeStamp = 0;
  calibrationButtonPressedDuration = 0;
  calibrationWriteLastTimeStamp = 0;
  
  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);
}

void loop() {
  scanMatrix();  // Scan matrix rows and columns
}

// Run through matrix row and column pins
void scanMatrix() {
  Xstick = analogRead(STICK_X);
  Ystick = 1023 - analogRead(STICK_Y);
  isCalibrationButtonPressed = !digitalRead(joystickButton);
  
  if (!isInCalibration) {
    checkCalibrationTrigger();
    
    if ((Xstick > 512 && Xstick <= upperBound) || (Xstick < 512 && Xstick >= lowerBound)) {
      Xstick = 512;
    }
  
    if ((Ystick > 512 && Ystick <= upperBound) || (Ystick < 512 && Ystick >= lowerBound)) {
      Ystick = 512;
    }

    Joystick.X(Xstick);
    Joystick.Y(Ystick);
    
    // Activate each row pin one at a time
  for (int i = 0; i < rowCount; i++) {
    int row = rowPins[i];
    
    activateRow(row);

    // Test each column one at a time
    for (int z = 0; z < colCount; z++) {
      int col = colPins[z];
      int key = keys[i][z];

      // Check the column for low voltage
      if (checkColumn(col)) {
          Joystick.button(key, true); // Press the key
      }
      else {
          Joystick.button(key, false);  // Release the key
      }
    }

    digitalWriteFast(row, HIGH);  // Set the row high
  }
  
  Joystick.button(21, !digitalRead(thumbButton));
  Joystick.button(22, !digitalRead(joystickButton));
  
  Joystick.button(23, !digitalRead(14));
  Joystick.button(24, !digitalRead(15));
  Joystick.button(25, !digitalRead(16));
  Joystick.button(26, !digitalRead(17));
  Joystick.button(27, !digitalRead(18));
  
  Joystick.Z(512);
  Joystick.Zrotate(512);
  Joystick.sliderLeft(0);
  Joystick.sliderRight(0);
  Joystick.hat(-1);
  
  Joystick.send_now();
  }
  else {
    persistBounds();
  }
}

// Set all column pins to input pullup
void resetColumns() {
  for (int i = 0; i < colCount; i++) {
    int col = colPins[i];

    pinMode(col, INPUT_PULLUP);
  }
}

// Verify row pin is output and set low voltage
void activateRow(int row) {
  pinMode(row, OUTPUT);
  digitalWriteFast(row, LOW);
}

// Check column pin
boolean checkColumn(int col) {
  boolean returnValue = false;
  pinMode(col, INPUT_PULLUP);

  if (digitalRead(col) == LOW) {
    returnValue = true;
  }
  else {
    returnValue = false;
  }

  return returnValue;
}

void setDeadzone() {
  deadzone = 0;
  deadzone = EEPROM.read(0) << 8 | EEPROM.read(1);
}

void setBounds() {
  deadzone = deadzone - 512;
  upperBound = 512 + (deadzone + 10);
  lowerBound = 512 - (deadzone + 10);
}

void checkCalibrationTrigger() {
  unsigned long now = millis();
  
  if (isCalibrationButtonPressed) {
    if (calibrationButtonLastPressedTimeStamp == 0) {
      calibrationButtonLastPressedTimeStamp = now;
    }
    
    calibrationButtonPressedDuration = calibrationButtonPressedDuration + (now - calibrationButtonLastPressedTimeStamp);
    calibrationButtonLastPressedTimeStamp = now;
  }
  else {
    calibrationButtonLastPressedTimeStamp = 0;
    calibrationButtonPressedDuration = 0;
  }

  if (calibrationButtonPressedDuration >= 5000) {
    isInCalibration = true;
    digitalWrite(13, HIGH);
  }
}

void persistBounds() {
  unsigned long now = millis();
  
  if (((calibrationWriteLastTimeStamp + now) - calibrationWriteLastTimeStamp) >= 1000) {
    int highValue = Xstick;
    
    if (Ystick > Xstick) {
      highValue = Ystick;
    }

    EEPROM.write(0, highByte(highValue));
    EEPROM.write(1, lowByte(highValue));

    calibrationWriteLastTimeStamp = now;
  }
}
