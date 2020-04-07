/*Program name: ArduRUNo!
  Version: 3.0
  Developer: Jaryn D. Giampaoli
  Description of program: A fun little game ran on an LCD w/ a joystick module

  FINAL Project for CPSC-42-1191

  +----------------------------------------------------------------------------------------------+
  |NOTES: Basic code (game graphics) courtesy of Mohammed Magdy from the Arduino Project Gallery.|
  |       Buzzer, joystick functionality, and LEDs were added to the basic code by the           |
  |       developer of this specific program.                                                    |
  +----------------------------------------------------------------------------------------------+
*/

//INCLUDE NECESSARY LIBRARIES
#include <LiquidCrystal.h>
#include "pitches.h"

//DECLARE LCD
LiquidCrystal lcd(9, 8, 6, 5, 4, 3);

//DEFINE LCD GAME PINS
#define PIN_AUTOPLAY 1
#define PIN_READWRITE 10
#define PIN_CONTRAST 12

//DECLARE LED COLORS (BLUE, RED, YELLOW & GREEN) IN AN ARRAY
const int ledColorsByPin [4] = {A5, 10, 11, 12};

//DECLARE DELAY FOR HIGH SCORE LED BLINKING
static const unsigned int delayOfHighScoreLED = 125; //In milliseconds

//DECLARE DELAY FOR GAME OVER LED BLINKING
static const unsigned int delayOfGameOverLED = 225; //In milliseconds

//DECLARE GAME OVER & GAME START LED VARIABLES
int switchValue;
static unsigned int gameStartLEDtimesBlinked = 0;
static unsigned int gameOverLEDgameStartLEDtimesBlinked = 0;

//DEFINE JOYSTICK PINS
#define JOY_PRESS 2
#define joyX A0
#define joyY A1

//DECLARE JOYSTICK VARIABLES
static int xPosition = 0;
static int yPosition = 0;
static int mapX = 0;
static int mapY = 0;
//DEFINE JOYSTICK PRESSED
static bool joystickPressed = false;

/**FOR THE INTERRUPT ON JOYSTICK PRESS*/
void joystickPush() {
  joystickPressed = true;
}//End joystickPush

//DEFINE TERRAIN
#define SPRITE_RUN1 1
#define SPRITE_RUN2 2
#define SPRITE_JUMP 3
#define SPRITE_JUMP_UPPER '.'         // Use the '.' character for the head
#define SPRITE_JUMP_LOWER 4
#define SPRITE_TERRAIN_EMPTY ' '      // Use the ' ' character
#define SPRITE_TERRAIN_SOLID 5
#define SPRITE_TERRAIN_SOLID_RIGHT 6
#define SPRITE_TERRAIN_SOLID_LEFT 7

#define TERRAIN_WIDTH 16
#define TERRAIN_EMPTY 0
#define TERRAIN_LOWER_BLOCK 1
#define TERRAIN_UPPER_BLOCK 2

//DEFINE VARIABLES FOR RUNNING MAN (CHARACTER IN GAME)
#define RUNNING_MAN_HORIZONTAL_POSITION 1    // Horizontal position of RUNNING_MAN on screen

#define RUNNING_MAN_POSITION_OFF 0          // RUNNING_MAN is invisible
#define RUNNING_MAN_POSITION_RUN_LOWER_1 1  // RUNNING_MAN is running on lower row (pose 1)
#define RUNNING_MAN_POSITION_RUN_LOWER_2 2  //                                     (pose 2)

#define RUNNING_MAN_POSITION_JUMP_1 3       // Starting a jump
#define RUNNING_MAN_POSITION_JUMP_2 4       // Half-way up
#define RUNNING_MAN_POSITION_JUMP_3 5       // Jump is on upper row
#define RUNNING_MAN_POSITION_JUMP_4 6       // Jump is on upper row
#define RUNNING_MAN_POSITION_JUMP_5 7       // Jump is on upper row
#define RUNNING_MAN_POSITION_JUMP_6 8       // Jump is on upper row
#define RUNNING_MAN_POSITION_JUMP_7 9       // Half-way down
#define RUNNING_MAN_POSITION_JUMP_8 10      // About to land

#define RUNNING_MAN_POSITION_RUN_UPPER_1 11 // RUNNING_MAN is running on upper row (pose 1)
#define RUNNING_MAN_POSITION_RUN_UPPER_2 12 //                                     (pose 2)

