const int rowCount = 5;  // Number of row pins
const int colCount = 4;  // Number of column pins
int rowPins[rowCount] = {6, 5, 4, 3, 2};  // Matrix row pin numbers
int colPins[colCount] = {10, 9, 8, 7};  // Matrix column pin numbers

int keys[rowCount][colCount] = {
  {1,2, 3, 4},
  {5, 6, 7, 8},
  {9, 10, 11, 12},
  {13, 14,15, 16},
  {17, 18, 19, 20}
};

void setup() {
  resetColumns();  // Setup column pins
  pinMode(1, INPUT_PULLUP);
  pinMode(21, INPUT_PULLUP);
  
  pinMode(14, INPUT_PULLUP);
  pinMode(15, INPUT_PULLUP);
  pinMode(16, INPUT_PULLUP);
  pinMode(17, INPUT_PULLUP);
  pinMode(18, INPUT_PULLUP);
  
  Joystick.useManualSend(true);
}

void loop() {
  scanMatrix();  // Scan matrix rows and columns
}

// Run through matrix row and column pins
void scanMatrix() {
  Joystick.X(analogRead(9));
  Joystick.Y(1023 - analogRead(8));
  
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
          //delay(2);  // Delay to prevent phantom key presses
      }
      else {
          Joystick.button(key, false);  // Release the key
      }
    }

    digitalWriteFast(row, HIGH);  // Set the row high
  }
  
  Joystick.button(21, !digitalRead(1));
  Joystick.button(22, !digitalRead(21));
  
  Joystick.button(23, !digitalRead(14));
  Joystick.button(24, !digitalRead(15));
  Joystick.button(25, !digitalRead(16));
  Joystick.button(26, !digitalRead(17));
  Joystick.button(27, !digitalRead(18));
  
  Joystick.send_now();
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
