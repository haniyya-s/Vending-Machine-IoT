#include <WiFi.h>
#include <HTTPClient.h>
#include <ESP32Servo.h>

// ---------- WIFI ----------
const char* WIFI_SSID     = "kampungi";
const char* WIFI_PASSWORD = "haniyyas";

// ---------- SERVER PHP ----------
const char* SERVER_IP = "10.119.51.4"; 
// Mengubah inisialisasi agar String IP digabung dengan benar tanpa error konversi
String API_BELI;

const int PRODUK_ID = 1;   // sesuaikan dengan id produk di database

Servo servoMotor;

const int servoPin = 13;
const int buttonPin = 14;

// ---------- REVISI KALIBRASI SERVO 360 DERAJAT ----------
const int STOP_SERVO  = 90;  // Titik netral/berhenti (Ubah ke 89/91 jika servo merayap)
const int PUTAR_SERVO = 0;   // REVISI: Nilai 0 membuat servo berputar SEARAH JARUM JAM dengan kecepatan penuh

// REVISI: Servo 360° berputar berdasarkan milidetik (ms). 
// Putaran spiral besi biasanya butuh waktu sekitar 1,5 detik (1500 ms) untuk 1 putaran penuh.
// Silakan naik-turunkan angka 1500 ini secara fisik sampai spiralnya pas berputar 360 derajat.
const unsigned long DURASI_PUTAR = 1500; 

const unsigned long DEBOUNCE_DELAY = 50;

int lastButtonReading = HIGH;
unsigned long lastDebounce = 0;
int buttonState = HIGH;

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

void triggerServo() {
  Serial.println("Tombol ditekan, lapor ke server...");
  if (laporKeServer(PRODUK_ID)) {
    servoMotor.write(PUTAR_SERVO);   // Mulai putar searah jarum jam kecepatan penuh
    delay(DURASI_PUTAR);             // Berputar selama durasi yang ditentukan
    servoMotor.write(STOP_SERVO);    // Berhenti (Mengunci posisi)
    Serial.println("Servo: Berhasil putar 1 putaran lalu berhenti");
  } else {
    Serial.println("Servo TIDAK digerakkan (stok habis / server gagal)");
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

  // Inisialisasi alamat API di dalam setup agar aman dari error compiler string
  API_BELI = "http://" + String(SERVER_IP) + "/vending/beli.php";

  ESP32PWM::allocateTimer(0);

  pinMode(buttonPin, INPUT_PULLUP);

  servoMotor.setPeriodHertz(50);
  servoMotor.attach(servoPin, 500, 2400);
  servoMotor.write(STOP_SERVO);   // Pastikan diam sejak awal nyala

  setupWiFi();

  Serial.println("Sistem siap! (1 servo 360 derajat, tanpa relay)");
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi terputus, mencoba sambung ulang...");
    WiFi.disconnect();
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    int percobaan = 0;
    while (WiFi.status() != WL_CONNECTED && percobaan < 20) {
      delay(500);
      Serial.print(".");
      percobaan++;
    }
    Serial.println();
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("WiFi tersambung ulang!");
    }
  }

  int reading = digitalRead(buttonPin);
  if (reading != lastButtonReading) {
    lastDebounce = millis();
  }
  if ((millis() - lastDebounce) > DEBOUNCE_DELAY) {
    if (reading != buttonState) {
      buttonState = reading;
      if (buttonState == LOW) {
        triggerServo();
      }
    }
  }
  lastButtonReading = reading;
}
