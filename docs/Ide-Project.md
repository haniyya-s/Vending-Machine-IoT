# Ide Project: 
Vending Machine.

## Fitur:
- Pemilihan Produk
- Tampilan nama produk dan harga di LCD
- Pembayarn menggunakan kartu identitas
- Pengeluaran produk secara otomatis
- Sistem saldo/cek saldo
- Notifikasi stok habis
- Pengembalian transaksi gagal
- Mode admin

## Komponen & Fungsi:
Komponen & fungsi

- ESP32 (Otak seluruh vending machine)
- RC522 (Membaca kartu NFC)
- LCD 16×2 I2C (Menampilkan instruksi, saldo, harga, dll)
- Keypad 4×4 (Memilih produk)
- TB6612FNG (Mengontrol motor DC)
- DC Gear Motor (Memutar spiral)
- Spiral coil (Mendorong produk ke depan)
- Limit switch (Menentukan posisi/batas mekanisme)
- IR sensor (Memastikan produk benar-benar keluar)
- Buzzer (Bunyi notifikasi)
- LED merah (Error / tidak tersedia)
- LED hijau (Berhasil / siap)
- Push button (Tombol tambahan, misalnya reset/batal)
- Buck converter (Menurunkan 12 V menjadi 5 V)
- Power supply 12 V (Sumber listrik Utama)
- Kabel jumper (Penghubung rangkaian)
- Tempat keluaran
