# ใบงานที่ 7.2 — SoftAP Provisioning & HTTP Sniffing

> รหัสนักศึกษา 67030098 | ESP-IDF v6.0.2

## 1. ผลการทดลอง (Results)

### 1.1 ข้อมูล SoftAP ที่ ESP32 สร้างขึ้น

| รายการ | ค่าที่ได้ |
| :--- | :--- |
| SSID (Service Name) | `PROV_0E01D0` |
| Proof-of-Possession | `abcd1234` |
| Security Scheme | `NETWORK_PROV_SECURITY_1` |
| IP Address ของ ESP32 | _(ตอบ)_ |
| Transport | `softap` |

### 1.2 Protocomm Endpoints ที่พบ

| Endpoint | หน้าที่ | สังเกตจาก |
| :--- | :--- | :--- |
| `prov-session` | _(ตอบ)_ | |
| `prov-config` | _(ตอบ)_ | |
| `prov-scan` | _(ตอบ)_ | |
| `proto-ver` | _(ตอบ)_ | |

### 1.3 ลำดับ Event ที่เกิดขึ้นบน Serial Monitor

```
_(วาง log ที่ได้จริงที่นี่)_
```

---

## 2. คำถามท้ายการทดลอง (Post-Lab Questions)

### ข้อ 1. ในโหมด SoftAP Scheme สมาร์ตโฟนส่งข้อมูลหา ESP32 ผ่านโปรโตคอลและ IP Address ใด?

_(เขียนคำตอบที่นี่)_

### ข้อ 2. หากผู้ใช้ป้อนรหัสผ่าน Wi-Fi ผิดในแอปมือถือ จะเกิด Event ใดขึ้นบน ESP32 (`NETWORK_PROV_WIFI_CRED_FAIL` หรือไม่) และ ESP32 มีพฤติกรรมอย่างไร?

_(เขียนคำตอบที่นี่)_

### ข้อ 3. ทำไมผู้ผลิต IoT ส่วนใหญ่จึงมองว่ากระบวนการเชื่อมต่อแบบ SoftAP มีขั้นตอนที่ยุ่งยากสำหรับผู้ใช้ทั่วไปเมื่อเทียบกับ BLE?

_(เขียนคำตอบที่นี่)_

---

## 3. สรุปผลการทดลอง (Conclusion)

_(เขียนสรุปที่นี่)_
