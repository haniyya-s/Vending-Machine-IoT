#include <WiFi.h>
#include <HTTPClient.h>
#include <ESP32Servo.h>

// ---------- WIFI ----------
const char* WIFI_SSID     = "hani";
const char* WIFI_PASSWORD = "hebatkan";

// ---------- SERVER PHP (laptop kamu) ----------
// Pastikan laptop & ESP32 nyambung ke WiFi yang SAMA
const char* SERVER_IP = "172.25.19.4";
String API_BELI = "http://" + String(SERVER_IP) + "/vending/beli.php";

// ID produk di database (cek di phpMyAdmin, tabel produk, kolom id)
const int PRODUK_ID_MERAH  = 1;  // sesuai id "beng beng"
const int PRODUK_ID_KUNING = 2;  // sesuai id "better"

Servo servoMerah;
Servo servoKuning;

const int servoPin1 = 13;
const int buttonPin1 = 14;   // tombol merah

const int servoPin2 = 27;
const int buttonPin2 = 26;   // tombol kuning

const unsigned long DEBOUNCE_DELAY = 50;

int lastButton1Reading = HIGH;
unsigned long lastDebounce1 = 0;
int button1State = HIGH;

int lastButton2Reading = HIGH;
unsigned long lastDebounce2 = 0;
int button2State = HIGH;

bool laporKeServer(int produkId) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi tidak terhubung, tidak bisa lapor ke server");
    return false;
  }

  HTTPClient http;
  http.begin(API_BELI);
  http.addHeader("Content-Type", "application/json");

  String body = "{\"produk_id\":" + String(produkId) + "}";
  int httpCode = http.POST(body);

  bool sukses = false;

  if (httpCode == 200) {
    String response = http.getString();
    Serial.print("Balasan server: ");
    Serial.println(response);

    if (response.indexOf("\"sukses\":true") >= 0) {
      sukses = true;
    }
  } else {
    Serial.print("Gagal hubungi server, kode HTTP: ");
    Serial.println(httpCode);
  }

  http.end();
  return sukses;
}

void triggerMerah() {
  Serial.println("Tombol merah ditekan, lapor ke server...");
  if (laporKeServer(PRODUK_ID_MERAH)) {
    servoMerah.write(0);
    delay(200);
    servoMerah.write(90);
    Serial.println("Servo merah: TRIGGER -> tetap di 90 derajat");
  } else {
    Serial.println("Servo merah TIDAK digerakkan (stok habis / server gagal)");
  }
}

void triggerKuning() {
  Serial.println("Tombol kuning ditekan, lapor ke server...");
  if (laporKeServer(PRODUK_ID_KUNING)) {
    servoKuning.write(0);
    delay(200);
    servoKuning.write(90);
    Serial.println("Servo kuning: TRIGGER -> tetap di 90 derajat");
  } else {
    Serial.println("Servo kuning TIDAK digerakkan (stok habis / server gagal)");
  }
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

  Serial.println("Sistem siap! (tombol fisik + lapor otomatis ke server)");
}

void loop() {
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
}
