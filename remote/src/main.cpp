#include <Arduino.h>
#include <SendOnlySoftwareSerial.h>

const uint8_t GREEN_BTN_PIN = 6;
const uint8_t BLUE_BTN_PIN = 5;
const uint8_t YELLOW_BTN_PIN = 7;
const uint8_t JS_BTN_PIN = 8;
const uint8_t JS_X_PIN = A0;
const uint8_t JS_Y_PIN = A1;

SendOnlySoftwareSerial swSerial(12);

struct Remote_State {
  uint8_t green_btn: 1;
  uint8_t blue_btn: 1;
  uint8_t yellow_btn: 1;
  uint8_t js_btn: 1;
  int16_t x;
  int16_t y;
};

void setup() {
  pinMode(GREEN_BTN_PIN, INPUT);
  pinMode(BLUE_BTN_PIN, INPUT);
  pinMode(YELLOW_BTN_PIN, INPUT);
  pinMode(JS_BTN_PIN, INPUT);

  Serial.begin(115200);
  swSerial.begin(115200);
}

void loop() {
  Remote_State remote_state;

  remote_state.green_btn = digitalRead(GREEN_BTN_PIN);
  remote_state.blue_btn = digitalRead(BLUE_BTN_PIN);
  remote_state.yellow_btn = digitalRead(YELLOW_BTN_PIN);
  remote_state.js_btn = digitalRead(JS_BTN_PIN);
  remote_state.x = map(analogRead(JS_X_PIN), 0, 1023, 254, -254);
  remote_state.y = map(analogRead(JS_Y_PIN), 0, 1023, -254, 254);

  Serial.write(0x5A);
  Serial.write(0xA8);

  uint8_t ch;
  for(uint8_t i = 0; i < sizeof(Remote_State); i++) {
    ch = *(((char *) &remote_state)+i);
    Serial.write(ch);
    //swSerial.print(i);
    //swSerial.print("tx ");
    //swSerial.println(ch, HEX);
  }

  delay(10);
}
