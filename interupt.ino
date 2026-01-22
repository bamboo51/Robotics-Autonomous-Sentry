#include "MeOrion.h"
#include "MsTimer2.h"
#include <math.h>
#include "control_lib.h"

#define HEIGHT 7.5
#define EXTEND_LENGTH 5
#define TRACKED_DISTANCE 35
#define FIRED_DISTANCE 10
#define SIZE 12

enum State {
  SCANNING,
  TRACKING,
  FIRING,
};

State currentState = SCANNING;
int currentDistance = 100;

unsigned long lastScanTime = 0;
unsigned long lastFireTime = 0;

volatile int buzzerInterval = 0; 
volatile bool buzzerState = false;

void onTimer() {
  static int timeCounter = 0;

  if (buzzerInterval > 0) {
    timeCounter += 10;
    if (timeCounter >= buzzerInterval) {
      timeCounter = 0;
      buzzerState = !buzzerState;
      if (buzzerState) buzzerOn();
      else buzzerOff();
    }
  } else {
    buzzerOff();
    timeCounter = 0;
  }
}

void setup() {
  Serial.begin(9600);
  init_step_motor(POSITION);
  init_ussensor();
  init_servo_motor(3, 1);
  init_servo_motor(3, 2);
  update_servo_angle(3, 1, 68);
  update_servo_angle(3, 2, 0);

  MsTimer2::set(10, onTimer); 
  MsTimer2::start();
}

void scanning(void) {
  static int index = 0;
  static bool isReversed = false;

  currentDistance = get_distance();
  if (currentDistance > 0 && currentDistance < TRACKED_DISTANCE) {
    currentState = TRACKING;
    Serial.println("!!! INTRUDER DETECTED !!!");
    return;
  }
  
  buzzerInterval = 0; 

  if (millis() - lastScanTime > 500) {
    lastScanTime = millis();

    if (index == 0) isReversed = false;
    else if (index == SIZE - 1) isReversed = true;

    if (isReversed) index--;
    else index++;

    float stepPosition = (index * 15 * M_PI / 180);
    update_step_position(1, stepPosition, 1);
    
    Serial.print("Scanning Index: ");
    Serial.println(index);
  }
}

void tracking(void) {
  currentDistance = get_distance();

  if (currentDistance > TRACKED_DISTANCE) {
    currentState = SCANNING;
    buzzerInterval = 0;
    Serial.println("SAFE");
    return;
  }
  if (currentDistance > 0 && currentDistance < FIRED_DISTANCE) {
    currentState = FIRING;
    buzzerInterval = 0;
    Serial.println("FIRING!");
    return;
  }

  float x = (float)currentDistance + (float)EXTEND_LENGTH;
  float y = (float)HEIGHT;
  float degree = atan2(y, x) * 180.0 / M_PI;
  int angle = constrain((int)degree + 68, 0, 180);
  update_servo_angle(3, 1, angle);

  int interval = map(currentDistance, FIRED_DISTANCE, TRACKED_DISTANCE, 50, 500);
  buzzerInterval = constrain(interval, 50, 500); 
}

void fire(void) {
  // 拳銃を撃つシーケンス
  // State 0: 撃つ
  // State 1: 元の位置に戻る
  // State 2: リロード
  
  static int fireState = 0;
  static unsigned long fireTimer = 0;

  if (fireState == 0) {
    update_servo_angle(3, 2, 90);
    fireTimer = millis();
    fireState = 1;
    Serial.println("BANG!");
  }
  else if (fireState == 1) {
    if (millis() - fireTimer > 500) {
      update_servo_angle(3, 2, 0); 
      update_servo_angle(3, 1, 68);
      Serial.println("Reloading...");
      fireState = 2;
    }
  }
  else if (fireState == 2) {
    if (millis() - fireTimer > 5500) {
      currentState = SCANNING;
      fireState = 0;
      Serial.println("Ready.");
    }
  }
}

void loop() {
  switch (currentState) {
    case SCANNING:
      scanning();
      break;
    case TRACKING:
      tracking();
      break;
    case FIRING:
      fire();
      break;
    default:
      currentState = SCANNING;
      break;
  }
}