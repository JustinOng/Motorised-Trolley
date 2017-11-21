#include <Arduino.h>
#include <SendOnlySoftwareSerial.h>

const uint8_t MAX_PWM = 30;

// SC = Speed pulse signal 3 pulses per rotation?, ZF = Direction, VR = Speed
// VR pins are PWM controlled
const uint8_t SC1_PIN = 5;
const uint8_t ZF1_PIN = 7;
const uint8_t VR1_PIN = 9;

const uint8_t SC2_PIN = 6;
const uint8_t ZF2_PIN = 8;
const uint8_t VR2_PIN = 10;

struct Remote_State {
  uint8_t green_btn: 1;
  uint8_t blue_btn: 1;
  uint8_t yellow_btn: 1;
  uint8_t js_btn: 1;
  int16_t x;
  int16_t y;
};

SendOnlySoftwareSerial swSerial(12);

void setup() {
  pinMode(SC1_PIN, INPUT);
  pinMode(SC2_PIN, INPUT);
  pinMode(ZF1_PIN, OUTPUT);
  pinMode(ZF2_PIN, OUTPUT);
  pinMode(13, OUTPUT);
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

  Serial.begin(115200);
  swSerial.begin(115200);
}

void loop() {
  static uint32_t last_remote_update = 0;
  /*
    state:
    0: idle
    1: 0x5A (first byte of header) received
    2: 0xA8 (second byte of header) received
    3 - (3+sizeof(Remote_State)): nth byte of Remote_State
  */
  static uint8_t state = 0;

  static Remote_State remote_state;

  static uint32_t left_move_start = 0, right_move_start = 0;

  if (Serial.available()) {
    uint8_t ch = Serial.read();
    //swSerial.print("Rx ");
    //swSerial.println(ch, HEX);

    if (state == 0 && ch == 0x5A) {
      state = 1;
    }
    else if (state == 1 && ch == 0xA8) {
      state = 2;
    }
    else if (state >= 2) {
      *(((char *) &remote_state)+(state - 2)) = ch;
      state ++;
    }
  }

  if (state >= sizeof(Remote_State)+2) {
    do {
      state = 0;
      //swSerial.println("New packet received");
      //swSerial.println(remote_state.y);

      digitalWrite(13, HIGH);


      if (abs(remote_state.x) < 30 && abs(remote_state.y) < 30) {
        OCR1A = OCR1B = 0;
        break;
      };

      int16_t left_raw_speed = (int16_t) (remote_state.y + remote_state.x);
      int16_t right_raw_speed = (int16_t) -(remote_state.y - remote_state.x);

      digitalWrite(ZF1_PIN, left_raw_speed > 0);
      digitalWrite(ZF2_PIN, right_raw_speed > 0);

      uint8_t left_speed = min(abs(left_raw_speed), 255);
      uint8_t right_speed = min(abs(right_raw_speed), 255);

      left_speed = map(left_speed, 0, 255, 0, MAX_PWM);
      right_speed = map(right_speed, 0, 255, 0, MAX_PWM);

      if (OCR1A == 0 && left_speed > 0) {
        left_move_start = millis();
      }

      if (OCR1B == 0 && right_speed > 0) {
        right_move_start = millis();
      }

      OCR1A = left_speed;
      OCR1B = right_speed;

      /*Serial.print("Left speed: ");
      Serial.print(left_raw_speed);
      Serial.print("/");
      Serial.print(left_speed);
      Serial.print(", right speed: ");
      Serial.print(right_raw_speed);
      Serial.print("/");
      Serial.println(right_speed);*/
    }
    while(0);

    last_remote_update = millis();
  }

  if (left_move_start > 0) {
    if (OCR1A > 0 && (millis() - left_move_start) < 500) {
      OCR1A = 255;
    }
    else {
      left_move_start = 0;
    }
  }

  if (right_move_start > 0) {
    if (OCR1B > 0 && (millis() - right_move_start) < 500) {
      OCR1B = 255;
    }
    else {
      left_move_start = 0;
    }
  }

  if ((millis() - last_remote_update) > 100) {
    OCR1A = OCR1B = 0;
    digitalWrite(13, LOW);
  }

  static uint8_t pSC1 = 0, pSC2 = 0;
  static uint16_t SC1_count = 0, SC2_count;
  static uint32_t last_sc_reset = 0;

  uint8_t SC1 = digitalRead(SC1_PIN);
  uint8_t SC2 = digitalRead(SC2_PIN);

  if (pSC1 ^ SC1) {
    SC1_count ++;
  }

  if (pSC2 ^ SC2) {
    SC2_count ++;
  }

  if ((millis() - last_sc_reset) > 50) {
    /*swSerial.print(OCR1A);
    swSerial.print(", ");
    swSerial.print(OCR1B);
    swSerial.print(" M: ");
    swSerial.print(SC1_count);
    swSerial.print(", ");
    swSerial.println(SC2_count);*/

    SC1_count = SC2_count = 0;
    last_sc_reset = millis();
  }

  pSC1 = SC1;
}
