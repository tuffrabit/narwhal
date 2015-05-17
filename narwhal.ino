const int rowCount = 5;  // Number of row pins
const int colCount = 4;  // Number of column pins
int rowPins[rowCount] = {6, 5, 4, 3, 2};  // Matrix row pin numbers
int colPins[colCount] = {10, 9, 8, 7};  // Matrix column pin numbers

int keys[rowCount][colCount] = {
  {KEY_1, KEY_2, KEY_3, KEY_4},
  {KEY_5, KEY_TAB, KEY_Q, KEY_W},
  {KEY_E, KEY_R, KEY_CAPS_LOCK, KEY_A},
  {KEY_S, KEY_D, KEY_F, KEY_LEFT_SHIFT},
  {KEY_Z, KEY_X, KEY_C, KEY_V}
};

void setup() {
  resetColumns();  // Setup column pins
  pinMode(1, INPUT_PULLUP);
}

void loop() {
  scanMatrix();  // Scan matrix rows and columns
}

// Run through matrix row and column pins
void scanMatrix() {
  // Activate each row pin one at a time
  for (int i = 0; i < rowCount; i++) {
    int row = rowPins[i];
    
    activateRow(row);

    // Test each column one at a time
    for (int z = 0; z < colCount; z++) {
      int col = colPins[z];
      //int key = getKeyFromCombo(row, col);  // Get the keyboard int for the current row and column
      int key = keys[i][z];

      // Check the column for low voltage
      if (checkColumn(col)) {
          Keyboard.press(key);  // Press the key
          delay(2);  // Delay to prevent phantom key presses
      }
      else {
          Keyboard.release(key);  // Release the key
      }
    }

    digitalWrite(row, HIGH);  // Set the row high
  }
  
  if (!digitalRead(1)) {
    Keyboard.press(KEY_SPACE);
  }
  else {
    Keyboard.release(KEY_SPACE);
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
  digitalWrite(row, LOW);
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
