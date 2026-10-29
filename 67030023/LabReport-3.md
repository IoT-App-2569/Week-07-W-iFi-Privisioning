## 5. กิจกรรมถอดรหัสซอร์สโค้ดและเขียนผังงาน (Code Deconstruction & BLE GATT Architecture Assignment)

ให้นักศึกษาแกะรอยการทำงานของโมดูล BLE Provisioning ใน `main/main.c` แล้วเขียน **ผังโครงสร้างและลำดับเหตุการณ์**:

### ภารกิจที่ 1: ผังโครงสร้าง GATT Tree & Endpoint Mapping
ให้นักศึกษาวาดโครงสร้างต้นไม้ (Tree Diagram / Block Diagram) แสดงความสัมพันธ์ระหว่าง:
- **Primary Service (128-bit UUID: `021a9004-...`)**
  - **Characteristic UUIDs** แต่ละตัว
  - **Descriptor 0x2901 (User Description)** ที่ผูกเข้ากับ Protocomm Endpoints (`prov-session`, `prov-config`, `prov-scan`, `proto-ver`, `custom-data`)
### ภารกิจที่ 1: ผังโครงสร้าง GATT Tree & Endpoint Mapping

```mermaid
graph TD
    Service["Primary Service<br/>(UUID: 021a9004-0346-4299-80b8-cf524a87efce)"]

    Service --> C1["Characteristic 1"]
    Service --> C2["Characteristic 2"]
    Service --> C3["Characteristic 3"]
    Service --> C4["Characteristic 4"]
    Service --> C5["Characteristic 5"]

    C1 --> D1["Descriptor (0x2901)<br/>User Description: 'prov-session'"]
    C2 --> D2["Descriptor (0x2901)<br/>User Description: 'prov-config'"]
    C3 --> D3["Descriptor (0x2901)<br/>User Description: 'prov-scan'"]
    C4 --> D4["Descriptor (0x2901)<br/>User Description: 'proto-ver'"]
    C5 --> D5["Descriptor (0x2901)<br/>User Description: 'custom-data'"]
### ภารกิจที่ 2: ผังลำดับการคืนหน่วยความจำ Bluetooth (BLE Lifecycle & Memory Reclaim Flow)
ให้นักศึกษาวาด Flowchart / Sequence แสดงว่า:
1. การเชื่อมต่อ BLE ถูกตรวจพบผ่าน Event `PROTOCOMM_TRANSPORT_BLE_CONNECTED` (LED 2 กระพริบเร็ว 100ms)
2. เมื่อเชื่อมต่อ Wi-Fi สำเร็จ (`WIFI_PROV_CRED_SUCCESS`) $\rightarrow$ เกิด Event `WIFI_PROV_END`
3. Provisioning Manager สั่งเรียก `esp_bt_mem_release()` เพื่อปล่อย DRAM คืนสู่ระบบอย่างไร

```text
sequenceDiagram
    autonumber
    actor App as Smartphone App
    participant ESP as ESP32 (Provisioning Manager)
    participant BLE as BLE Stack (NimBLE/Bluedroid)
    participant Heap as System Heap (DRAM)

    App->>ESP: 1. Connect BLE to ESP32
    Note over ESP: Event: PROTOCOMM_TRANSPORT_BLE_CONNECTED<br/>[LED 2: Fast Blink (100ms Mark / 100ms Space)]

    App->>ESP: 2. Send Wi-Fi Credentials via BLE
    ESP->>ESP: Connect to Wi-Fi AP
    Note over ESP: Event: WIFI_PROV_CRED_SUCCESS

    ESP->>ESP: 3. Trigger Event: WIFI_PROV_END
    ESP->>BLE: Stop BLE Provisioning & Disconnect
    Note over ESP: [LED 2: OFF]

    ESP->>BLE: De-init BLE Controller & Host Stack
    ESP->>Heap: 4. Call esp_bt_mem_release(ESP_BT_MODE_BLE)
    Note over Heap: คืนพื้นที่ DRAM จาก BLE Controller<br/>กลับเข้าสู่ระบบ Heap Memory