//DECLARE GLOBAL VARIABLES NEEDED WHILE IN GAME PLAY
static byte RUNNING_MANPos = RUNNING_MAN_POSITION_RUN_LOWER_1;
static byte newTerrainType = TERRAIN_EMPTY;
static byte newTerrainDuration = 1;
static char terrainUpper[TERRAIN_WIDTH + 1];
static char terrainLower[TERRAIN_WIDTH + 1];
static bool playing = false; //Controls when the game start and/or ends
static bool blink = false; //Controls the LCD blinking of the title screen
static unsigned int distance = 0; //Restart at 0 each game

//DECLARE GLOBAL BUZZER VARIABLE
const static int buzzer = 7; //Piezo buzzer on PIN #7

//DECLARE GLOBAL HIGH SCORE VARIABLE
static unsigned int gameLevelIndicator = 0; //Keeps track of the current score of the player
static unsigned int gameHighScore = 0; //Stores the highest score made by a player in the game
static boolean highScoreAchieved = false; //What to do if a high score is achieved

//VARIABLES FOR THE GAME START SONG
const int gameStartSong[] = {
  NOTE_CS5, NOTE_CS5, NOTE_CS5, NOTE_D5, NOTE_E5, NOTE_A5, NOTE_AS5
};

// note durations: 4 = quarter note, 8 = eighth note, etc.:
const int noteDurations4Song1[] = {
  9, 9, 9, 8, 5, 6, 5
};

/**GAME START BUZZER TONE*/
void gameStartBuzzer() {
  for (int thisNote = 0; thisNote < 7; thisNote++) {

    // to calculate the note duration, take one second divided by the note type.
    //e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
    int noteDuration = 1000 / noteDurations4Song1[thisNote];
    tone(7, gameStartSong[thisNote], noteDuration);

    // to distinguish the notes, set a minimum time between them.
    // the note's duration + 30% seems to work well:
    int pauseBetweenNotes = noteDuration * 1.30;
    delay(pauseBetweenNotes);

    //Stop the tone playing:
    noTone(7);
  }
}//End gameStartBuzzer

//VARIABLES FOR THE GAME OVER SONG
const int gameOverSong[] = {
  NOTE_GS6, NOTE_FS6, NOTE_DS6, NOTE_AS5, NOTE_E5, NOTE_E6, NOTE_A5
};
const int noteDurations4Song2[] = {
  10, 10, 10, 10, 10, 8, 4, 8
};//note durations: 4 = quarter note, 8 = eighth note, etc.:

/**GAME OVER BUZZER TONE*/
void gameOverBuzzer() {
  for (int thisNote = 0; thisNote < 7; thisNote++) {

    // to calculate the note duration, take one second divided by the note type.
    //e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
    int noteDuration = 1000 / noteDurations4Song2[thisNote];
    tone(7, gameOverSong[thisNote], noteDuration);

    // to distinguish the notes, set a minimum time between them.
    // the note's duration + 30% seems to work well:
    int pauseBetweenNotes = noteDuration * 1.30;
    delay(pauseBetweenNotes);

    //Stop the tone playing:
    noTone(7);
  }
}//End gameOverBuzzer

//VARIABLES FOR THE GAME HIGH SCORE CHIME
const int gameHighScoreSong[] = {
  NOTE_B5, NOTE_B3, NOTE_B5, NOTE_B3, NOTE_B6, NOTE_B7, NOTE_C8
};
const int noteDurations4Song3[] = {
  8, 7, 8, 7, 6, 3, 5
};//note durations: 4 = quarter note, 8 = eighth note, etc.:

/**GAME HIGH SCORE BUZZER TONE*/
void gameHighScoreBuzzer() {
  for (int thisNote = 0; thisNote < 7; thisNote++) {

    // to calculate the note duration, take one second divided by the note type.
    //e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
    int noteDuration = 1000 / noteDurations4Song3[thisNote];
    tone(7, gameHighScoreSong[thisNote], noteDuration);

    // to distinguish the notes, set a minimum time between them.
    // the note's duration + 30% seems to work well:
    int pauseBetweenNotes = noteDuration * 1.30;
    delay(pauseBetweenNotes);

    //Stop the tone playing:
    noTone(7);
  }
}//End gameHighScoreBuzzer

//DECLARE GLOBAL GAME LEVEL INCREASE TONE VARIABLES
static unsigned long previousMillis = 0;

static unsigned int levelOneNotesOutputted = 0;
static unsigned int levelTwoNotesOutputted = 0;
static unsigned int levelThreeNotesOutputted = 0;
static unsigned int levelFourNotesOutputted = 0;
static unsigned int levelFiveNotesOutputted = 0;
static unsigned int levelMasterNotesOutputted = 0;

boolean outputTone = false;
static unsigned long currentMillis;
static unsigned long pauseBetweenNotes;

