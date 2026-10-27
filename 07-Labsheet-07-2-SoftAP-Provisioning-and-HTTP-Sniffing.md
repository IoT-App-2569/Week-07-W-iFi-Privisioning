# ใบงานที่ 7.2 การคอนฟิก Wi-Fi ผ่าน SoftAP Scheme และการวิเคราะห์ Protocomm Endpoints

## 0. กล่าวนำ (Introduction)
ในใบงานนี้ นักศึกษาจะได้ทดลองตั้งค่าเครือข่าย Wi-Fi ให้กับ ESP32 ผ่านช่องทาง **Wi-Fi SoftAP Scheme (`wifi_prov_scheme_softap`)** โดยใช้สมาร์ตโฟนเชื่อมต่อเข้ากับเครือข่ายจำลองที่ ESP32 สร้างขึ้น 

นักศึกษาจะได้เรียนรู้โครงสร้างของ QR Code Payload, การทำงานของ Protocomm ผ่านโปรโตคอล HTTP REST-like Endpoints (`/prov-session`, `/prov-config`), และการส่งข้อมูลการตั้งค่า Wi-Fi จากแอปพลิเคชันมือถือ **ESP SoftAP Provisioning**

---

## 1. วัตถุประสงค์ (Objectives)
1. สามารถคอนฟิกตัวอย่าง `wifi_prov_mgr` ให้ทำงานในโหมด **SoftAP Transport Scheme**
2. สามารถใช้สมาร์ตโฟนเชื่อมต่อและทำ Provisioning ผ่านแอปพลิเคชัน **ESP SoftAP Provisioning** (หรือผ่าน Web Browser QR Code) ได้สำเร็จ
3. สังเกตและวิเคราะห์ Event Sequence Lifecycle ใน Serial Monitor ระหว่างการทำ SoftAP Provisioning
4. เข้าใจการทำงานของ Protocomm Endpoint ในระดับ Application Layer

---

## 2. อุปกรณ์และซอฟต์แวร์ที่ใช้ในการทดลอง
1. บอร์ดไมโครคอนโทรลเลอร์ ESP32 พร้อมสาย USB
2. สมาร์ตโฟน (Android หรือ iOS) ที่ติดตั้งแอปพลิเคชัน **ESP SoftAP Provisioning** (หรือแอปกล้องสแกน QR Code)
3. Wi-Fi Access Point ภายในห้องเรียนหรือ Hotspot จากสมาร์ตโฟนอีกเครื่อง

---

## 3. สถาปัตยกรรมและแผนภาพลำดับเหตุการณ์ (Sequence Diagram)

```mermaid
sequenceDiagram
    autonumber
    participant App as 📱 Mobile App (ESP SoftAP Prov)
    participant ESP as ⚡ ESP32 (SoftAP Mode)
    participant Router as 📡 Home/Lab Router AP

    Note over ESP: เริ่มต้น SoftAP SSID: PROV_XXXXXX<br/>IP: 192.168.4.1
    App->>ESP: เชื่อมต่อ Wi-Fi SSID: PROV_XXXXXX
    App->>ESP: HTTP POST /prov-session (Security Handshake + PoP: abcd1234)
    ESP-->>App: HTTP 200 OK (Secured Session Ready)
    
    App->>ESP: HTTP POST /prov-scan (สั่ง ESP32 สแกนหา Wi-Fi รอบตัว)
    ESP-->>App: รายชื่อ Wi-Fi SSIDs + ค่า RSSI
    
    App->>ESP: HTTP POST /prov-config (ส่ง SSID เป้าหมาย + รหัสผ่าน)
    ESP->>Router: ทดลองเชื่อมต่อไปยัง Router
    Router-->>ESP: เชื่อมต่อสำเร็จ ได้รับ IP Address
    ESP-->>App: HTTP 200 OK (Provisioning Success)
    Note over ESP: ปิด SoftAP และเริ่มทำงานในโหมด Station ต่อเนื่อง
```

---

## 4. ขั้นตอนการทดลอง (Step-by-Step Procedures)

