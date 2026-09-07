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

```mermaid
sequenceDiagram
    autonumber
    participant App as Mobile App
    participant ESP as ESP32 (IP 192.168.4.1)
    participant Router as Router

    Note over ESP: เกิด WIFI_PROV_START<br/>(ไฟ LED ขา 5 ติด)
    App->>ESP: เชื่อมต่อ Wi-Fi SSID: PROV_39BD64
    Note over ESP: เกิด WIFI_EVENT_AP_STACONNECTED

    App->>ESP: HTTP POST /prov-session (ส่งรหัส PoP)
    ESP-->>App: HTTP 200 OK

    App->>ESP: HTTP POST /prov-scan (สั่งสแกน Wi-Fi)
    ESP-->>App: ส่งรายชื่อ Wi-Fi บริเวณนั้นกลับไป

    App->>ESP: HTTP POST /prov-config (ส่งรหัสผ่านเป้าหมาย)
    Note over ESP: เกิด WIFI_PROV_CRED_RECV
    ESP-->>App: HTTP 200 OK

    ESP->>Router: ทดลองเชื่อมต่อไปยัง Router
    Router-->>ESP: เชื่อมต่อสำเร็จ แจก IP
    Note over ESP: เกิด IP_EVENT_STA_GOT_IP<br/>(ไฟ LED ขา 2 ติด)

    Note over ESP: เกิด WIFI_PROV_CRED_SUCCESS<br/>(ไฟ LED ขา 5 ดับ)
```

---

## 6. ตารางบันทึกผลการทดลอง (Experiment Results)

| รายการตรวจสอบ                                            | ค่าที่บันทึกได้จากการทดลอง                                                |
| :------------------------------------------------------- | :------------------------------------------------------------------------ |
| **1. ชื่อ SoftAP SSID ของ ESP32**                        | `PROV_39BD64`                                                             |
| **2. รหัส PoP (Proof of Possession)**                    | `abcd1234`                                                                |
| **3. ข้อความใน QR Code Payload (JSON)**                  | `{"ver":"v1","name":"PROV_39BD64","pop":"abcd1234","transport":"softap"}` |
| **4. พฤติกรรมไฟ LED 3 (GPIO 5) ช่วงรอ vs ช่วงส่งข้อมูล** | ตอนรอจะสว่างค้างไว้ พอส่งเสร็จไฟจะดับลง                                   |
| **5. IP Address ที่ ESP32 ได้รับจาก Router**             | `192.168.1.11`                                                            |
| **6. เวลาที่ใช้ตั้งแต่เริ่มจนจบกระบวนการ (วินาที)**      | ประมาณ 68 วินาที                                                          |

---

## 7. คำถามท้ายการทดลอง (Post-Lab Questions)

1. ในโหมด SoftAP Scheme สมาร์ตโฟนส่งข้อมูลหา ESP32 ผ่านโปรโตคอลและ IP Address ใด?
> ผ่านโปรโตคอล HTTP ใช้ IP 192.168.4.1

2. หากผู้ใช้ป้อนรหัสผ่าน Wi-Fi ผิดในแอปมือถือ จะเกิด Event ใดขึ้นบน ESP32 (`WIFI_PROV_CRED_FAIL` หรือไม่) และ ESP32 มีพฤติกรรมอย่างไร?
> เกิด WIFI_PROV_CRED_FAIL บอร์ดจะแจ้ง error ใน log แต่ยังปล่อย Wi-Fi ให้เราใส่รหัสใหม่ได้เรื่อยๆ

3. ทำไมผู้ผลิต IoT ส่วนใหญ่จึงมองว่ากระบวนการเชื่อมต่อแบบ SoftAP มีขั้นตอนที่ยุ่งยากสำหรับผู้ใช้ทั่วไปเมื่อเทียบกับ BLE?
> เพราะเวลาต่อ SoftAP มือถือมักจะฟ้องว่าไม่มีเน็ตแล้วชอบสลับกลับไปใช้ 4G เอง ทำให้หลุดบ่อย ถ้าเป็นบลูทูธมือถือจะไม่ต้องสลับเน็ตเลยทำให้ตั้งค่าง่ายกว่า

---

## Log ผลการทดลอง

```text
I (657) LAB7_2_SOFTAP: Starting SoftAP Provisioning (SSID: PROV_39BD64, PoP: abcd1234)   
I (777) wifi:mode : sta (84:1f:e8:39:bd:64) + softAP (84:1f:e8:39:bd:65)
I (787) esp_netif_lwip: DHCP server started on interface WIFI_AP_DEF with IP: 192.168.4.1
I (807) network_prov_mgr: Provisioning started with service name : PROV_39BD64 
I (817) LAB7_2_SOFTAP: [PROV EVENT]: SoftAP Provisioning Started!
I (857) LAB7_2_SOFTAP: Payload JSON: {"ver":"v1","name":"PROV_39BD64","pop":"abcd1234","transport":"softap"}
I (40697) LAB7_2_SOFTAP: [SOFTAP]: Mobile Phone connected to ESP32 SoftAP!
I (40897) esp_netif_lwip: DHCP server assigned IP to a client, IP is: 192.168.4.2
I (49857) LAB7_2_SOFTAP: [SOFTAP]: Mobile Phone connected to ESP32 SoftAP!
I (49937) esp_netif_lwip: DHCP server assigned IP to a client, IP is: 192.168.4.3
I (62227) LAB7_2_SOFTAP: =================================================
I (62227) LAB7_2_SOFTAP: [CREDENTIALS RECEIVED]:
I (62227) LAB7_2_SOFTAP:   -> Target SSID     : FBT4402_***
I (62227) LAB7_2_SOFTAP:   -> Target Password : **********
I (62237) LAB7_2_SOFTAP: =================================================
I (67267) wifi:connected with FBT4402_***, aid = 3, channel 2, BW20, bssid = 34:4a:c3:**:**:**
I (68667) LAB7_2_SOFTAP: =================================================
I (68667) LAB7_2_SOFTAP: [ONLINE]: Got IP: 192.168.1.***
I (68667) LAB7_2_SOFTAP: =================================================
I (68677) network_prov_mgr: STA Got IP
I (68677) LAB7_2_SOFTAP: [SUCCESS]: Provisioning Completed Successfully!
I (70547) wifi:mode : sta (84:1f:e8:39:bd:64)
I (70557) network_prov_mgr: Provisioning stopped
I (70557) LAB7_2_SOFTAP: [PROV EVENT]: De-initializing Provisioning Manager
```