//VARIABLES FOR GENERAL LEVEL UP MELEODY
const int levelUpChime[] = {
  NOTE_A6, NOTE_F7, NOTE_C8
};
const int levelUpChimeDuration[] = {
  80, 90, 100
};//note durations: 4 = quarter note, 8 = eighth note, etc.:

unsigned int numberOfNotesDivision4GenLevels = sizeof(levelUpChime) / sizeof(int);
static unsigned int levelOneNumberOfNotes = numberOfNotesDivision4GenLevels;
static unsigned int levelTwoNumberOfNotes = numberOfNotesDivision4GenLevels;
static unsigned int levelThreeNumberOfNotes = numberOfNotesDivision4GenLevels;
static unsigned int levelFourNumberOfNotes = numberOfNotesDivision4GenLevels;
static unsigned int levelFiveNumberOfNotes = numberOfNotesDivision4GenLevels;

/**LEVEL 1 TONE SETTINGS*/
void level1Tone() {
  currentMillis = millis();
  unsigned long noteDuration = 1000 / levelUpChimeDuration[levelOneNotesOutputted];
  pauseBetweenNotes = noteDuration * 0.3;

  if (levelOneNotesOutputted < levelOneNumberOfNotes) {
    if (outputTone) {
      if (currentMillis - previousMillis >= noteDuration) {
        previousMillis = currentMillis;
        noTone(buzzer);
        outputTone = false;
        levelOneNotesOutputted++;
      }
    }

    else {
      if (currentMillis - previousMillis >= pauseBetweenNotes) {
        previousMillis = currentMillis;
        if (levelUpChime[levelOneNotesOutputted] == 0) {
          noTone(buzzer);
        } else {
          tone(buzzer, levelUpChime[levelOneNotesOutputted]);
        }
        outputTone = true;
      }
    }
  }
}

/**LEVEL 2 TONE SETTINGS*/
void level2Tone() {
  currentMillis = millis();
  unsigned long noteDuration = 1000 / levelUpChimeDuration[levelTwoNotesOutputted];
  pauseBetweenNotes = noteDuration * 0.3;

  if (levelTwoNotesOutputted < levelTwoNumberOfNotes) {
    if (outputTone) {
      if (currentMillis - previousMillis >= noteDuration) {
        previousMillis = currentMillis;
        noTone(buzzer);
        outputTone = false;
        levelTwoNotesOutputted++;
      }
    }

    else {
      if (currentMillis - previousMillis >= pauseBetweenNotes) {
        previousMillis = currentMillis;
        if (levelUpChime[levelTwoNotesOutputted] == 0) {
          noTone(buzzer);
        } else {
          tone(buzzer, levelUpChime[levelTwoNotesOutputted]);
        }
        outputTone = true;
      }
    }
  }
}

/**LEVEL 3 TONE SETTINGS*/
void level3Tone() {
  currentMillis = millis();
  unsigned long noteDuration = 1000 / levelUpChimeDuration[levelThreeNotesOutputted];
  pauseBetweenNotes = noteDuration * 0.3;

  if (levelThreeNotesOutputted < levelThreeNumberOfNotes) {
    if (outputTone) {
      if (currentMillis - previousMillis >= noteDuration) {
        previousMillis = currentMillis;
        noTone(buzzer);
        outputTone = false;
        levelThreeNotesOutputted++;
      }
    }

    else {
      if (currentMillis - previousMillis >= pauseBetweenNotes) {
        previousMillis = currentMillis;
        if (levelUpChime[levelThreeNotesOutputted] == 0) {
          noTone(buzzer);
        } else {
          tone(buzzer, levelUpChime[levelThreeNotesOutputted]);
        }
        outputTone = true;
      }
    }
  }
}

/**LEVEL 4 TONE SETTINGS*/
void level4Tone() {
  currentMillis = millis();
  unsigned long noteDuration = 1000 / levelUpChimeDuration[levelFourNotesOutputted];
  pauseBetweenNotes = noteDuration * 0.3;

  if (levelFourNotesOutputted < levelFourNumberOfNotes) {
    if (outputTone) {
      if (currentMillis - previousMillis >= noteDuration) {
        previousMillis = currentMillis;
        noTone(buzzer);
        outputTone = false;
        levelFourNotesOutputted++;
      }
    }

    else {
      if (currentMillis - previousMillis >= pauseBetweenNotes) {
        previousMillis = currentMillis;
        if (levelUpChime[levelFourNotesOutputted] == 0) {
          noTone(buzzer);
        } else {
          tone(buzzer, levelUpChime[levelFourNotesOutputted]);
        }
        outputTone = true;
      }
    }
  }
}

