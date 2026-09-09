<?php
// File ini tugasnya: ambil semua data produk dari database, kirim sebagai JSON

header('Content-Type: application/json');
header('Access-Control-Allow-Origin: *'); // biar website dari mana aja boleh akses (untuk belajar/testing)

// ---------- KONEKSI KE DATABASE ----------
$host = "localhost";
$user = "root";       // default XAMPP, biasanya "root"
$password = "";        // default XAMPP, biasanya kosong
$database = "vending_db";

$koneksi = new mysqli($host, $user, $password, $database);

// Cek apakah koneksi berhasil
if ($koneksi->connect_error) {
    http_response_code(500);
    echo json_encode(["error" => "Koneksi database gagal: " . $koneksi->connect_error]);
    exit();
}

// ---------- AMBIL DATA PRODUK ----------
$query = "SELECT id, nama_produk, harga_produk, stok_produk FROM produk";
$hasil = $koneksi->query($query);

$daftarProduk = [];

if ($hasil->num_rows > 0) {
    while ($baris = $hasil->fetch_assoc()) {
        $daftarProduk[] = $baris;
    }
}

// ---------- KIRIM SEBAGAI JSON ----------
echo json_encode($daftarProduk);

$koneksi->close();
?>
