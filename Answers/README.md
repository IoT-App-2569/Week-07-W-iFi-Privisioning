# คำตอบใบงานสัปดาห์ที่ 7 — Wi-Fi Provisioning

| รหัสนักศึกษา | 67030098 |
| :--- | :--- |
| **Branch** | `67030098-HW` |
| **ESP-IDF** | v6.0.2 |
| **บอร์ด** | ESP32-D0WD-V3 (rev v3.1), MAC `14:33:5c:0e:01:d0` |

## รายการไฟล์คำตอบ

1. [Lab7-1 — Reset Mechanisms & NVS Forensics](Lab7-1-Answers.md)
2. [Lab7-2 — SoftAP Provisioning & HTTP Sniffing](Lab7-2-Answers.md)
3. [Lab7-3 — BLE Provisioning & GATT Forensics](Lab7-3-Answers.md)
4. [Lab7-4 — Security Schemes & Custom Data](Lab7-4-Answers.md)

## หมายเหตุการทดลอง

โค้ดตัวอย่างทั้ง 4 แล็บใน `codes/` ถูกปรับมาใช้ **ESP-IDF v6.0.2** แล้ว
โดยเปลี่ยนจากคอมโพเนนต์ `wifi_provisioning` (ถูกถอดออกจาก IDF core ตั้งแต่ v6.0)
ไปใช้ Managed Component `espressif/network_provisioning` v1.2.4
