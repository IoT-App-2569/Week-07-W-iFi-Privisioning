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

พบและแก้บั๊กจริงในโค้ดต้นฉบับของ Lab 7-2: `led_task()` ไม่เคยถูกเรียกด้วย `xTaskCreate()`
ทำให้ GPIO 2/5 ไม่เคยถูกตั้งเป็น output — แก้แล้วโดยย้าย `gpio_config()` เข้า `app_main()`

## หมายเหตุเรื่องความสมบูรณ์ของข้อมูล

ทุกจุดในเอกสารชุดนี้ใช้ **ข้อมูลจริง** ที่ยืนยันได้จาก log ที่จับได้บนบอร์ดจริงหรือจากซอร์สโค้ดจริง
ยกเว้น **2 กรณีใน Lab 7-4** (PoP ผิด และ Custom Data) ที่ยังไม่ได้ทดสอบบนฮาร์ดแวร์จริง — สองกรณีนี้เติมด้วย
**ข้อมูลจำลอง (Mockup/Simulated)** กำกับด้วยเครื่องหมาย ⚠️ ทุกจุดที่ปรากฏ โดยสร้างจากรูปแบบ log จริงในซอร์สโค้ด
(TAG, ข้อความ ESP_LOG ที่ตรงกับที่เขียนไว้จริงในไฟล์) แต่ **ค่า timestamp เป็นตัวเลขสมมติ**
สามารถยืนยันด้วยข้อมูลจริงภายหลังได้โดยสลับแอปไปโหมด "Provision Manually" (สำหรับ PoP ผิด)
และเข้าเมนู Custom Data ในแอปหลัง session สำเร็จ (สำหรับ Custom Data)

นอกจากนี้ Lab 7-2 ยังขาดข้อมูลจริง 2 จุด (IP ที่ ESP32 ได้รับจาก router และเวลารวมแบบ end-to-end)
เนื่องจาก log ที่บันทึกไว้ตัดตอนก่อนถึงจุดนั้น — จุดนี้**ไม่ได้เติมด้วยข้อมูลจำลอง** ยังคงระบุตรงไปตรงมาว่าขาดข้อมูล
