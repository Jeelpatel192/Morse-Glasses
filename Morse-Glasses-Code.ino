#include <LiquidCrystal.h>


const char CLEAR = 0;

const char DOT = 1;

const char DASH = 2;

const char alphabet[26][6] {

  { 'A', DOT, DASH, CLEAR, CLEAR, CLEAR},

  { 'B', DASH, DOT, DOT, DOT, CLEAR},

  { 'C', DASH, DOT, DASH, DOT, CLEAR},

  { 'D', DASH, DOT, DOT, CLEAR, CLEAR},

  { 'E', DOT, CLEAR, CLEAR, CLEAR, CLEAR},

  { 'F', DOT, DOT, DASH, DOT, CLEAR},

  { 'G', DASH, DASH, DOT, CLEAR, CLEAR},

  { 'H', DOT, DOT, DOT, DOT, CLEAR},

  { 'I', DOT, DOT, CLEAR, CLEAR, CLEAR},

  { 'J', DOT, DASH, DASH, DASH, CLEAR},

  { 'K', DASH, DOT, DASH, CLEAR, CLEAR},

  { 'L', DOT, DASH, DOT, DOT, CLEAR},

  { 'M', DASH, DASH, CLEAR, CLEAR, CLEAR},

  { 'N', DASH, DOT, CLEAR, CLEAR, CLEAR},

  { 'O', DASH, DASH, DASH, CLEAR, CLEAR},

  { 'P', DOT, DASH, DASH, DOT, CLEAR},

  { 'Q', DASH, DASH, DOT, DASH, CLEAR},

  { 'R', DOT, DASH, DOT, CLEAR, CLEAR},

  { 'S', DOT, DOT, DOT, CLEAR, CLEAR},

  { 'T', DASH, CLEAR, CLEAR, CLEAR, CLEAR},

  { 'U', DOT, DOT, DASH, CLEAR, CLEAR},

  { 'V', DOT, DOT, DOT, DASH, CLEAR},

  { 'W', DOT, DASH, DASH, CLEAR, CLEAR},

  { 'X', DASH, DOT, DOT, DASH, CLEAR},

  { 'Y', DASH, DOT, DASH, DASH, CLEAR},

  { 'Z', DASH, DASH, DOT, DOT, CLEAR}

};


const unsigned long dotDuration = 1000; // duration [ms] of a dot; basic time unit

const unsigned long tolerance = 500; // duration that dash, dot or gap may deviate from the ideal length in order to still be recognized

const unsigned long dashDuration = 3 * dotDuration;

const unsigned long shortGap = dotDuration;

const unsigned long mediumGap = 3 * dotDuration;

const unsigned long longGap = 7 * dotDuration;


// current state of the button

enum State { 

  UP = 0, 

  DOWN = 1 } state;


// time index of the last button state change (from up to down or down to up)

unsigned long lastChange;


// duration of the last button press

unsigned long downDuration;


// dash-dot-sequence of the current character

char character[5];


// index of the next dot/dash in the current character

int characterIndex;


// the next thing to do

enum Action {

  START = 0,

  READ_DASHDOT = 1,

  READ_CHARACTER = 2,

  READ_WORD = 3

} action;


// pin to which the button is connected

int buttonPin = 6;

// pin to which the buzzer is connected

int buzzerPin = 9;


LiquidCrystal lcd(12, 11, 5, 4, 3, 2);


void setup() {

  Serial.begin(9600);


  lcd.begin(16, 2);

  

  pinMode(buttonPin, INPUT_PULLUP);


  state = UP;

  characterIndex = 0;

  downDuration = 0;

  lastChange = 0;

  action = START;

}


// reset the dash-dot-sequence of the current character

void clearCharacter() {

  characterIndex = 0;

  for (int i = 0; i < 5; ++i) {

    character[i] = CLEAR;

  }

}


// determine the next dash or dot in the dash-dot sequence of the current character

void readDashDot() {

  Serial.print("Down duration was: ");

  Serial.println(downDuration);

  

  if (downDuration >= dashDuration - tolerance && downDuration < dashDuration + tolerance) {

    character[characterIndex] = DASH;

    Serial.println("DASH");

    characterIndex++;

  } else if (downDuration >= dotDuration - tolerance && downDuration < dotDuration + tolerance) {

    character[characterIndex] = DOT;

    Serial.println("DOT");

    characterIndex++;

  } else {

    Serial.println("Down duration unrecognized");

  }

}


// interpret the current dash-dot-sequence as a character

char readCharacter() {

  bool found;

  for (int i = 0; i < 26; ++i) {

    found = true;

    for (int j = 0; found && j < 5; ++j) {

      if (character[j] != alphabet[i][j + 1]) {

        found = false;

      }

    }

    if (found) {

      return alphabet[i][0];

    }

  }

  return 0;

}


void loop() {

  State newState = digitalRead(buttonPin) ? UP : DOWN;


  // if the button is pressed, play a tone

  if (newState == DOWN) {

    tone(buzzerPin, 800);

  } else {

    noTone(buzzerPin);

  }


  if (newState == state) {

    if (newState == UP) {

      // calculate the the time the button has been in UP state

      unsigned long upDuration = (millis() - lastChange);

     

      if (action == READ_DASHDOT && upDuration >= shortGap - tolerance && upDuration < shortGap + tolerance) {

        readDashDot();

        action = READ_CHARACTER;

      } else if (action == READ_CHARACTER && upDuration >= mediumGap - tolerance && upDuration < mediumGap + tolerance) {

        char c = readCharacter();

        if (c != 0) {

          // a valid character has been recognized

          Serial.print("Read character: ");

          Serial.println(c);

          lcd.print(c);

        } else {

          Serial.println("Unrecognized character");

        }

        clearCharacter();

        action = READ_WORD;

      } else if (action == READ_WORD && upDuration >= longGap - tolerance && upDuration < longGap + tolerance) {

        Serial.println("Read next word");

        lcd.print(' ');

        action = READ_DASHDOT;

      }

    } else {

      downDuration = (millis() - lastChange);

    }

  } else {

    if (state == UP && newState == DOWN) {

      downDuration = 0;

    }

    lastChange = millis();

    state = newState;

    action = READ_DASHDOT;

  }

}
