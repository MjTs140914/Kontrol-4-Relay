# 🔑 Relay Control System with Button, Touch, RFID & EEPROM

Proyek ini adalah **kontrol 4 relay** berbasis Arduino dengan input dari:
- Tombol multi-click
- Touch sensor
- RFID (MFRC522)
- EEPROM untuk menyimpan konfigurasi

> ⚡ **Catatan Penting:**  
> Relay 4 difungsikan khusus untuk **mematikan/menyalakan mesin**.  
> Karena itu Relay 4 dapat diakses lewat banyak cara (tombol klik ke-4, touch sensor, dan kartu RFID).

---

## ⚡ Pinout

| Pin Arduino | Fungsi |
|-------------|--------|
| **D2** | Tombol utama (multi-click) |
| **D3, D4, D5, D6** | Relay 1–4 (aktif LOW) |
| **D7** | Touch sensor |
| **D8** | Buzzer |
| **D9** | RST pin RFID |
| **D10** | SS/SDA RFID |
| **D11–D13** | MOSI, MISO, SCK (SPI) |

---

## 🔌 Wiring Diagram (ASCII)

```text
Arduino UNO/Nano
┌──────────────┐
│ D2   → Push Button
│ D3   → Relay 1 (aktif LOW)
│ D4   → Relay 2 (aktif LOW)
│ D5   → Relay 3 (aktif LOW)
│ D6   → Relay 4 (aktif LOW, untuk mesin)
│ D7   → Touch Sensor Input
│ D8   → Buzzer
│ D9   → RFID RST
│ D10  → RFID SDA/SS
│ D11  → RFID MOSI
│ D12  → RFID MISO
│ D13  → RFID SCK
│ 5V   → VCC Relay, RFID, Touch
│ GND  → Semua GND (Relay, RFID, Touch, Buzzer)
└──────────────┘
```

> **Catatan Wiring**  
> - Relay harus tipe **aktif LOW** (LOW = ON, HIGH = OFF).  
> - Gunakan power supply eksternal untuk 4 relay (hindari beban berlebih pada Arduino).  
> - Semua GND (Arduino + relay + RFID + touch) harus tersambung bersama.  

---

## 🧩 Fitur Utama
- **Relay 1–3** → toggle via tombol multi-click.  
- **Relay 4 (khusus mesin):**
  - Default **ON saat startup** (mesin hidup otomatis).  
  - Bisa toggle via tombol (klik ke-4), touch sensor, atau kartu RFID.  
- **EEPROM** → menyimpan status relay 1–3, mode beep, RFID, dan touch.  
- **Buzzer Feedback** → pola bunyi berbeda untuk ON, OFF, error, dan mode change.  
- **RFID RC522** → toggle Relay 4 jika UID kartu cocok dengan daftar.  
- **Watchdog RFID** → reset otomatis modul RFID jika tidak merespon.  

---

## 🎛️ Fungsi Tombol Multi-Click

| Jumlah Klik | Fungsi |
|-------------|--------|
| **1x** | Toggle Relay 1 |
| **2x** | Toggle Relay 2 |
| **3x** | Toggle Relay 3 |
| **4x** | Toggle Relay 4 (**matikan/nyalakan mesin**) |
| **6x** | Ganti mode bunyi (1–5 = beep, 6 = silent mode) |
| **7x** | Aktif/nonaktif RFID |
| **8x** | Aktif/nonaktif touch sensor |
| **Long press (5 detik)** | Toggle semua Relay 1–3 sekaligus |

---

## 📦 Dependencies
Install library berikut:
- [ClickButton](https://github.com/marcobrianza/ClickButton)  
- [MFRC522](https://github.com/miguelbalboa/rfid)  

Sudah ada di Arduino IDE:
- `EEPROM.h`  
- `SPI.h`  

---

## 🚀 Cara Pakai (Flow)

1. **Startup:**
   - Relay 1–3 memulihkan status terakhir dari EEPROM.  
   - Relay 4 (**mesin**) otomatis **ON** saat start.  
   - Buzzer berbunyi welcome tone (kecuali silent mode).  

2. **Kontrol via Tombol:**
   - Klik 1x → toggle relay 1.  
   - Klik 2x → toggle relay 2.  
   - Klik 3x → toggle relay 3.  
   - Klik 4x → toggle relay 4 (**mesin ON/OFF**).  
   - Klik 6x → ganti mode bunyi (1–5 beep, 6 silent).  
   - Klik 7x → aktif/nonaktifkan RFID.  
   - Klik 8x → aktif/nonaktifkan touch sensor.  
   - Long press 5 detik → toggle semua relay 1–3.  

3. **Kontrol via Touch Sensor:**
   - Sentuh sensor (≥250ms) → toggle relay 4 (mesin).  

4. **Kontrol via RFID:**
   - Tempelkan kartu.  
   - Jika UID valid → relay 4 toggle (mesin ON/OFF).  
   - Jika UID tidak dikenal → buzzer error.  

5. **RFID Watchdog:**
   - Jika modul RFID tidak merespon >3 detik → sistem reset otomatis modul.  

---

## 📋 UID RFID Valid

Kode hanya menerima UID RFID tertentu untuk toggle **Relay 4 (mesin)**.  
Anda bisa menambah UID baru di bagian kode:

```cpp
if (uidString == "idtag1" || uidString == "idtag2") {
  toggleRelay4();
}
```
© 2025 MjTs-140914™