#include <ESP32Servo.h>

const int pinServo  = 18;
const int pinTombol = 14;

Servo myServo;

// --- KALIBRASI TITIK DIAM MURNI ---
// Jika nanti servo langsung muter pas dinyalain, kita hanya perlu otak-atik angka 90 ini!
const int NILAI_DIAM = 89; 

void setup() {
  Serial.begin(115200);
  
  pinMode(pinTombol, INPUT_PULLUP);
  
  ESP32PWM::allocateTimer(0);
  myServo.setPeriodHertz(50);
  myServo.attach(pinServo);
  
  // Perintah awal: Wajib diam total saat pertama dinyalakan
  myServo.write(NILAI_DIAM);
  
  Serial.println("======================================");
  Serial.println("=== MULAI TES HARWARE DARI NOL ===");
  Serial.println("======================================");
}

void loop() {
  // Jika tombol dipencet (LOW)
  if (digitalRead(pinTombol) == LOW) {
    Serial.println("Tombol terdeteksi DIPENCET!");
    
    myServo.write(0);    // Putar searah jarum jam kecepatan penuh
    delay(1500);         // Berputar selama 1,5 detik
    myServo.write(NILAI_DIAM); // Berhenti kembali
    
    Serial.println("Selesai berputar, harusnya servo diam.");
    delay(1000);         // Jeda pelindung tombol
  }
}
