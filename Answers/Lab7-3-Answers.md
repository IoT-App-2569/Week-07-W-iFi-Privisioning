# ใบงานที่ 7.3 — BLE Provisioning & GATT Forensics

> รหัสนักศึกษา 67030098 | ESP-IDF v6.0.2

## 1. ผลการทดลอง (Results)

### 1.1 ข้อมูล BLE Advertising

| รายการ | ค่าที่ได้ |
| :--- | :--- |
| Device Name | `PROV_0E01D0` |
| Custom Service UUID (128-bit) | `021a9004-0382-4aea-bff4-6b3f1c5adfb4` |
| Proof-of-Possession | `abcd1234` |
| Transport | `ble` |

### 1.2 GATT Services & Characteristics ที่พบจาก nRF Connect

| Characteristic UUID | Descriptor `0x2901` (ชื่อ) | Properties | หน้าที่ |
| :--- | :--- | :--- | :--- |
| _(ตอบ)_ | `prov-session` | | |
| _(ตอบ)_ | `prov-config` | | |
| _(ตอบ)_ | `prov-scan` | | |
| _(ตอบ)_ | `proto-ver` | | |

### 1.3 Log การคืนหน่วยความจำ Bluetooth หลัง Provisioning สำเร็จ

```
_(วาง log ช่วง BT memory released ที่นี่)_
```

---

## 2. คำถามท้ายการทดลอง (Post-Lab Questions)

### ข้อ 1. เหตุใด BLE Provisioning จึงไม่ส่งผลให้สัญญาณ Wi-Fi บนสมาร์ตโฟนของผู้ใช้หลุดระหว่างทำรายการ?

_(เขียนคำตอบที่นี่)_

### ข้อ 2. Descriptor `0x2901` มีความสำคัญอย่างไรต่อการที่แอปพลิเคชันมือถือจะทราบว่า Characteristic แต่ละตัวใช้ทำหน้าที่อะไร?

_(เขียนคำตอบที่นี่)_

### ข้อ 3. การที่ ESP-IDF มีฟังก์ชัน `esp_bt_mem_release()` มีประโยชน์อย่างไรต่อการทำงานของแอปพลิเคชัน IoT หลังเชื่อมต่อ Wi-Fi สำเร็จ?

_(เขียนคำตอบที่นี่)_

---

## 3. สรุปผลการทดลอง (Conclusion)

_(เขียนสรุปที่นี่)_
