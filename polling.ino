#include "MeOrion.h"
#include "MsTimer2.h"
#include <math.h>
#include "control_lib.h"

// 拳銃の縦と横の幅（㎝）
#define HEIGHT 7.5
#define EXTEND_LENGTH 5

// 危険領域 (mm)
#define TRACKED_DISTANCE 35
#define FIRED_DISTANCE 10

// 距離の記録
#define SIZE 12

enum State {
  STARTUP,
  SCANNING,
  TRACKING,
  FIRING,
};

State currentState = STARTUP;
unsigned long startTime;

void setup() {
  init_step_motor(POSITION);
  init_ussensor();

  init_servo_motor(3, 1);
  init_servo_motor(3, 2);
  update_servo_angle(3, 1, 68);
  update_servo_angle(3, 2, 0);
  startTime = millis();
}

void scanning(void) {
  static int index = 0;
  static bool isReversed = false;
  static unsigned long lastStepTime = 0;
  static unsigned long lastScanTime = 0;

  float stepSpeed = 1;
  float stepPosition;
  int distantAtAngle;

  unsigned long now = millis();
  static unsigned long lastToggle = 0;
  static bool buzzerState = false;

  if (now - lastToggle >= 1000) {
    lastToggle = now;
    buzzerState = !buzzerState;

    if (buzzerState) buzzerOn();
    else buzzerOff();
  }

  if (now - lastStepTime >= 7500) {
    lastStepTime = now;
    if (index == 0) {
      isReversed = false;
    }
    if (index == SIZE - 1) {
      isReversed = true;
    }

    if (isReversed) {
      index--;
    } else {
      index++;
    }
    stepPosition = (index * 15 * M_PI / 180);
    update_step_position(1, stepPosition, stepSpeed);
  }

  if (now - lastScanTime >= 100) {
    lastScanTime = now;
    int distance = get_distance();
    Serial.println(index);
    Serial.println(distance);

    if (distance > 0 && distance < TRACKED_DISTANCE) {
      currentState = TRACKING;
      Serial.print("Found intruder at");
      Serial.println(index);
    }
  }
}

void tracking(void) {
  static unsigned long lastToggle = 0;
  static bool buzzerState = false;

  // 拳銃が近づけてくる物体を追跡する
  float speed = 0.5;
  float position = 2;
  int distance;
  float degree = 0;
  float x, y;

  distance = get_distance();
  Serial.println(distance);

  if (distance > 0) {
    x = (float)distance + (float)EXTEND_LENGTH;
    y = (float)HEIGHT;
    degree = atan2(y, x) * 180.0 / M_PI;
  }

  int angle = constrain((int)degree + 68, 0, 180);
  update_servo_angle(3, 1, angle);
  update_servo_angle(3, 2, 0);
  Serial.println(angle);
  int interval = map(distance,
                     FIRED_DISTANCE, TRACKED_DISTANCE,
                     50, 1000);
  interval = constrain(interval, 50, 1000);

  unsigned long now = millis();

  if (now - lastToggle >= interval) {
    lastToggle = now;
    buzzerState = !buzzerState;

    if (buzzerState) buzzerOn();
    else buzzerOff();
  }

  if (distance < FIRED_DISTANCE && distance > 0) {
    Serial.println("DANGER");
    currentState = FIRING;
  }
  if (distance > TRACKED_DISTANCE) {
    Serial.println("SAFE");
    update_servo_angle(3, 2, 0);
    currentState = SCANNING;
  }
}

void fire() {
  static int phase = 0;
  static unsigned long phaseTime = 0;

  unsigned long now = millis();

  switch (phase) {
    case 0:
      update_servo_angle(3, 2, 90);
      phaseTime = now;
      phase = 1;
      break;

    case 1:
      if (now - phaseTime >= 500) {
        update_servo_angle(3, 2, 0);
        update_servo_angle(3, 1, 68);
        phaseTime = now;
        phase = 2;
      }
      break;

    case 2: // reload delay
      if (now - phaseTime >= 5000) {
        phase = 0;
        currentState = SCANNING;
      }
      break;
  }
}


void startup() {
  static unsigned long lastToggle = 0;
  static int patternIndex = 0;
  static bool buzzerState = false;

  const int pattern[] = {
    300, 100, 100, 100, 100, 100,
    300, 100, 100, 100, 100, 100,
    300, 100, 100, 100, 100, 0
  };

  unsigned long now = millis();

  if (pattern[patternIndex] == 0) {
    buzzerOff();
    currentState = SCANNING;
    return;
  }

  if (now - lastToggle >= pattern[patternIndex]) {
    lastToggle = now;
    buzzerState = !buzzerState;

    if (buzzerState) buzzerOn();
    else buzzerOff();

    patternIndex++;
  }
}

void loop() {
  static bool sensorReady = false;
  if (!sensorReady) {
    if (millis() - startTime > 2000) {
      sensorReady = true;
    } else {
      return;
    }
  }
  
  switch (currentState) {
    case STARTUP:
      startup();
      break;
    case SCANNING:
      Serial.println("SCANNING");
      scanning();
      break;
    case TRACKING:
      Serial.println("TRACKING");
      tracking();
      break;
    case FIRING:
      Serial.println("FIRING");
      fire();
      break;
    default:
      currentState = SCANNING;
      break;
  }
}