```

---

## 6. ตารางบันทึกผลการทดลอง (Experiment Results)

| รายการตรวจสอบ | ผลการทดลอง / ข้อมูลที่สังเกตได้ |
| :--- | :--- |
| **1. BLE Device Name ที่สแกนเจอ** | `PROV_6703023` |
| **2. Primary Service UUID (128-bit)** | `021a9004-0346-4299-80b8-cf524a87efce` |
| **3. Characteristic Endpoint ที่พบ (0x2901)** | 1. `prov-session`<br/>2. `prov-config`<br/>3. `prov-scan` |
| **4. พฤติกรรมไฟ LED 2 (GPIO 4) ช่วงรอ vs ช่วงต่อ BLE** | ช่วงรอ: **กระพริบช้า (500ms ON / 500ms OFF)**<br/>ช่วงต่อ: **กระพริบถี่เร็ว (100ms Mark / 100ms Space)** |
| **5. พฤติกรรมเมื่อต่อ Wi-Fi สำเร็จ** | **มี** Log แสดงการคืนหน่วยความจำ Bluetooth (`esp_bt_mem_release`) |

---

## 7. คำถามท้ายการทดลอง (Post-Lab Questions)
1. เหตุใด BLE Provisioning จึงไม่ส่งผลให้สัญญาณ Wi-Fi บนสมาร์ตโฟนของผู้ใช้หลุดระหว่างทำรายการ?
คำตอบ เพราะสมาร์ตโฟนและ ESP32 สื่อสารแลกเปลี่ยนข้อมูลตั้งค่า (Credentials) ผ่านช่องทาง Bluetooth Low Energy (BLE) โดยตรง มือถือจึงยังคงเชื่อมต่อกับโครงข่าย Wi-Fi เดิมหรือใช้ Mobile Data ได้อย่างต่อเนื่อง ไม่จำเป็นต้องตัดสัญญาณ Wi-Fi เพื่อไปเกาะ SoftAP เหมือนวิธี SoftAP Scheme

2. Descriptor 0x2901 มีความสำคัญอย่างไรต่อการที่แอปพลิเคชันมือถือจะทราบว่า Characteristic แต่ละตัวใช้ทำหน้าที่อะไร?
คำตอบ Descriptor 0x2901 (Characteristic User Description) ทำหน้าที่เก็บ String ชื่อ Endpoint (เช่น "prov-session", "prov-config") ทำให้แอปพลิเคชันบนสมาร์ตโฟนสามารถอ่านค่า Descriptor นี้เพื่อค้นหาและระบุได้ทันทีว่า Characteristic แต่ละตัวผูกกับ Protocomm Handler ใด โดยไม่จำเป็นต้อง Hardcode ค่า UUID 128-bit ที่ยาวและซับซ้อนไว้ในตัวแอป

3. การที่ ESP-IDF มีฟังก์ชัน esp_bt_mem_release() มีประโยชน์อย่างไรต่อการทำงานของแอปพลิเคชัน IoT หลังเชื่อมต่อ Wi-Fi สำเร็จ?
คำตอบ ช่วยในการ Reclaim DRAM (คืนหน่วยความจำ RAM) ที่เคยถูกจองไว้สำหรับ BLE Controller และ BLE Stack (ประมาณ 10–30 KB) กลับเข้าสู่ System Free Heap Memory เนื่องจากเมื่ออุปกรณ์ Provisioning สิ้นสุดลงแล้ว BLE จะไม่ได้ถูกใช้อีก การคืน RAM ส่วนนี้จะช่วยให้อุปกรณ์มีหน่วยความจำเพียงพอสำหรับรันแอปพลิเคชันหลัก เช่น MQTT, TLS/SSL Connection หรือ Web Server ได้อย่างมีเสถียรภาพโดยไม่เกิดปัญหามือดับจาก RAM หมด (Out of Memory)