/**LEVEL 5 TONE SETTINGS*/
void level5Tone() {
  currentMillis = millis();
  unsigned long noteDuration = 1000 / levelUpChimeDuration[levelFiveNotesOutputted];
  pauseBetweenNotes = noteDuration * 0.3;

  if (levelFiveNotesOutputted < levelFiveNumberOfNotes) {
    if (outputTone) {
      if (currentMillis - previousMillis >= noteDuration) {
        previousMillis = currentMillis;
        noTone(buzzer);
        outputTone = false;
        levelFiveNotesOutputted++;
      }
    }

    else {
      if (currentMillis - previousMillis >= pauseBetweenNotes) {
        previousMillis = currentMillis;
        if (levelUpChime[levelFiveNotesOutputted] == 0) {
          noTone(buzzer);
        } else {
          tone(buzzer, levelUpChime[levelFiveNotesOutputted]);
        }
        outputTone = true;
      }
    }
  }
}

//MASTER LEVEL MELODY
const int theFinalCountdownMelody[] = {
  NOTE_B4, NOTE_A4, NOTE_B4, NOTE_E4,
  0,
  NOTE_C5, NOTE_B4, NOTE_C5, NOTE_B4, NOTE_A4,
  0,
  NOTE_C5, NOTE_B4, NOTE_C5, NOTE_E4, NOTE_FS4,
  0,
  NOTE_A4, NOTE_G4, NOTE_A4, NOTE_G4, NOTE_FS4, NOTE_A4, NOTE_G4,
  0,
  NOTE_B4, NOTE_A4, NOTE_B4, NOTE_E4,
  0,
  NOTE_C5, NOTE_B4, NOTE_C5, NOTE_B4, NOTE_A4,
  0,
  NOTE_B4, NOTE_A4, NOTE_B4, NOTE_E4, NOTE_FS4,
  0,
  NOTE_A4, NOTE_G4, NOTE_A4, NOTE_G4, NOTE_FS4, NOTE_A4,
  NOTE_G4, NOTE_FS4, NOTE_G4, NOTE_A4, NOTE_G4, NOTE_A4, NOTE_B4, NOTE_A4, NOTE_G4, NOTE_FS4, NOTE_E4, NOTE_B4,
  NOTE_C5, NOTE_G4,
  0,
  NOTE_B4, NOTE_A4, NOTE_B4, NOTE_E4,
  0,
  NOTE_C5, NOTE_B4, NOTE_C5, NOTE_B4, NOTE_A4,
  0,
  NOTE_C5, NOTE_B4, NOTE_C5, NOTE_E4, NOTE_FS4,
  0,
  NOTE_A4, NOTE_G4, NOTE_A4, NOTE_G4, NOTE_FS4, NOTE_A4,
  NOTE_G4,
  0,
  NOTE_B4, NOTE_A4, NOTE_B4, NOTE_E4,
  0,
  NOTE_C5, NOTE_B4, NOTE_C5, NOTE_B4, NOTE_A4,
  0,
  NOTE_C5,
  NOTE_B4, NOTE_C5, NOTE_E4, NOTE_FS4,
  0,
  NOTE_A4, NOTE_G4, NOTE_A4, NOTE_G4, NOTE_FS4, NOTE_A4, NOTE_G4
};

const long theFinalCountdownDuration[] = {
  16, 16, 4, 4, 2, 16, 16, 8, 8, 4, 2, 16, 16, 4, 4, 4, 4, 16, 16, 8, 8, 8, 8,  4, 8, 16, 16, 4, 4, 2, 16, 16, 8, 8, 4, 2, 16, 16, 4, 4, 4, 6, 16, 16, 8, 8, 8, 8,  2, 16, 16, 3, 16, 16, 8, 8, 8, 8, 4, 8, 1, 4, 8, 16, 16, 4, 4, 2, 16, 16, 8, 8, 8,  2, 16, 16, 4, 4, 4, 8, 16, 16, 8, 8, 8, 8, 4, 6, 16, 16, 4, 4, 2, 16, 16, 8, 8, 4, 2, 16, 16, 4, 4,  4, 6, 16, 16, 8, 8, 8, 8, 4
};//note durations: 4 = quarter note, 8 = eighth note, etc.:

unsigned int numberOfNotesDivision4MasterLevel = sizeof(theFinalCountdownMelody) / sizeof(int);
static int unsigned levelMasterNumberOfNotes = numberOfNotesDivision4MasterLevel;

