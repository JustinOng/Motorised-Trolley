#include <Arduino.h>
#include "RF24.h"

const uint8_t MAX_PWM = 100;

// SC = Speed pulse signal, ZF = Direction, VR = Speed
// VR pins are PWM controlled
const uint8_t SC1_PIN = 5;
const uint8_t ZF1_PIN = 7;
const uint8_t VR1_PIN = 9;

const uint8_t SC2_PIN = 6;
const uint8_t ZF2_PIN = 8;
const uint8_t VR2_PIN = 10;

const uint8_t REMOTE_ADDRESS[6] =  { 0x9f, 0x93, 0xcf, 0x53, 0x0f, 0x87 };
const uint8_t TROLLEY_ADDRESS[6] = { 0x16, 0xb9, 0x72, 0x54, 0xd3, 0x88 };

RF24 radio(3,4);

struct Remote_State {
  uint8_t green_btn: 1;
  uint8_t blue_btn: 1;
  uint8_t yellow_btn: 1;
  uint8_t js_btn: 1;
  int16_t x;
  int16_t y;
};

void setup() {
  pinMode(SC1_PIN, INPUT);
  pinMode(SC2_PIN, INPUT);
  pinMode(ZF1_PIN, OUTPUT);
  pinMode(ZF2_PIN, OUTPUT);
  // Set OC1A/B (VR1/2) as output
  DDRB |= ((1<<PB1) | (1<<PB2));

  // Timer 1: Mode 5, Fast PWM with TOP = 0xFF
  TCCR1A = (1<<WGM10);
  TCCR1B = (1<<WGM12);

  // Clear OC1A/B on Compare Match
  TCCR1A |= ((1<<COM1A1) | (1<<COM1B1));

  // Set prescalar to 1
  TCCR1B |= (1<<CS10);

  OCR1A = 0;
  OCR1B = 0;

  radio.begin();
  radio.setPALevel(RF24_PA_LOW);
  radio.setPayloadSize(sizeof(Remote_State));
  radio.setDataRate(RF24_250KBPS);

  radio.openWritingPipe(REMOTE_ADDRESS);
  radio.openReadingPipe(1, TROLLEY_ADDRESS);

  radio.startListening();

  Serial.begin(115200);
}

void loop() {
  static uint32_t last_remote_update = 0;

  Remote_State remote_state;
  if (radio.available()) {
    while(radio.available()) {
      radio.read(&remote_state, sizeof(Remote_State));

      int16_t left_raw_speed = (int16_t) -(remote_state.y + remote_state.x);
      int16_t right_raw_speed = (int16_t) remote_state.y - remote_state.x;

      digitalWrite(ZF1_PIN, left_raw_speed > 0);
      digitalWrite(ZF2_PIN, right_raw_speed > 0);

      uint8_t left_speed = min(abs(left_raw_speed), 255);
      uint8_t right_speed = min(abs(right_raw_speed), 255);

      left_speed = map(left_speed, 0, 255, 0, MAX_PWM);
      right_speed = map(right_speed, 0, 255, 0, MAX_PWM);

      OCR1A = left_speed;
      OCR1B = right_speed;

      Serial.print("Left speed: ");
      Serial.print(left_raw_speed);
      Serial.print("/");
      Serial.print(left_speed);
      Serial.print(", right speed: ");
      Serial.print(right_raw_speed);
      Serial.print("/");
      Serial.println(right_speed);
    }

    last_remote_update = millis();
  }

  if ((millis() - last_remote_update) > 100) {
    OCR1A = OCR1B = 0;
  }
}
