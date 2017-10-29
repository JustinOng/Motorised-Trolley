#include "RF24.h"

const uint8_t GREEN_BTN_PIN = 6;
const uint8_t BLUE_BTN_PIN = 5;
const uint8_t YELLOW_BTN_PIN = 7;
const uint8_t JS_BTN_PIN = 8;
const uint8_t JS_X_PIN = A0;
const uint8_t JS_Y_PIN = A1;

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
  pinMode(GREEN_BTN_PIN, INPUT);
  pinMode(BLUE_BTN_PIN, INPUT);
  pinMode(YELLOW_BTN_PIN, INPUT);
  pinMode(JS_BTN_PIN, INPUT);

  radio.begin();
  radio.setPALevel(RF24_PA_LOW);
  radio.setPayloadSize(sizeof(Remote_State));
  radio.setDataRate(RF24_250KBPS);

  radio.openWritingPipe(TROLLEY_ADDRESS);
  radio.openReadingPipe(1, REMOTE_ADDRESS);

  radio.startListening();

  Serial.begin(115200);
}

void loop() {
  Remote_State remote_state;

  remote_state.green_btn = digitalRead(GREEN_BTN_PIN);
  remote_state.blue_btn = digitalRead(BLUE_BTN_PIN);
  remote_state.yellow_btn = digitalRead(YELLOW_BTN_PIN);
  remote_state.js_btn = digitalRead(JS_BTN_PIN);
  remote_state.x = map(analogRead(JS_X_PIN), 0, 1023, 254, -254);
  remote_state.y = map(analogRead(JS_Y_PIN), 0, 1023, -254, 254);

  radio.stopListening();
  if (!radio.write(&remote_state, sizeof(Remote_State))) {
    Serial.println("Failed to tx");
  }

  radio.startListening();
  delay(10);
}