/**MASTER LEVEL TONE SETTINGS*/
void levelMasterTone() {
  currentMillis = millis();
  unsigned long noteDuration = 1000 / theFinalCountdownDuration[levelMasterNotesOutputted];
  pauseBetweenNotes = noteDuration * 0.3;

  if (levelMasterNotesOutputted < levelMasterNumberOfNotes) {
    if (outputTone) {
      if (currentMillis - previousMillis >= noteDuration) {
        previousMillis = currentMillis;
        noTone(buzzer);
        outputTone = false;
        levelMasterNotesOutputted++;
      }
    }

    else {
      if (currentMillis - previousMillis >= pauseBetweenNotes / 4) {
        previousMillis = currentMillis;
        if (theFinalCountdownMelody[levelMasterNotesOutputted] == 0) {
          noTone(buzzer);
        } else {
          tone(buzzer, theFinalCountdownMelody[levelMasterNotesOutputted]);
        }
        outputTone = true;
      }
    }
  }
}

/**GAME DIFFICULTY LEVEL (SPEED) SETTINGS*/
void gameSpeed() {
  if (gameLevelIndicator < 50) {
    delay(105);//EASY
  } else if ((50 < gameLevelIndicator) && (gameLevelIndicator < 100)) {
    level1Tone();
    delay(100);//MEDIUM
  } else if ((100 < gameLevelIndicator) && (gameLevelIndicator < 150)) {
    level2Tone();
    delay(95);//HARD
  } else if ((150 < gameLevelIndicator) && (gameLevelIndicator < 200)) {
    level3Tone();
    delay(90);//EXPERT
  } else if ((200 < gameLevelIndicator) && (gameLevelIndicator < 250)) {
    level4Tone();
    delay(80);//MASTER
  } else if ((250 < gameLevelIndicator) && (gameLevelIndicator < 300)) {
    level5Tone();
    delay(75);//PROFESSIONAL
  } else if (300 < gameLevelIndicator) {
    levelMasterTone();
    delay(70);//IMPOSSIBLE
  } else {
    delay(105);//Give program time to shift to new gameSpeed
  }
}//End gameSpeed

/**INITIALIZE GAME GRAPHICS*/
void initializeGraphics() {
  static byte graphics[] = {
    // Running man (POSE 1)
    B01100,
    B01100,
    B00000,
    B11100,
    B01110,
    B01100,
    B11010,
    B10011,

    // Running man (POSE 2)
    B01100,
    B01100,
    B00000,
    B01100,
    B01100,
    B01100,
    B01100,
    B01100,

    // Running man (Jumping up)
    B01100,
    B01100,
    B00000,
    B11110,
    B01101,
    B11111,
    B10000,
    B00000,

    // Running man (Jumping down)
    B11110,
    B01101,
    B11111,
    B10000,
    B00000,
    B00000,
    B00000,
    B00000,

    // Ground (Center of terrain obstacle)
    B11111,
    B11111,
    B11011,
    B10101,
    B10101,
    B11011,
    B11111,
    B11111,

    // Ground left
    B00011,
    B00011,
    B00011,
    B00011,
    B00011,
    B00011,
    B00011,
    B00011,

    // Ground right
    B11000,
    B11000,
    B11000,
    B11000,
    B11000,
    B11000,
    B11000,
    B11000,

    0b00100,
    0b00101,
    0b10101,
    0b10101,
    0b10111,
    0b11100,
    0b00100,
    0b00000
  };

  /*Skip using character 0 to allow the lcd.print() function to
     be used to quickly draw multiple characters */
  int i;

  for (i = 0; i < 8; ++i) {
    lcd.createChar(i + 1, & graphics[i * 8]);
  }
  for (i = 0; i < TERRAIN_WIDTH; ++i) {
    terrainUpper[i] = SPRITE_TERRAIN_EMPTY;
    terrainLower[i] = SPRITE_TERRAIN_EMPTY;
  }
}//End of initializeGraphics

// Slide the terrain to the left in half-character increments
void advanceTerrain(char* terrain, byte newTerrain) {
  for (int i = 0; i < TERRAIN_WIDTH; ++i) {
    char current = terrain[i];
    char next = (i == TERRAIN_WIDTH - 1) ? newTerrain : terrain[i + 1];
    switch (current) {
      case SPRITE_TERRAIN_EMPTY:
        terrain[i] = (next == SPRITE_TERRAIN_SOLID) ? SPRITE_TERRAIN_SOLID_RIGHT : SPRITE_TERRAIN_EMPTY;
        break;
      case SPRITE_TERRAIN_SOLID:
        terrain[i] = (next == SPRITE_TERRAIN_EMPTY) ? SPRITE_TERRAIN_SOLID_LEFT : SPRITE_TERRAIN_SOLID;
        break;
      case SPRITE_TERRAIN_SOLID_RIGHT:
        terrain[i] = SPRITE_TERRAIN_SOLID;
        break;
      case SPRITE_TERRAIN_SOLID_LEFT:
        terrain[i] = SPRITE_TERRAIN_EMPTY;
        break;
    }
  }
}//End of advanceTerrain