### ขั้นตอนที่ 1: การเปิดโปรเจกต์ Lab 7-2
1. เปิด Terminal ในโฟลเดอร์โปรเจกต์ `Week-07-W-iFi-Privisioning/Example_codes/Lab7-2-SoftAP-Provisioning`
2. โค้ดในโปรเจกต์นี้ได้รับการตั้งค่าเป็น **SoftAP Scheme** และ **Security 1 (PoP: `abcd1234`)** ไว้ล่วงหน้าเรียบร้อยแล้ว

---

### ขั้นตอนที่ 2: การ Flash และสังเกต QR Code
1. สั่งล้าง Flash และ Flash โปรแกรม:
   ```powershell
   idf.py -p COM24 erase-flash flash monitor
   ```
2. สังเกต Log ใน Serial Monitor จะปรากฏข้อความและ QR Code:
   ```text
   I (776) wifi_prov_scheme_softap: Starting SoftAP with SSID: PROV_XXXXXX
   I (786) app: Scan this QR code from the provisioning application for Provisioning.
   ... [รูป QR Code แบบ ASCII Text] ...
   I (816) app: If QR code is not visible, copy paste the below URL in a browser.
   https://espressif.github.io/esp-jumpstart/qrcode.html?data={"ver":"v1","name":"PROV_XXXXXX","pop":"abcd1234","transport":"softap"}
   ```

---

### ขั้นตอนที่ 3 ดำเนินการ Provisioning ผ่านสมาร์ตโฟน
1. เปิดแอป **ESP SoftAP Provisioning** บนสมาร์ตโฟน
2. **วิธีที่ A (สแกน QR Code)** แตะปุ่ม "Scan QR Code" แล้วสแกนภาพ QR Code บนหน้าจอ Serial Monitor (หรือเปิดผ่าน URL ที่ได้จาก Log)
3. **วิธีที่ B (เชื่อมต่อ Manual)**
   - ไปที่การตั้งค่า Wi-Fi บนมือถือ เชื่อมต่อ Wi-Fi ชื่อ `PROV_XXXXXX`
   - เปิดแอป กด "Provision" และป้อน PoP เป็น `abcd1234`
4. เมื่อแอปค้นหา ESP32 พบ ให้เลือกชื่อ Wi-Fi ภายในห้องเรียนหรือ Hotspot ที่ต้องการเชื่อมต่อ และป้อนรหัสผ่าน Wi-Fi
5. กดปุ่ม **Provision** และสังเกตแถบสถานะบนแอปจนกระทั่งขึ้น **"Provisioning Successful!"**

---

### ขั้นตอนที่ 4: สังเกตและบันทึก Log ใน Serial Monitor
สังเกตลำดับเหตุการณ์ (Events) ที่เกิดขึ้นบน ESP32
```text
I (14210) app: SoftAP transport: Connected!
I (15320) app: Secured session established!
I (16440) app: Received Wi-Fi credentials
	SSID     : Lab_WiFi_2.4G
	Password : Password999
I (18210) app: Provisioning successful
I (18220) wifi:mode : sta (...)
I (19850) app: Connected with IP Address: 192.168.1.150
```

---

---

## 5. กิจกรรมถอดรหัสซอร์สโค้ดและเขียนผังงาน (Code Deconstruction & Sequence Flow Assignment)

ให้นักศึกษาแกะรอยการทำงานจาก `main/main.c` ในโหมด SoftAP แล้วเขียน **แผนภาพลำดับเหตุการณ์ (Sequence Diagram)**:

### ภารกิจที่ 1: ผังลำดับการสื่อสารผ่าน HTTP Endpoints (SoftAP Scheme Sequence Flow)
 Sequence Diagram แสดงปฏิสัมพันธ์ระหว่าง 3 ฝ่าย:
1. **Smartphone App (ESP SoftAP Prov)**
2. **ESP32 SoftAP Webserver (Protocomm Layer)**
3. **Wi-Fi Router (AP ปลายทาง)**

**จุดที่ระบุในผัง:**
- จังหวะที่มือถือยิง HTTP POST ไปยัง Endpoint แต่ละตัว (`/prov-session`, `/prov-scan`, `/prov-config`)
- Event ของ ESP-IDF ที่ถูก Trigger ใน `event_handler()`:
  - `NETWORK_PROV_START`
  - `WIFI_EVENT_AP_STACONNECTED`
  - `PROTOCOMM_SECURITY_SESSION_SETUP_OK`
  - `NETWORK_PROV_WIFI_CRED_RECV`
  - `IP_EVENT_STA_GOT_IP`
  - `NETWORK_PROV_WIFI_CRED_SUCCESS`
  - `NETWORK_PROV_END`
