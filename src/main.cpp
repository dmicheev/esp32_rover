#include <Arduino.h>

#include "servo.h"
#include "dcmotor.h"

void setup() {

  Serial.begin(115200);
  Serial.println("Robot starting...");
  setup_serv();

  //setup_dc();

}

void loop() {
  loop_serv();
  //loop_dc();
  Serial.println("...");
  delay(1000);
}

