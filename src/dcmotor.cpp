#include <Arduino.h>

#define B_IA 12  // Канал B вперед
#define B_IB 11  // Канал B назад

void setup_dc() {
  
  pinMode(B_IA, OUTPUT);
  pinMode(B_IB, OUTPUT);
  
  digitalWrite(B_IA, LOW);
  digitalWrite(B_IB, LOW);
}

void loop_dc() {
   Serial.println("loop_dc");
  // Канал B поворот
  analogWrite(B_IA, 00);
  analogWrite(B_IB, 200);
  delay(2000);
}