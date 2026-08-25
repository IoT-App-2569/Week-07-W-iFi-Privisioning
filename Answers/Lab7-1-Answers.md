# ใบงานที่ 7.1 — Reset Mechanisms & NVS Forensics

> รหัสนักศึกษา 67030098 | ESP-IDF v6.0.2

## 1. ผลการทดลอง (Results)

### 1.1 Boot Log ที่ได้จากบอร์ดจริง

```
I (443) app_init: ESP-IDF:          v6.0.2
I (424) app_init: Project name:     lab7_1_reset_nvs_forensics
I (429) app_init: App version:      f874021-dirty
I (512) LAB7_1_RESET: Hold GPIO 18 button for 3 seconds to trigger Factory Reset...
W (612) LAB7_1_RESET: [STATUS]: Device is NOT provisioned (NVS is empty)
W (622) LAB7_1_RESET: Ready for Provisioning Lab 7-2 (SoftAP) or 7-3 (BLE)!
```

### 1.2 Partition Table ที่อ่านได้ตอน Boot

| Label | Usage | Type | ST | Offset | Length |
| :--- | :--- | :--- | :--- | :--- | :--- |
| nvs | WiFi data | 01 | 02 | `0x009000` | `0x006000` |
| phy_init | RF data | 01 | 01 | `0x00f000` | `0x001000` |
| factory | factory app | 00 | 00 | `0x010000` | `0x100000` |

### 1.3 ผลการทดสอบ Reset ทั้ง 3 รูปแบบ

| วิธี Reset | คำสั่ง / การกระทำ | ผลที่สังเกตได้ | เฟิร์มแวร์หายหรือไม่ |
| :--- | :--- | :--- | :--- |
| 1. CLI Flash Erase | `idf.py erase-flash` | _(ตอบ)_ | _(ตอบ)_ |
| 2. Menuconfig Build Flag | `network_prov_mgr_reset_wifi_provisioning()` | _(ตอบ)_ | _(ตอบ)_ |
| 3. Hardware Pushbutton | กด GPIO 18 ค้าง 3 วินาที | _(ตอบ)_ | _(ตอบ)_ |

---

## 2. คำถามท้ายการทดลอง (Post-Lab Questions)

### ข้อ 1. เพราะเหตุใดการกดปุ่ม BOOT (GPIO 0) ค้างไว้ในจังหวะรีเซ็ตบอร์ด จึงทำให้โปรแกรมค้างอยู่ที่ ROM Bootloader และไม่ยอมทำงานต่อ?

_(เขียนคำตอบที่นี่)_

### ข้อ 2. เพราะเหตุใดคำสั่ง `idf.py erase-flash` จึงทำให้ข้อมูลเฟิร์มแวร์ Application หายไปด้วย ในขณะที่ `nvs_flash_erase()` ไม่ทำให้เฟิร์มแวร์หาย?

_(เขียนคำตอบที่นี่)_

### ข้อ 3. การออกแบบปุ่ม Factory Reset บนอุปกรณ์ IoT เชิงพาณิชย์ เหตุใดจึงต้องกำหนดให้ผู้ใช้กดปุ่มค้างไว้ 3-5 วินาที แทนที่จะสั่งลบข้อมูลทันทีที่แตะปุ่มเพียงเสี้ยววินาที?

_(เขียนคำตอบที่นี่)_

### ข้อ 4. หากอุปกรณ์ IoT ถูกติดตั้งอยู่บนเสาสูงหรือฝังอยู่ในผนัง วิธีการ Reset ทางกายภาพรูปแบบใดเหมาะสมที่สุด?

_(เขียนคำตอบที่นี่)_

---

## 3. สรุปผลการทดลอง (Conclusion)

_(เขียนสรุปที่นี่)_
