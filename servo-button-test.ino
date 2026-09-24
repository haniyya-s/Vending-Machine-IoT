#include <WiFi.h>
#include <HTTPClient.h>
#include <ESP32Servo.h>

const char* WIFI_SSID     = "kampungi";
const char* WIFI_PASSWORD = "haniyyas";
const char* SERVER_IP     = "10.119.51.4"; 
String API_BELI;
const int PRODUK_ID       = 1;   

Servo servoMotor;
const int servoPin  = 4;   // Pin Sinyal Servo (GPIO 4)
const int buttonPin = 14;  // Pin Sinyal Tombol (GPIO 14)

const int PULSA_STOP  = 1500;  // Titik diam murni servo 360°
const int PULSA_PUTAR = 1000;  // Putar searah jarum jam kecepatan penuh
const unsigned long DURASI_PUTAR = 1500; 

const unsigned long DEBOUNCE_DELAY = 50;
int lastButtonReading = HIGH;
unsigned long lastDebounce = 0;
int buttonState = HIGH;

bool laporKeServer(int produkId) {
  if (WiFi.status() != WL_CONNECTED) return false;
  HTTPClient http;
  http.begin(API_BELI);
  http.addHeader("Content-Type", "application/json");
  String body = "{\"produk_id\":" + String(produkId) + "}";
  int httpCode = http.POST(body);
  bool sukses = (httpCode == 200 && http.getString().indexOf("\"sukses\":true") >= 0);
  http.end();
  return sukses;
}

void triggerServo() {
  Serial.println("Tombol ditekan, lapor ke server...");
  bool statusServer = laporKeServer(PRODUK_ID);
  
  if (statusServer) {
    WiFi.disconnect(); // ANTI-INTERFERENSI: Memutus WiFi sesaat sewaktu motor menyala
    delay(50); 
    
    servoMotor.writeMicroseconds(PULSA_PUTAR); // Perintah putar kontinu murni maju terus
    delay(DURASI_PUTAR);                       
    servoMotor.writeMicroseconds(PULSA_STOP);  // Perintah berhenti total
    
    setupWiFi(); // Menghubungkan ulang internet setelah motor diam
  }
}

void setupWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  int percobaan = 0;
  while (WiFi.status() != WL_CONNECTED && percobaan < 30) {
    delay(500);
    percobaan++;
  }
}

void setup() {
  Serial.begin(115200);
  API_BELI = "http://" + String(SERVER_IP) + "/vending/beli.php";
  ESP32PWM::allocateTimer(0);
  pinMode(buttonPin, INPUT_PULLUP);
  
  servoMotor.setPeriodHertz(50);
  servoMotor.attach(servoPin, 500, 2500);
  servoMotor.writeMicroseconds(PULSA_STOP);   

  setupWiFi();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    delay(500);
  }
  int reading = digitalRead(buttonPin);
  if (reading != lastButtonReading) lastDebounce = millis();
  if ((millis() - lastDebounce) > DEBOUNCE_DELAY) {
    if (reading != buttonState) {
      buttonState = reading;
      if (buttonState == LOW) triggerServo();
    }
  }
  lastButtonReading = reading;
}
