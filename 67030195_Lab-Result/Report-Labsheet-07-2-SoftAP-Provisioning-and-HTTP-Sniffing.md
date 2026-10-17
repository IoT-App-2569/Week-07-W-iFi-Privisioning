# Report ใบงานที่ 7.2 การคอนฟิก Wi-Fi ผ่าน SoftAP Scheme และการวิเคราะห์ Protocomm Endpoints
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

<img width="974" height="1022" alt="แบบแผนที่ยังไม่ได้ตั้งชื่อ drawio" src="https://github.com/user-attachments/assets/9521175a-2f81-41f9-b630-448a43650655" />


---

## 6. ตารางบันทึกผลการทดลอง (Experiment Results)

| รายการตรวจสอบ | ค่าที่บันทึกได้จากการทดลอง |
| :--- | :--- |
| **1. ชื่อ SoftAP SSID ของ ESP32** | `PROV_A550C4` |
| **2. รหัส PoP (Proof of Possession)** | abcd1234 |
| **3. ข้อความใน QR Code Payload (JSON)** | Payload: {"ver":"v1","name":"PROV_A550C4","pop":"abcd1234","transport":"softap"} |
| **4. พฤติกรรมไฟ LED 3 (GPIO 5) ช่วงรอ vs ช่วงส่งข้อมูล** | ช่วงรอ: LED สว่างค้าง <br/>ช่วงส่ง: LED ไฟอาจจะกระพริบเร็วขึ้น  |
| **5. IP Address ที่ ESP32 ได้รับจาก Router** | 192.168.4.1 |
| **6. เวลาที่ใช้ตั้งแต่เริ่มจนจบกระบวนการ (วินาที)** | ประมาณ 45 วินาที |

---

## 7. คำถามท้ายการทดลอง (Post-Lab Questions)
1. ในโหมด SoftAP Scheme สมาร์ตโฟนส่งข้อมูลหา ESP32 ผ่านโปรโตคอลและ IP Address ใด?

   **ตอบ** ส่งผ่านโปรโตคอล HTTP (HTTP REST-like Endpoints) โดยเรียกไปยัง IP Address 192.168.4.1 ซึ่งเป็นค่า Default IP ของ ESP32 ในโหมด SoftAP
   
2. หากผู้ใช้ป้อนรหัสผ่าน Wi-Fi ผิดในแอปมือถือ จะเกิด Event ใดขึ้นบน ESP32 (`WIFI_PROV_CRED_FAIL` หรือไม่) และ ESP32 มีพฤติกรรมอย่างไร?

   **ตอบ** จะเกิด Event WIFI_PROV_CRED_FAIL ขึ้น โดย ESP32 จะแจ้งผลการเชื่อมต่อล้มเหลวกลับไปยังแอปพลิเคชัน และจะยังคงเปิดโหมด SoftAP ต่อไปเพื่อรอรับรหัสผ่านใหม่ที่ถูกต้องโดยไม่ต้องเริ่มกระบวนการทั้งหมดใหม่ตั้งแต่ต้น
   
3. ทำไมผู้ผลิต IoT ส่วนใหญ่จึงมองว่ากระบวนการเชื่อมต่อแบบ SoftAP มีขั้นตอนที่ยุ่งยากสำหรับผู้ใช้ทั่วไปเมื่อเทียบกับ BLE?

   **ตอบ** แบบ SoftAP ยุ่งยากกว่า BLE เพราะผู้ใช้ต้องสลับหน้าจอไปต่อ Wi-Fi ของอุปกรณ์จำลองก่อน ทำให้การเชื่อมต่ออินเทอร์เน็ตปกติบนมือถือถูกตัดขาดชั่วคราวระหว่างการตั้งค่าและมีขั้นตอนการสลับหน้าจอตั้งค่าไปมา


