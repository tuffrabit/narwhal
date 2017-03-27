#include <EEPROM.h>

#define  STICK_X 9
#define STICK_Y 8

const int rowCount = 5;  // Number of row pins
const int colCount = 4;  // Number of column pins
const int thumbButton = 1;  // Pin # for thumb button
const int joystickButton = 21;  // Pin # for joystick button
const int dpad0 = 14;  // Pin # for D-pad 0
const int dpad1 = 15;  // Pin # for D-pad 1
const int dpad2 = 16;  // Pin # for D-pad 2
const int dpad3 = 17;  // Pin # for D-pad 3
const int dpad4 = 18;  // Pin # for D-pad 4

int rowPins[rowCount] = {6, 5, 4, 3, 2};  // Matrix row pin numbers
int colPins[colCount] = {10, 9, 8, 7};  // Matrix column pin numbers

// Holds last button transition millis value for each button/key
unsigned long debounceTimes[27] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

// Holds previous button reading (HIGH or LOW)
int previousButtonReadings[27] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

// Holds current button reading (HIGH or LOW)
int currentButtonReadings[27];
unsigned long debounceDelay = 50;  // Time, in milliseconds, to use for button state transitions
int Xstick;  // Joystick X axis value
int Ystick;  // Joystick Y axis value
int deadzone;  // Joystick deadzone size, used to calculate upper and lower bounds
int upperBound;  // Joystick deadzone upper bound
int lowerBound;  // Joystick deadzone lower bound
bool isInCalibration;  // Flag for whether or not the main loop is in joystick deadzone calibration mode or not
bool isCalibrationButtonPressed;  // Tracks joystick deadzone calibration button pressed state
unsigned long calibrationButtonLastPressedTimeStamp;
unsigned long calibrationButtonPressedDuration;
unsigned long calibrationWriteLastTimeStamp;
int calibrationDurationDelay = 8000;  // How long the calibration button must be pressed before entering joystick deadzone calibration mode

int keys[rowCount][colCount] = {
  {1, 2, 3, 4},
  {5, 6, 7, 8},
  {9, 10, 11, 12},
  {13, 14, 15, 16},
  {17, 18, 19, 20}
};

void setup() {
  resetColumns();  // Setup column pins
  pinMode(joystickButton, INPUT_PULLUP);
  pinMode(thumbButton, INPUT_PULLUP);

  pinMode(dpad0, INPUT_PULLUP);
  pinMode(dpad1, INPUT_PULLUP);
  pinMode(dpad2, INPUT_PULLUP);
  pinMode(dpad3, INPUT_PULLUP);
  pinMode(dpad4, INPUT_PULLUP);

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
        int reading = checkColumn(col);

        // Check the column for low voltage
        if (reading) {
          Joystick.button(key, true); // Press the key
          handleJoystickButtonPress(key, reading);
        }
        else {
          Joystick.button(key, false);  // Release the key
        }
      }

      digitalWriteFast(row, HIGH);  // Set the row high
    }

    handleJoystickButtonPress(21, digitalRead(thumbButton));
    handleJoystickButtonPress(22, digitalRead(joystickButton));
    handleJoystickButtonPress(23, digitalRead(dpad0));
    handleJoystickButtonPress(24, digitalRead(dpad1));
    handleJoystickButtonPress(25, digitalRead(dpad2));
    handleJoystickButtonPress(26, digitalRead(dpad3));
    handleJoystickButtonPress(27, digitalRead(dpad4));

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

// Debounce and/or press joystick button
void handleJoystickButtonPress(int key, int reading) {
  int keyIndex = key - 1;
  int lastButtonState = previousButtonReadings[keyIndex];
  int currentButtonState = currentButtonReadings[keyIndex];
  unsigned long lastDebounceTime = debounceTimes[keyIndex];

  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != currentButtonState) {
      currentButtonReadings[keyIndex] = reading;
      Joystick.button(key, !reading);
    }
  }

  debounceTimes[keyIndex] = lastDebounceTime;
  previousButtonReadings[keyIndex] = reading;
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

  if (calibrationButtonPressedDuration >= calibrationDurationDelay) {
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
