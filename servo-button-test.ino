#include <WiFi.h>
#include <PubSubClient.h>
#include <ESP32Servo.h>

// ---------- WIFI ----------
const char* WIFI_SSID     = "zal";
const char* WIFI_PASSWORD = "jerapahtinggi";

// ---------- MQTT ----------
const char* MQTT_BROKER    = "broker.hivemq.com";
const int   MQTT_PORT      = 1883;
const char* MQTT_CLIENT_ID = "esp32-servo-kontrol-2servo";

const char* TOPIC_MERAH  = "servo-kontrol/servoMerah/cmd";
const char* TOPIC_KUNING = "servo-kontrol/servoKuning/cmd";

WiFiClient espClient;
PubSubClient mqttClient(espClient);

Servo servoMerah;
Servo servoKuning;

const int servoPin1 = 13;
const int buttonPin1 = 14;

const int servoPin2 = 27;
const int buttonPin2 = 26;

const unsigned long PULSE_DURATION = 1000;
const unsigned long DEBOUNCE_DELAY = 50;

bool merahAktif = false;
unsigned long merahPulseStart = 0;
int lastButton1Reading = HIGH;
unsigned long lastDebounce1 = 0;
int button1State = HIGH;

bool kuningAktif = false;
unsigned long kuningPulseStart = 0;
int lastButton2Reading = HIGH;
unsigned long lastDebounce2 = 0;
int button2State = HIGH;

void triggerMerah() {
  servoMerah.write(90);
  merahAktif = true;
  merahPulseStart = millis();
  Serial.println("Servo merah: TRIGGER -> 90 derajat");
}

void triggerKuning() {
  servoKuning.write(90);
  kuningAktif = true;
  kuningPulseStart = millis();
  Serial.println("Servo kuning: TRIGGER -> 90 derajat");
}

void setupWiFi() {
  Serial.print("Menghubungkan ke WiFi: ");
  Serial.println(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  int percobaan = 0;
  while (WiFi.status() != WL_CONNECTED && percobaan < 30) {
    delay(500);
    Serial.print(".");
    percobaan++;
  }
  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("WiFi terhubung, IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.print("GAGAL connect WiFi. Status code: ");
    Serial.println(WiFi.status());
    Serial.println("Cek lagi nama WiFi & password, lalu tekan tombol EN untuk coba ulang.");
  }
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  String message;
  for (unsigned int i = 0; i < length; i++) message += (char)payload[i];

  Serial.print("MQTT masuk [");
  Serial.print(topic);
  Serial.print("]: ");
  Serial.println(message);

  if (String(topic) == TOPIC_MERAH) {
    triggerMerah();
  } else if (String(topic) == TOPIC_KUNING) {
    triggerKuning();
  }
}

void reconnectMQTT() {
  while (!mqttClient.connected()) {
    Serial.print("Menghubungkan ke MQTT broker...");
    if (mqttClient.connect(MQTT_CLIENT_ID)) {
      Serial.println("berhasil!");
      mqttClient.subscribe(TOPIC_MERAH);
      mqttClient.subscribe(TOPIC_KUNING);
    } else {
      Serial.print("gagal, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" coba lagi 3 detik lagi");
      delay(3000);
    }
  }
}

void setup() {
  Serial.begin(115200);

  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);

  pinMode(buttonPin1, INPUT_PULLUP);
  pinMode(buttonPin2, INPUT_PULLUP);

  servoMerah.setPeriodHertz(50);
  servoMerah.attach(servoPin1, 500, 2400);
  servoMerah.write(0);

  servoKuning.setPeriodHertz(50);
  servoKuning.attach(servoPin2, 500, 2400);
  servoKuning.write(0);

  setupWiFi();
  mqttClient.setServer(MQTT_BROKER, MQTT_PORT);
  mqttClient.setCallback(mqttCallback);

  Serial.println("Sistem siap! (2 servo, mode pulse, + MQTT)");
}

void loop() {
  if (!mqttClient.connected()) {
    reconnectMQTT();
  }
  mqttClient.loop();

  int reading1 = digitalRead(buttonPin1);
  if (reading1 != lastButton1Reading) {
    lastDebounce1 = millis();
  }
  if ((millis() - lastDebounce1) > DEBOUNCE_DELAY) {
    if (reading1 != button1State) {
      button1State = reading1;
      if (button1State == LOW) {
        triggerMerah();
      }
    }
  }
  lastButton1Reading = reading1;

  int reading2 = digitalRead(buttonPin2);
  if (reading2 != lastButton2Reading) {
    lastDebounce2 = millis();
  }
  if ((millis() - lastDebounce2) > DEBOUNCE_DELAY) {
    if (reading2 != button2State) {
      button2State = reading2;
      if (button2State == LOW) {
        triggerKuning();
      }
    }
  }
  lastButton2Reading = reading2;

  if (merahAktif && (millis() - merahPulseStart >= PULSE_DURATION)) {
    servoMerah.write(0);
    merahAktif = false;
    Serial.println("Servo merah: otomatis balik ke 0 derajat");
  }

  if (kuningAktif && (millis() - kuningPulseStart >= PULSE_DURATION)) {
    servoKuning.write(0);
    kuningAktif = false;
    Serial.println("Servo kuning: otomatis balik ke 0 derajat");
  }
}
