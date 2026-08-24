## 5. กิจกรรมถอดรหัสซอร์สโค้ดและเขียนผังงาน (Code Deconstruction & Sequence Flow Assignment)

ให้นักศึกษาแกะรอยการทำงานจาก `main/main.c` ในโหมด SoftAP แล้วเขียน **แผนภาพลำดับเหตุการณ์ (Sequence Diagram)**:

### ภารกิจที่ 1: ผังลำดับการสื่อสารผ่าน HTTP Endpoints (SoftAP Scheme Sequence Flow)
ให้นักศึกษาวาด Sequence Diagram แสดงปฏิสัมพันธ์ระหว่าง 3 ฝ่าย:
1. **Smartphone App (ESP SoftAP Prov)**
2. **ESP32 SoftAP Webserver (Protocomm Layer)**
3. **Wi-Fi Router (AP ปลายทาง)**

**จุดที่ต้องระบุในผัง:**
- จังหวะที่มือถือยิง HTTP POST ไปยัง Endpoint แต่ละตัว (`/prov-session`, `/prov-scan`, `/prov-config`)
- Event ของ ESP-IDF ที่ถูก Trigger ใน `event_handler()` เช่น:
  - `WIFI_EVENT_AP_STACONNECTED`
  - `PROTOCOMM_SECURITY_SESSION_SETUP_OK`
  - `WIFI_PROV_CRED_RECV`
  - `WIFI_PROV_CRED_SUCCESS`
  - `IP_EVENT_STA_GOT_IP`
- สถานะจังหวะการกระพริบของ **LED 3 (GPIO 5)** และ **LED 1 (GPIO 2)** ในแต่ละช่วง

![alt text](/Report/img/image-1.png)

---

## 6. ตารางบันทึกผลการทดลอง (Experiment Results)

| รายการตรวจสอบ | ค่าที่บันทึกได้จากการทดลอง |
| :--- | :--- |
| **1. ชื่อ SoftAP SSID ของ ESP32** | PROV_AE1D88 |
| **2. รหัส PoP (Proof of Possession)** | abcd1234 |
| **3. ข้อความใน QR Code Payload (JSON)** | {"ver":"v1","name":"PROV_AE1D88","pop":"abcd1234","security":2,"transport":"softap"} |
| **4. พฤติกรรมไฟ LED 3 (GPIO 5) ช่วงรอ vs ช่วงส่งข้อมูล** | ช่วงรอ: ไฟ LED ติดสว่างค้าง<br/>ช่วงส่ง: ไฟ LED สถานะกระพริบหรือคงสถานะติดสว่างต่อเนื่องเมื่อสมาร์ตโฟนเข้ามาเชื่อมต่อผ่าน SoftAP ทรานสปอร์ต |
| **5. IP Address ที่ ESP32 ได้รับจาก Router** | 192.168.4.1 |
| **6. เวลาที่ใช้ตั้งแต่เริ่มจนจบกระบวนการ (วินาที)** | ~30 - 45 วินาที |

---

## 7. คำถามท้ายการทดลอง (Post-Lab Questions)
1. ในโหมด SoftAP Scheme สมาร์ตโฟนส่งข้อมูลหา ESP32 ผ่านโปรโตคอลและ IP Address ใด?
```
สื่อสารผ่านโปรโตคอล HTTP (REST-like endpoints) โดยปกติ ESP32 จะทำหน้าที่เป็น Access Point และกำหนดหมายเลข IP Address ของตัวเองเป็น 192.168.4.1 สมาร์ตโฟนจึงต้องส่งข้อมูล (POST Request) มายัง IP นี้
```

2. หากผู้ใช้ป้อนรหัสผ่าน Wi-Fi ผิดในแอปมือถือ จะเกิด Event ใดขึ้นบน ESP32 (`WIFI_PROV_CRED_FAIL` หรือไม่) และ ESP32 มีพฤติกรรมอย่างไร?
```
ESP32 จะพยายามนำรหัสผ่านไปเชื่อมต่อกับเร้าเตอร์ เมื่อเร้าเตอร์ปฏิเสธ จะเกิด Event WIFI_EVENT_STA_DISCONNECTED ขึ้นก่อน จากนั้นตัวจัดการ Provisioning จะตรวจสอบพบข้อผิดพลาดและ Trigger Event WIFI_PROV_CRED_FAIL (พร้อมเหตุผล WIFI_PROV_STA_AUTH_ERROR) โดย ESP32 จะไม่ปิดโหมด SoftAP แต่มันจะส่งสถานะ Error กลับไปยังแอปพลิเคชัน เพื่อรอให้ผู้ใช้ป้อนรหัสผ่านใหม่อีกครั้ง
```

3. ทำไมผู้ผลิต IoT ส่วนใหญ่จึงมองว่ากระบวนการเชื่อมต่อแบบ SoftAP มีขั้นตอนที่ยุ่งยากสำหรับผู้ใช้ทั่วไปเมื่อเทียบกับ BLE?
```
การใช้ SoftAP ผู้ใช้จะต้องตัดการเชื่อมต่อจาก Wi-Fi อินเทอร์เน็ตที่ใช้งานอยู่ (หรือ Cellular Data) แล้วสลับไปจับสัญญาณ Wi-Fi ของตัวอุปกรณ์ IoT แทน ซึ่งบ่อยครั้งสมาร์ตโฟนจะแจ้งเตือนว่า "เครือข่ายนี้ไม่มีอินเทอร์เน็ต" และพยายามสลับกลับไปใช้เน็ตมือถือ ทำให้การสื่อสารหลุดกลางคัน ในขณะที่การใช้ BLE (Bluetooth Low Energy) สมาร์ตโฟนสามารถส่งข้อมูลการตั้งค่าไปยังอุปกรณ์ IoT ได้โดยตรงเป็นพื้นหลัง โดยที่ผู้ใช้ไม่ต้องออกจากเครือข่าย Wi-Fi เดิมเลย ทำให้ User Experience (UX) ราบรื่นกว่ามาก
```
