#include <Servo.h>

Servo servo;

const int servoPin = 9;
const int buttonPin = 2;

bool fokus = false;
bool tombolSebelumnya = HIGH;

void setup() {
  servo.attach(servoPin);
  pinMode(buttonPin, INPUT_PULLUP);

  // Posisi awal = TERSEDIA
  servo.write(0);
}

void loop() {
  bool tombolSekarang = digitalRead(buttonPin);

  if (tombolSebelumnya == HIGH && tombolSekarang == LOW) {

    fokus = !fokus;

    if (fokus) {
      servo.write(90);
    } else {
      servo.write(0);
    }

    delay(300);
  }

  tombolSebelumnya = tombolSekarang;
}
