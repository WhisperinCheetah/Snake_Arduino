//BUG FIXES:
//FIXED Overflow error when grabbing 9th apple (arrays can't keep up) 
//apple is too big and leave trail
//gameover screen needs work
//
//add sound when picking up apple, when changing direction, on game over.


// include the necessary libraries
#include <SPI.h>
#include <TFT.h> // Hardware-specific library

#define LCD_CS 10
#define DC    9
#define RESET    8

//Butten pins
const int pinNorth = 2;
const int pinEast = 5;
const int pinSouth = 7;
const int pinWest = 3;

TFT myScreen = TFT(LCD_CS, DC, RESET);

//Snake vars
static const int SCREEN_WIDTH = 160;
static const int SCREEN_HEIGTH = 120;
static const int UNIT_SIZE = 8;
static const int XGAME_UNITS = SCREEN_WIDTH / UNIT_SIZE;
static const int YGAME_UNITS = SCREEN_HEIGTH / UNIT_SIZE;
static const int DELAY = 250;
int x[XGAME_UNITS * YGAME_UNITS];
int y[YGAME_UNITS * YGAME_UNITS];

int bodyParts = 6;
int applesEaten;
int appleX;
int appleY;
char direction = 'R'; // R: right, L: left, U: up, D: down
bool running = false;
long randomNumber;

char printout[4];

//restart vars
bool north = false;
bool east = false;
bool west = false;
bool south = false;
bool checkRestartInput;

void setup() {
  //Random value
  Serial.begin(9600);
  randomSeed(analogRead(A0));

  myScreen.begin();
  myScreen.background(0, 0, 0);
  
  pinMode(pinNorth, INPUT);
  pinMode(pinEast, INPUT);
  pinMode(pinSouth, INPUT);
  pinMode(pinWest, INPUT);
  digitalWrite(pinNorth, LOW);
  digitalWrite(pinEast, LOW);
  digitalWrite(pinSouth, LOW);
  digitalWrite(pinWest, LOW);

  newApple();  

/**
  tone(6, 1047, 330);
  delay(220);
  tone(6, 1568, 330);
  delay(220);
  tone(6, 2093, 330);
  delay(220);
  tone(6, 1976, 880);
  delay(440);
  tone(6, 1568, 330);
**/
  
  running = true;

}

void loop(){
  if (running) {
    move();
    draw();
    checkApple();
    checkCollision();
    delay(DELAY);
  } else {
    gameOver();
    if (digitalRead(pinNorth) == HIGH && digitalRead(pinEast) == HIGH && digitalRead(pinSouth) == HIGH && digitalRead(pinWest) == HIGH) {
      startGame();
    }
  }
}

void startGame() {
  myScreen.background(0, 0, 0);
  bodyParts = 6;
  applesEaten = 0;
  newApple();
  running = true;  
}

void newApple() {
  tone(6, 1047, 165);
  delay(110);
  tone(6, 1319, 165);
  myScreen.fillCircle(appleX + (UNIT_SIZE/2), appleY + (UNIT_SIZE/2), UNIT_SIZE/2, myScreen.Color565(144, 238, 144));
  appleX = random(0, XGAME_UNITS) * UNIT_SIZE;
  appleY = random(0, YGAME_UNITS) * UNIT_SIZE;
}

void checkInput() {
  if (digitalRead(pinWest) == HIGH && direction != 'R') { 
    Serial.println("WEST");
    direction = 'L'; 
  }
  if (digitalRead(pinSouth) == HIGH && direction != 'U') { 
    Serial.println("SOUTH");
    direction = 'D'; 
  }
  if (digitalRead(pinNorth) == HIGH && direction != 'D') { 
    Serial.println("NORTH");
    direction = 'U'; 
  }
  if (digitalRead(pinEast) == HIGH && direction != 'L') { 
    Serial.println("EAST");
    direction = 'R'; 
  }

}

void move() {
  checkInput();

  for (int i = bodyParts; i > 0; i--) {
    x[i] = x[i-1];
    y[i] = y[i-1];
  }

  switch(direction) {
    case 'U':
      y[0] = y[0] - UNIT_SIZE;
      break;
    case 'D':
      y[0] = y[0] + UNIT_SIZE;
      break;
    case 'R':
      x[0] = x[0] + UNIT_SIZE;
      break;
    case 'L':
      x[0] = x[0] - UNIT_SIZE;
      break;
  }
}

void draw() {
  //draw apple
  myScreen.fill(255, 0, 0);
  myScreen.fillCircle(appleX + (UNIT_SIZE/2) - 1, appleY + (UNIT_SIZE/2) - 1, UNIT_SIZE/2 - 1, ST7735_RED);

  //draw snake  
  myScreen.noStroke();
  myScreen.fill(0, 0, 0);
  myScreen.rect(x[bodyParts - 1], y[bodyParts - 1], UNIT_SIZE, UNIT_SIZE);
  myScreen.fill(144, 238, 144);
  myScreen.rect(x[0], y[0], UNIT_SIZE, UNIT_SIZE);
}

void checkApple() {
  if (x[0] == appleX && y[0] == appleY) {
    newApple();
    bodyParts++;
    applesEaten++;
  }
}

void checkCollision() {
  for (int i = bodyParts; i > 0; i--) {
    if ((x[0] == x[i]) && (y[0] == y[i])) {
      running = false;
    }
  }

  //collision LEFT
  if (x[0] < 0) {
    running = false;
  }

  //collision RIGHT
  if (x[0] > SCREEN_WIDTH) {
    running = false;
  }

  //collision TOP
  if (y[0] < 0) {
    running = false;
  }

  //collision BOTTOM 
  if (y[0] > SCREEN_HEIGTH) {
    running = false;
  }

}

void gameOver() {
  String score = String(applesEaten);
  score.toCharArray(printout, 4);  
  myScreen.stroke(255, 255, 255);
  myScreen.setTextSize(2);
  int i = 0;
  while (!running) {
    myScreen.stroke(255, 255, 255);
    myScreen.background(0, 0, 0);
    myScreen.text("You ate", SCREEN_WIDTH/2 - 35, (SCREEN_HEIGTH/2 - 50) + 20*(i%3));
    myScreen.stroke(0, 255, 0);
    myScreen.text(printout, SCREEN_WIDTH/2 - 5, (SCREEN_HEIGTH/2 - 30) + 20*(i%3));
    myScreen.stroke(255, 0, 0);
    myScreen.text("apples", SCREEN_WIDTH/2 - 30, (SCREEN_HEIGTH/2 - 10) + 20*(i%3));
    if (checkRestart()) {
      startGame();
    }
    delay(1000);
    i++;
  }

}


bool checkRestart() {
  if (digitalRead(pinNorth) == HIGH) {
    Serial.println("NORTH");
    bool north = true;
  }
  if (digitalRead(pinWest) == HIGH) {
    Serial.println("WEST");
    bool west = true;
  }
  if (digitalRead(pinEast) == HIGH) {
    Serial.println("EAST");
    bool east = true;
  }
  if (digitalRead(pinSouth) == HIGH) {
    Serial.println("SOUTH");
    bool south = true;
  }

  return south && east && west && north;


}