- สถานะของ **LED 3 (GPIO 5 - SoftAP)** และ **LED 1 (GPIO 2 - STA)** ในแต่ละช่วงกระบวนการ

![Sequence Diagram การสื่อสารผ่าน HTTP Endpoints ในโหมด SoftAP](sequence_diagram_lab7_2.png)

---

## 6. ตารางบันทึกผลการทดลอง (Experiment Results)

| รายการตรวจสอบ | ค่าที่บันทึกได้จากการทดลอง |
| :--- | :--- |
| **1. ชื่อ SoftAP SSID ของ ESP32** | `PROV_38D0EC` |
| **2. รหัส PoP (Proof of Possession)** | `abcd1234` |
| **3. ข้อความใน QR Code Payload (JSON)** | `{"ver":"v1","name":"PROV_38D0EC","pop":"abcd1234","transport":"softap"}` |
| **4. พฤติกรรมไฟ LED 3 (GPIO 5) ช่วงรอ vs ช่วงส่งข้อมูล** | ช่วงรอ: สว่างค้าง (Logic HIGH) แสดงสถานะเปิด SoftAP พร้อมรับข้อมูล<br/>ช่วงส่งข้อมูล/สำเร็จ: ดับลง (Logic LOW) เมื่อ Provisioning สำเร็จและปิด SoftAP |
| **5. IP Address ที่ ESP32 ได้รับจาก Router** | `10.15.7.72` (เชื่อมต่อกับ SSID: `KMITL-WIFI`) |
| **6. เวลาที่ใช้ตั้งแต่เริ่มจนจบกระบวนการ (วินาที)** | ประมาณ 15 วินาที (ใช้เวลาเชื่อมต่อและรับ IP สำเร็จใน 7.23 วินาทีหลังส่งค่า Wi-Fi) |

### รูปภาพประกอบการทดลองจริงจากสมาร์ตโฟน

| 1. เชื่อมต่อ Wi-Fi เข้ากับ SoftAP (`PROV_38D0EC`) | 2. หน้าจอแอปพลิเคชันค้นหาและเลือกเครือข่าย Wi-Fi |
| :---: | :---: |
| ![เชื่อมต่อ Wi-Fi SoftAP PROV_38D0EC](softap_wifi_connected.jpg) | ![ค้นหาและเลือกเครือข่าย Wi-Fi KMITL-WIFI](softap_select_network.jpg) |

---

## 7. คำถามท้ายการทดลอง (Post-Lab Questions)

### 1. ในโหมด SoftAP Scheme สมาร์ตโฟนส่งข้อมูลหา ESP32 ผ่านโปรโตคอลและ IP Address ใด?
**คำตอบ:**  
ในโหมด SoftAP Scheme สมาร์ตโฟนส่งข้อมูลสื่อสารกับ ESP32 ผ่านโปรโตคอล **HTTP (Hypertext Transfer Protocol)** ในรูปแบบ REST-like API โดยใช้คำสั่ง HTTP POST ในการส่งข้อมูล Payload ที่ถูกเข้ารหัสและจัดโครงสร้างด้วย Protocol Buffers (Protobuf) ตามมาตรฐานความปลอดภัยของ Protocomm Layer  
โดยส่งไปยังหมายเลข IP Address ค่าเริ่มต้นของ ESP32 ในโหมด SoftAP คือ **192.168.4.1** (พอร์ตมาตรฐาน 80) ซึ่ง ESP32 ทำหน้าที่เป็น Default Gateway และรันบริการ DHCP Server เพื่อแจกหมายเลข IP ให้อยู่ในเครือข่ายย่อยเดียวกันกับสมาร์ตโฟน (เช่น 192.168.4.2)

---

