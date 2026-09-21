#include <WiFi.h>
#include <HTTPClient.h>
#include <ESP32Servo.h>

// ---------- WIFI ----------
const char* WIFI_SSID     = "kampungi";
const char* WIFI_PASSWORD = "haniyyas";

// ---------- SERVER PHP ----------
const char* SERVER_IP = "10.142.45.4";
String API_BELI = "http://" + String(SERVER_IP) + "/vending/beli.php";

// ---------- PRODUK ----------
const int PRODUK_ID_MERAH  = 1;
const int PRODUK_ID_KUNING = 2;

// ---------- SERVO ----------
Servo servoMerah;
Servo servoKuning;

const int servoPin1 = 13;
const int servoPin2 = 27;

// PERBAIKAN: Variabel untuk mencatat posisi terakhir servo (false = 0 derajat, true = 180 derajat)
bool posisiMerah180 = false;
bool posisiKuning180 = false;

// ---------- TOMBOL ----------
const int buttonPin1 = 14;
const int buttonPin2 = 26;

// ---------- RELAY ----------
const int relayPin = 32;

// ---------- DEBOUNCE ----------
const unsigned long DEBOUNCE_DELAY = 50;

int lastButton1Reading = HIGH;
unsigned long lastDebounce1 = 0;
int button1State = HIGH;

int lastButton2Reading = HIGH;
unsigned long lastDebounce2 = 0;
int button2State = HIGH;


// LAPOR KE SERVER
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


// SERVO MERAH
void triggerMerah() {
  Serial.println("Tombol merah ditekan, lapor ke server...");

  if (laporKeServer(PRODUK_ID_MERAH)) {
    digitalWrite(relayPin, HIGH); // Pastikan daya servo aktif
    delay(200);

    // PERBAIKAN: Cek posisi terakhir, lalu balikkan posisinya
    if (!posisiMerah180) {
      servoMerah.write(180);
      Serial.println("Servo merah: Bergerak ke 180 derajat & DIAM");
      posisiMerah180 = true; // Tandai posisi sekarang di 180
    } else {
      servoMerah.write(0);
      Serial.println("Servo merah: Kembali ke 0 derajat & DIAM");
      posisiMerah180 = false; // Tandai posisi sekarang di 0
    }
    
    delay(1000); // Beri waktu servo menyelesaikan putaran
  } else {
    Serial.println("Servo merah TIDAK digerakkan (stok habis / server gagal)");
  }
}


// SERVO KUNING
void triggerKuning() {
  Serial.println("Tombol kuning ditekan, lapor ke server...");

  if (laporKeServer(PRODUK_ID_KUNING)) {
    digitalWrite(relayPin, HIGH); // Pastikan daya servo aktif
    delay(200);

    // PERBAIKAN: Cek posisi terakhir, lalu balikkan posisinya
    if (!posisiKuning180) {
      servoKuning.write(180);
      Serial.println("Servo kuning: Bergerak ke 180 derajat & DIAM");
      posisiKuning180 = true; // Tandai posisi sekarang di 180
    } else {
      servoKuning.write(0);
      Serial.println("Servo kuning: Kembali ke 0 derajat & DIAM");
      posisiKuning180 = false; // Tandai posisi sekarang di 0
    }
    
    delay(1000); // Beri waktu servo menyelesaikan putaran
  } else {
    Serial.println("Servo kuning TIDAK digerakkan (stok habis / server gagal)");
  }
}


// WIFI SETUP
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
    Serial.println("WiFi terhubung!");
    Serial.print("IP ESP32: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.print("GAGAL connect WiFi. Status code: ");
    Serial.println(WiFi.status());
  }
}


// SETUP
void setup() {
  Serial.begin(115200);

  // ---------- RELAY ----------
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, HIGH); // Tetap berikan daya ke servo agar mengunci posisi awal
  delay(500);

  // ---------- PWM SERVO ----------
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);

  // ---------- BUTTON ----------
  pinMode(buttonPin1, INPUT_PULLUP);
  pinMode(buttonPin2, INPUT_PULLUP);

  // ---------- SERVO MERAH ----------
  servoMerah.setPeriodHertz(50);
  servoMerah.attach(servoPin1, 500, 2400);
  servoMerah.write(0); // Posisi awal saat mesin menyala

  // ---------- SERVO KUNING ----------
  servoKuning.setPeriodHertz(50);
  servoKuning.attach(servoPin2, 500, 2400);
  servoKuning.write(0); // Posisi awal saat mesin menyala

  // ---------- WIFI ----------
  setupWiFi();

  Serial.println();
  Serial.println("=================================");
  Serial.println(" SISTEM VENDING MACHINE READY");
  Serial.println("=================================");
}


// LOOP
void loop() {
  // ---------- CEK WIFI ----------
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

  // TOMBOL MERAH
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

  // TOMBOL KUNING
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
