#include "MeOrion.h"
#include "MsTimer2.h"
#include <math.h>
#include "control_lib.h"
 
// 拳銃の縦と横の幅（㎝）
#define HEIGHT 7
#define EXTEND_LENGTH 7
 
// 危険領域 (mm)
#define TRACKED_DISTANCE 20
#define FIRED_DISTANCE 10
 
// 距離の記録
#define SIZE 12
int dist[SIZE];
int minIndex;
 
enum State {
  SCANNING,
  RETURNING,
  TRACKING,
  FIRING,
  RESETTING
};
 
State currentState = SCANNING;
 
void setup() {
  init_step_motor(POSITION);
  init_ussensor();
 
  init_servo_motor(3, 1);
  init_servo_motor(3, 2);
  update_servo_angle(3, 1, 68);
  update_servo_angle(3, 2, 90);
 
  for (int i = 0; i < SIZE; i++) {
    dist[i] = 99;
  }
}
 
void scanning(void) {
  static int index = 0;
  static bool isReversed = false;
  float stepSpeed = 1;
  float stepPosition;
  int distantAtAngle;
 
  int distance = get_distance();
  dist[index] = distance;
  if (index == 0 || index == SIZE - 1) {
    minIndex = index;
  }
 
  if (distance < dist[minIndex]) {
    minIndex = index;
  }
 
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
  delay(2000);
 
  if (dist[minIndex] > 0 && dist[minIndex] < TRACKED_DISTANCE) {
    currentState = RETURNING;
  }
}
 
void returning(void) {
  float stepSpeed = 1;
  update_step_position(1, (minIndex * 15 * M_PI / 180), stepSpeed);
  delay(200);
 
  currentState = TRACKING;
}
 
void tracking(void) {
  // 拳銃が近づけてくる物体を追跡する
  float speed = 0.5;
  float position = 2;
  int distance;
  float degree = 0;
  float x, y;
 
  distance = get_distance();
 
  if (distance > 0) {
    x = (float)distance + (float)EXTEND_LENGTH;
    y = (float)HEIGHT;
    degree = atan2(y, x) * 180.0 / M_PI;
  }
 
  int angle = constrain((int)degree + 68, 0, 180);
  update_servo_angle(3, 1, angle);
  update_servo_angle(3, 2, 90);
  delay(100);
 
  if (distance < FIRED_DISTANCE && distance > 0) {
    Serial.println("DANGER");
    currentState = FIRING;
  }
  if(distance > TRACKED_DISTANCE){
    Serial.println("SAFE");
    currentState = SCANNING;
  }
}
 
void fire(void) {
  // 拳銃を打つ
  update_servo_angle(3, 2, 0);
  delay(500);
  update_servo_angle(3, 2, 90);
  update_servo_angle(3, 1, 68);
  delay(500);
  currentState = SCANNING;
  return;
}
 
void loop() {
  switch (currentState) {
    case SCANNING:
      Serial.println("SCANNING");
      scanning();
      break;
    case RETURNING:
      Serial.println("RETURNING");
      returning();
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