### 2. หากผู้ใช้ป้อนรหัสผ่าน Wi-Fi ผิดในแอปมือถือ จะเกิด Event ใดขึ้นบน ESP32 (`WIFI_PROV_CRED_FAIL` หรือไม่) และ ESP32 มีพฤติกรรมอย่างไร?
**คำตอบ:**  
เมื่อผู้ใช้ป้อนรหัสผ่าน Wi-Fi ไม่ถูกต้อง จะเกิดพฤติกรรมและลำดับเหตุการณ์ดังนี้:
1. **การเกิด Event ในระบบ:** ระบบจะตรวจจับความล้มเหลวในการเชื่อมต่อ Wi-Fi และเกิด Event **`NETWORK_PROV_WIFI_CRED_FAIL`** (หรือ `WIFI_PROV_CRED_FAIL`) ภายใน Provisioning Manager
2. **พฤติกรรมของ ESP32:**
   - ภาค Wi-Fi Station ของ ESP32 จะพยายามทำการเชื่อมต่อกับ Router ปลายทาง แต่ล้มเหลวในขั้นตอน 4-Way Handshake Authentication (`WIFI_REASON_AUTH_FAIL`)
   - ESP32 จะส่งข้อความตอบกลับความล้มเหลว (Failure Response Code) ผ่านทาง Protocomm Endpoint `/prov-config` กลับไปยังแอปพลิเคชันบนสมาร์ตโฟน เพื่อให้แอปพลิเคชันแจ้งเตือนผู้ใช้งานทันที
   - ESP32 **จะไม่ทำการปิด SoftAP** โดยยังคงรักษาการทำงานของ SoftAP และรอรับการตั้งค่ารอบใหม่ เพื่อให้ผู้ใช้สามารถแก้ไขชื่อ SSID หรือรหัสผ่านแล้วลองเชื่อมต่อใหม่อีกครั้งได้โดยไม่ต้องเริ่มรีเซ็ตระบบใหม่

---

### 3. ทำไมผู้ผลิต IoT ส่วนใหญ่จึงมองว่ากระบวนการเชื่อมต่อแบบ SoftAP มีขั้นตอนที่ยุ่งยากสำหรับผู้ใช้ทั่วไปเมื่อเทียบกับ BLE?
**คำตอบ:**  
ผู้ผลิตผลิตภัณฑ์ IoT มองว่าการทำ Provisioning ด้วย SoftAP มีความยุ่งยากและส่งผลต่อประสบการณ์ผู้ใช้งาน (User Experience) มากกว่าการใช้ BLE ด้วยสาเหตุดังต่อไปนี้:
1. **ต้องสลับเครือข่าย Wi-Fi ด้วยตนเอง (Manual Network Switching):** ผู้ใช้งานต้องออกจากแอปพลิเคชัน ไปที่หน้าเมนูตั้งค่า Wi-Fi ของโทรศัพท์ เพื่อตัดการเชื่อมต่อจาก Wi-Fi อินเทอร์เน็ตเดิม และเชื่อมต่อกับ Access Point ชั่วคราวของบอร์ด ESP32 ซึ่งเป็นขั้นตอนที่ซับซ้อนสำหรับผู้ใช้ทั่วไป
2. **ปัญหาระบบตรวจสอบการไม่มีอินเทอร์เน็ต (Captive Portal / No Internet Fallback):** ระบบปฏิบัติการสมาร์ตโฟนสมัยใหม่ (ทั้ง iOS และ Android) จะตรวจสอบการเข้าถึงอินเทอร์เน็ต เมื่อพบว่าเครือข่าย SoftAP ของอุปกรณ์ไม่มีการเชื่อมต่ออินเทอร์เน็ต ระบบมักจะตัดการเชื่อมต่อกลับไปใช้ Cellular Data (4G/5G) โดยอัตโนมัติ หรือแสดงข้อความเตือนความปลอดภัย ทำให้การส่งข้อมูลขาดช่วง
3. **ขาดความต่อเนื่องในการใช้งาน (Lack of Seamless Background Workflow):** ต่างจากเทคโนโลยี Bluetooth Low Energy (BLE) ที่แอปพลิเคชันสามารถค้นหา จับคู่ และส่งข้อมูล Wi-Fi Credentials ได้โดยอัตโนมัติจากเบื้องหลัง (Background) ภายในแอปพลิเคชัน โดยที่โทรศัพท์ไม่ต้องตัดสัญญาณอินเทอร์เน็ตและผู้ใช้ไม่ต้องสลับหน้าจอเลย