bool drawRUNNING_MAN(byte position, char* terrainUpper, char* terrainLower, unsigned int inGameScore) {
  bool collide = false;
  char upperSave = terrainUpper[RUNNING_MAN_HORIZONTAL_POSITION];
  char lowerSave = terrainLower[RUNNING_MAN_HORIZONTAL_POSITION];
  byte upper, lower;
  switch (position) {
    case RUNNING_MAN_POSITION_OFF:
      upper = lower = SPRITE_TERRAIN_EMPTY;
      break;
    case RUNNING_MAN_POSITION_RUN_LOWER_1:
      upper = SPRITE_TERRAIN_EMPTY;
      lower = SPRITE_RUN1;
      break;
    case RUNNING_MAN_POSITION_RUN_LOWER_2:
      upper = SPRITE_TERRAIN_EMPTY;
      lower = SPRITE_RUN2;
      break;
    case RUNNING_MAN_POSITION_JUMP_1:
    case RUNNING_MAN_POSITION_JUMP_8:
      upper = SPRITE_TERRAIN_EMPTY;
      lower = SPRITE_JUMP;
      break;
    case RUNNING_MAN_POSITION_JUMP_2:
    case RUNNING_MAN_POSITION_JUMP_7:
      upper = SPRITE_JUMP_UPPER;
      lower = SPRITE_JUMP_LOWER;
      break;
    case RUNNING_MAN_POSITION_JUMP_3:
    case RUNNING_MAN_POSITION_JUMP_4:
    case RUNNING_MAN_POSITION_JUMP_5:
    case RUNNING_MAN_POSITION_JUMP_6:
      upper = SPRITE_JUMP;
      lower = SPRITE_TERRAIN_EMPTY;
      break;
    case RUNNING_MAN_POSITION_RUN_UPPER_1:
      upper = SPRITE_RUN1;
      lower = SPRITE_TERRAIN_EMPTY;
      break;
    case RUNNING_MAN_POSITION_RUN_UPPER_2:
      upper = SPRITE_RUN2;
      lower = SPRITE_TERRAIN_EMPTY;
      break;
  }
  if (upper != ' ') {
    terrainUpper[RUNNING_MAN_HORIZONTAL_POSITION] = upper;
    collide = (upperSave == SPRITE_TERRAIN_EMPTY) ? false : true;
  }
  if (lower != ' ') {
    terrainLower[RUNNING_MAN_HORIZONTAL_POSITION] = lower;
    collide |= (lowerSave == SPRITE_TERRAIN_EMPTY) ? false : true;
  }

  //Convert the game score into proer bytes for proper display on LCD
  byte digits = (inGameScore > 9999) ? 5 : (inGameScore > 999) ? 4 : (inGameScore > 99) ? 3 : (inGameScore > 9) ? 2 : 1;

  // Draw the scene
  terrainUpper[TERRAIN_WIDTH] = '\0';
  terrainLower[TERRAIN_WIDTH] = '\0';
  char temp = terrainUpper[16 - digits];
  terrainUpper[16 - digits] = '\0';
  lcd.setCursor(0, 0);
  lcd.print(terrainUpper);
  terrainUpper[16 - digits] = temp;
  lcd.setCursor(0, 1);
  lcd.print(terrainLower);

  lcd.setCursor(16 - digits, 0);
  lcd.print(inGameScore);

  /*Keep track of the player's score to indicate
    when it's time to increase the current level
    of the game*/
  gameLevelIndicator = inGameScore;

  /*Overwrites previous game high score if
    player's score has broken the record*/
  if (inGameScore > gameHighScore) {
    gameHighScore = inGameScore;
    highScoreAchieved = true;
  }

  terrainUpper[RUNNING_MAN_HORIZONTAL_POSITION] = upperSave;
  terrainLower[RUNNING_MAN_HORIZONTAL_POSITION] = lowerSave;
  return collide;
}//End of boolean drawRUNNING_MAN

void setup() {

  //Declare Game START LEDs
  pinMode(ledColorsByPin[1], OUTPUT); //red
  pinMode(ledColorsByPin[2], OUTPUT); //yellow
  pinMode(ledColorsByPin[3], OUTPUT); //green

  //Declare Game High Score LED (blue)
  pinMode(ledColorsByPin[0], OUTPUT);

  //LCD Display
  pinMode(PIN_READWRITE, OUTPUT);
  digitalWrite(PIN_READWRITE, LOW);

  pinMode(PIN_CONTRAST, OUTPUT);
  digitalWrite(PIN_CONTRAST, LOW);

  //Activate joystick functionality
  pinMode(JOY_PRESS, INPUT_PULLUP);
  digitalWrite(JOY_PRESS, HIGH);

  pinMode(joyX, INPUT);
  pinMode(joyY, INPUT);

  //Interrupt for joystick press
  attachInterrupt(0/*JOY_PRESS*/, joystickPush, FALLING);

  //Have the terrain move while playing
  pinMode(PIN_AUTOPLAY, OUTPUT);
  digitalWrite(PIN_AUTOPLAY, HIGH);

  //Game graphics & LCD display
  joystickPressed = false;
  playing = false;
  initializeGraphics();
  lcd.begin(16, 2);
}//End setup

