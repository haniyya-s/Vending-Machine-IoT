#include <WiFi.h>
#include <HTTPClient.h>
#include <ESP32Servo.h>

// ---------- WIFI ----------
const char* WIFI_SSID     = "kampungi";
const char* WIFI_PASSWORD = "haniyyas";

// ---------- SERVER PHP ----------
const char* SERVER_IP = "10.70.70.156"; 
String API_BELI;

const int PRODUK_ID_MERAH  = 1;

// ---------- REVISI PIN HARDWARE ----------
const int servoPin1  = 16;  // REVISI: Menggunakan Pin 16 agar motor tidak muter sendiri saat booting/WiFi aktif
const int buttonPin1 = 14;  // Tombol Merah

Servo servoMerah;

// ---------- KALIBRASI SERVO 360 DERAJAT ----------
const int STOP_MERAH   = 90;  // Titik diam total (Ganti ke 89/91 jika motor merayap)
const int PUTAR_MERAH  = 0;   // 0 = Putar SEARAH JARUM JAM dengan torsi dan kecepatan penuh

const unsigned long DURASI_PUTAR = 1500; // Putaran laci spiral selama 1,5 detik (Pas 1 putaran penuh)
const unsigned long DEBOUNCE_DELAY = 50;

// Variabel Waktu Millis (Pengganti Delay)
unsigned long waktuMulaiMerah = 0;
bool servoMerahAktif = false;

// Variabel Filter Getaran Tombol
int lastButton1Reading = HIGH;
unsigned long lastDebounce1 = 0;
int button1State = HIGH;

bool laporKeServer(int produkId) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi tidak terhubung, gagal akses server!");
    return false;
  }

  HTTPClient http;
  // Mengirim parameter produk_id via HTTP GET Query URL agar PHP mudah membaca datanya
  String urlDenganParam = API_BELI + "?produk_id=" + String(produkId);
  
  http.begin(urlDenganParam);
  int httpCode = http.POST(""); // Kirim request POST dengan data terlampir di URL

  bool sukses = false;

  if (httpCode == 200) {
    String response = http.getString();
    Serial.print("Balasan asli server: ");
    Serial.println(response);
    
    // Validasi respons sukses dari skrip PHP di laptop kamu
    if (response.indexOf("\"sukses\":true") >= 0 || response.indexOf("sukses") >= 0) {
      sukses = true;
    }
  } else {
    Serial.print("Gagal koneksi server, kode HTTP: ");
    Serial.println(httpCode);
  }

  http.end();
  return sukses;
}

void triggerMerah() {
  Serial.println("Tombol merah dipencet, memverifikasi database...");
  if (laporKeServer(PRODUK_ID_MERAH)) {
    servoMerah.write(PUTAR_MERAH);    // Mulai putar searah jarum jam kekuatan penuh
    waktuMulaiMerah = millis();       // Catat waktu mulai putaran motor
    servoMerahAktif = true;           // Aktifkan status timer non-blocking
    Serial.println("Servo 16: Bergerak memutar spiral...");
  } else {
    Serial.println("Servo TIDAK digerakkan (Stok database habis / server error)");
  }
}

void setupWiFi() {
  Serial.print("Menghubungkan ke jaringan WiFi: ");
  Serial.println(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  int percobaan = 0;
  // Batasi pencarian sinyal agar ESP32 tidak hang selamanya jika WiFi putus
  while (WiFi.status() != WL_CONNECTED && percobaan < 20) {
    delay(500); 
    Serial.print(".");
    percobaan++;
  }
  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("WiFi Terhubung! IP ESP32: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("Koneksi WiFi Gagal. Sistem berjalan offline sementara.");
  }
}

void setup() {
  Serial.begin(115200);

  API_BELI = "http://" + String(SERVER_IP) + "/vending/beli.php";

  ESP32PWM::allocateTimer(0);

  pinMode(buttonPin1, INPUT_PULLUP);

  servoMerah.setPeriodHertz(50);
  servoMerah.attach(servoPin1);
  servoMerah.write(STOP_MERAH);   // Paksa posisi motor diam total saat awal booting

  setupWiFi();
  Serial.println("=== SISTEM VENDING MACHINE 1 SLOT ONLINE SIAP ===");
}

void loop() {
  // --- TIMER NON-BLOCKING MILLIS (PENGGANTI DELAY) ---
  if (servoMerahAktif && (millis() - waktuMulaiMerah >= DURASI_PUTAR)) {
    servoMerah.write(STOP_MERAH);  // Hentikan servo mendadak setelah durasi 1,5 detik habis
    servoMerahAktif = false;       // Reset status timer
    Serial.println("Servo 16: Berhenti otomatis (1 Putaran Selesai).");
  }

  // --- LOGIKA FILTER PEMBACAAN TOMBOL ---
  int reading1 = digitalRead(buttonPin1);
  if (reading1 != lastButton1Reading) lastDebounce1 = millis();
  
  if ((millis() - lastDebounce1) > DEBOUNCE_DELAY) {
    if (reading1 != button1State) {
      button1State = reading1;
      
      // Tombol mengeksekusi perintah jika ditekan (LOW) dan servo tidak sedang berputar
      if (button1State == LOW && !servoMerahAktif) {
        triggerMerah();
      }
    }
  }
  lastButton1Reading = reading1;
}
