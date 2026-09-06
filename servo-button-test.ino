#include <ESP32Servo.h>

Servo servoMerah;
Servo servoKuning;

const int servoPin1 = 13;   // servo merah
const int buttonPin1 = 14;  // tombol merah

const int servoPin2 = 27;   // servo kuning
const int buttonPin2 = 26;  // tombol kuning

void setup() {
  Serial.begin(115200);

  // WAJIB di versi ESP32 core terbaru, sebelum attach()
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);  // servo kedua butuh timer terpisah

  pinMode(buttonPin1, INPUT_PULLUP);
  pinMode(buttonPin2, INPUT_PULLUP);

  servoMerah.setPeriodHertz(50);
  servoMerah.attach(servoPin1, 500, 2400);
  servoMerah.write(0);

  servoKuning.setPeriodHertz(50);
  servoKuning.attach(servoPin2, 500, 2400);
  servoKuning.write(0);

  Serial.println("Sistem siap! (2 servo, 2 button)");
}

void loop() {
  // Tombol merah -> servo merah
  if (digitalRead(buttonPin1) == LOW) {
    servoMerah.write(90);
    Serial.println("Tombol merah ditekan!");
  } else {
    servoMerah.write(0);
  }

  // Tombol kuning -> servo kuning
  if (digitalRead(buttonPin2) == LOW) {
    servoKuning.write(90);
    Serial.println("Tombol kuning ditekan!");
  } else {
    servoKuning.write(0);
  }

  delay(50);
}