void loop() {
  //Gather joystick positions
  xPosition = analogRead(joyX);
  yPosition = analogRead(joyY);

  if (!playing) {
    drawRUNNING_MAN((blink) ? RUNNING_MAN_POSITION_OFF : RUNNING_MANPos, terrainUpper, terrainLower, distance >> 3);
    if (blink) {
      lcd.setCursor(0, 0);
      lcd.print("ArduRUNo! - v3.0");
      lcd.setCursor(0, 1);
      lcd.print("Press to play... ");//Display instructions to play game
    }

    delay(900);
    blink = !blink;

    if (joystickPressed) {
      levelOneNotesOutputted = 0;
      levelTwoNotesOutputted = 0;
      levelThreeNotesOutputted = 0;
      levelFourNotesOutputted = 0;
      levelFiveNotesOutputted = 0;
      levelMasterNotesOutputted;
      gameStartBuzzer();
      
      /**3 SECOND LED STOPLIGHT COUNTDOWN FOR GAME START**/
      lcd.setCursor(0, 0);
      lcd.print("   GET READY!   ");
      
      if ((switchValue == LOW) && (gameStartLEDtimesBlinked <= 3)) {
        //On your marks!
        digitalWrite(ledColorsByPin[1], HIGH);//Turn the RED LED on
        lcd.setCursor(0, 1);
        lcd.print("     In 3...    ");//Display instructions to play game
        tone(7, 1109, 300);
        delay(1000);// Keep LED on for 1 second

        digitalWrite(ledColorsByPin[1], LOW);   // turn the LED off
        delay(300);              // wait slightly to transition

        //Get set!
        digitalWrite(ledColorsByPin[2], HIGH);//Turn the YELLOW LED on
        lcd.setCursor(0, 1);
        lcd.print("     In 2...    ");//Display instructions to play game
        tone(7, 2093, 300);
        delay(1000);//Keep LED on for 1 second

        digitalWrite(ledColorsByPin[2], LOW);   // turn the LED off
        delay(300);              // wait slightly to transition

        //GO!
        digitalWrite(ledColorsByPin[3], HIGH);   //Turn the GREEN LED on
        lcd.setCursor(0, 1);
        lcd.print("     In 1...    ");//Display instructions to play game
        tone(7, 3136, 800);
        delay(1000);              // wait for a second
        digitalWrite(ledColorsByPin[3], LOW);   // turn the LED off
        delay(300);              // wait slightly to transition
        gameStartLEDtimesBlinked++;
      }

      initializeGraphics();
      RUNNING_MANPos = RUNNING_MAN_POSITION_RUN_LOWER_1;
      noTone(buzzer);
      playing = true;
      joystickPressed = false;
      gameStartLEDtimesBlinked = 0;
      distance = 0;
      levelOneNotesOutputted = 0;
      levelTwoNotesOutputted = 0;
      levelThreeNotesOutputted = 0;
      levelFourNotesOutputted = 0;
      levelFiveNotesOutputted = 0;
      levelMasterNotesOutputted = 0;
    }
    return;
  }

  // Shift the terrain to the left
  advanceTerrain(terrainLower, newTerrainType == TERRAIN_LOWER_BLOCK ? SPRITE_TERRAIN_SOLID : SPRITE_TERRAIN_EMPTY);
  advanceTerrain(terrainUpper, newTerrainType == TERRAIN_UPPER_BLOCK ? SPRITE_TERRAIN_SOLID : SPRITE_TERRAIN_EMPTY);

  // Make new terrain to enter on the right
  if (--newTerrainDuration == 0) {
    if (newTerrainType == TERRAIN_EMPTY) {
      newTerrainType = (random(5) == 0) ? TERRAIN_UPPER_BLOCK : TERRAIN_LOWER_BLOCK;
      newTerrainDuration = 2 + random(10);
    } else {
      newTerrainType = TERRAIN_EMPTY;
      newTerrainDuration = 15 + random(15);
    }
  }

//When player swipes the joystick up, make the character jump
  if (xPosition <= 20 || yPosition <= 300) {
    if (RUNNING_MANPos <= RUNNING_MAN_POSITION_RUN_LOWER_2) RUNNING_MANPos = RUNNING_MAN_POSITION_JUMP_1;
    joystickPressed = false;
  }

//GAME OVER
  if (drawRUNNING_MAN(RUNNING_MANPos, terrainUpper, terrainLower, distance >> 3)) {
    playing = false; // The RUNNING_MAN collided with something. Game over.
    lcd.setCursor(0, 0);
    lcd.println("You hit terrain!");
    lcd.setCursor(0, 1);
    lcd.print("GAME OVER!      ");
    gameOverBuzzer();

    /**FLAHS RED LED THREE TIMES TO INDICATE GAME OVER**/
    if ((switchValue == LOW) && (gameOverLEDgameStartLEDtimesBlinked <= 3)) {
      digitalWrite(ledColorsByPin[1], HIGH);   // turn the LED on
      delay(delayOfGameOverLED);
      digitalWrite(ledColorsByPin[1], LOW);   // turn the LED off
      delay(delayOfGameOverLED);
      digitalWrite(ledColorsByPin[1], HIGH);   // turn the LED on
      delay(delayOfGameOverLED);
      digitalWrite(ledColorsByPin[1], LOW);   // turn the LED off
      delay(delayOfGameOverLED);
      digitalWrite(ledColorsByPin[1], HIGH);   // turn the LED on
      delay(delayOfGameOverLED);
      digitalWrite(ledColorsByPin[1], LOW);   // turn the LED off
      gameStartLEDtimesBlinked++;

    }

    if (switchValue == HIGH) {
      gameOverLEDgameStartLEDtimesBlinked = 0;
    }

    /**HIGH SCORE FUNCTIONALITY*/
    if (highScoreAchieved) {
      delay(1500);

      gameHighScoreBuzzer();

      digitalWrite(ledColorsByPin[0], HIGH);
      delay(delayOfHighScoreLED);
      digitalWrite(ledColorsByPin[0], LOW);   // turn the LED off
      delay(delayOfHighScoreLED);
      digitalWrite(ledColorsByPin[0], HIGH);
      delay(delayOfHighScoreLED);
      digitalWrite(ledColorsByPin[0], LOW);   // turn the LED off
      delay(delayOfHighScoreLED);
      digitalWrite(ledColorsByPin[0], HIGH);
      delay(delayOfHighScoreLED);
      digitalWrite(ledColorsByPin[0], LOW);   // turn the LED off
      delay(delayOfHighScoreLED);
      digitalWrite(ledColorsByPin[0], HIGH);
      delay(delayOfHighScoreLED);
      digitalWrite(ledColorsByPin[0], LOW);   // turn the LED off
      delay(delayOfHighScoreLED);

      lcd.setCursor(0, 0);
      lcd.println("NEW HIGH SCORE  ");
      lcd.setCursor(0, 1);
      lcd.print(gameHighScore);
      lcd.print("                 ");
      highScoreAchieved = false;
      delay(6500);//Keep high score displayed for 6.5 secs before returning to title screen
    }
  }

  else {
    if (RUNNING_MANPos == RUNNING_MAN_POSITION_RUN_LOWER_2 || RUNNING_MANPos == RUNNING_MAN_POSITION_JUMP_8) {
      RUNNING_MANPos = RUNNING_MAN_POSITION_RUN_LOWER_1;
    } else if ((RUNNING_MANPos >= RUNNING_MAN_POSITION_JUMP_3 && RUNNING_MANPos <= RUNNING_MAN_POSITION_JUMP_5) && terrainLower[RUNNING_MAN_HORIZONTAL_POSITION] != SPRITE_TERRAIN_EMPTY) {
      RUNNING_MANPos = RUNNING_MAN_POSITION_RUN_UPPER_1;
    } else if (RUNNING_MANPos >= RUNNING_MAN_POSITION_RUN_UPPER_1 && terrainLower[RUNNING_MAN_HORIZONTAL_POSITION] == SPRITE_TERRAIN_EMPTY) {
      RUNNING_MANPos = RUNNING_MAN_POSITION_JUMP_5;
    } else gif (RUNNING_MANPos == RUNNING_MAN_POSITION_RUN_UPPER_2) {
      RUNNING_MANPos = RUNNING_MAN_POSITION_RUN_UPPER_1;
    } else {
      ++RUNNING_MANPos;
    }
    ++distance;//Increase amount of distance
    digitalWrite(PIN_AUTOPLAY, terrainLower[RUNNING_MAN_HORIZONTAL_POSITION + 4] == SPRITE_TERRAIN_EMPTY ? HIGH : LOW);
  }
  gameSpeed();//Change speed of game based on player score
}//End loop
