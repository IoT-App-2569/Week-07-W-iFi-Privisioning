# ใบงานที่ 7.3 การคอนฟิก Wi-Fi ผ่าน BLE Scheme และการสืบสวน GATT Services (BLE Forensics)

## 0. กล่าวนำ (Introduction)
**Bluetooth Low Energy (BLE) Provisioning** เป็นรูปแบบมาตรฐานสากลที่อุปกรณ์ Smart Home ชั้นนำ (เช่น Apple HomeKit, Google Home, Matter Protocol) เลือกใช้ เนื่องจากผู้ใช้ไม่ต้องสลับการเชื่อมต่อ Wi-Fi บนสมาร์ตโฟน 

ในใบงานนี้ นักศึกษาจะได้สลับ ESP32 มาทำงานในโหมด **BLE Scheme (`wifi_prov_scheme_ble`)** พร้อมทั้งใช้เครื่องมือวิเคราะห์เชิงลึก **nRF Connect for Mobile** เพื่อส่องดูโครงสร้างภายในของ **GATT Primary Services, 128-bit UUIDs, Characteristics และ Descriptors** ก่อนจะทำการ Provisioning ผ่านแอป **ESP BLE Provisioning**

---

## 1. วัตถุประสงค์ (Objectives)
1. สามารถคอนฟิกตัวอย่าง `wifi_prov_mgr` ให้ทำงานในโหมด **BLE Transport Scheme** ได้สำเร็จ
2. สามารถใช้เครื่องมือ **nRF Connect for Mobile** ในการสแกนและตรวจสอบโครงสร้าง GATT Services/Characteristics ของ Protocomm บน ESP32
3. อ่านและวิเคราะห์ Descriptor `0x2901` (User Characteristic Description) เพื่อระบุชื่อ Protocomm Endpoints
4. ดำเนินการ Provisioning ผ่านแอปพลิเคชัน **ESP BLE Provisioning** และสังเกตการคืนหน่วยความจำ Bluetooth RAM (`BTDM memory released`)

---

## 2. อุปกรณ์และซอฟต์แวร์ที่ใช้ในการทดลอง
1. บอร์ดไมโครคอนโทรลเลอร์ ESP32 พร้อมสาย USB
2. สมาร์ตโฟนที่รองรับ BLE และติดตั้งแอปพลิเคชัน:
   - **nRF Connect for Mobile** (โดย Nordic Semiconductor)
   - **ESP BLE Provisioning** (โดย Espressif)
3. Wi-Fi Access Point ภายในห้องเรียนหรือ Hotspot

---

## 3. สถาปัตยกรรม GATT Services & Endpoints บน BLE Scheme

```mermaid
graph TD
    subgraph BLE_Device["ESP32 BLE Device (Name: PROV_XXXXXX)"]
        direction TB
        Service["Primary Service UUID: 021a9004-0382-4aea-bff4-6b3f1c5adfb4"]
        
        Char1["Characteristic 1 (UUID: ...FF51)<br/>Descriptor 0x2901: 'prov-session'"]
        Char2["Characteristic 2 (UUID: ...FF52)<br/>Descriptor 0x2901: 'prov-config'"]
        Char3["Characteristic 3 (UUID: ...FF53)<br/>Descriptor 0x2901: 'prov-scan'"]
        Char4["Characteristic 4 (UUID: ...FF54)<br/>Descriptor 0x2901: 'proto-ver'"]
        Char5["Characteristic 5 (UUID: ...FF55)<br/>Descriptor 0x2901: 'custom-data'"]
        
        Service --> Char1 & Char2 & Char3 & Char4 & Char5
    end
```

---

## 4. ขั้นตอนการทดลอง (Step-by-Step Procedures)

### ขั้นตอนที่ 1: การเปิดโปรเจกต์ Lab 7-3
1. เปิด Terminal ในโฟลเดอร์โปรเจกต์ `Week-07-W-iFi-Privisioning/Example_codes/Lab7-3-BLE-Provisioning`
2. โค้ดในโปรเจกต์นี้ได้รับการตั้งค่าเปิดใช้งาน **BLE Scheme (NimBLE)** และ **Security 1 (PoP: `abcd1234`)** ไว้เรียบร้อยแล้ว

---

### ขั้นตอนที่ 2: Build, Flash และตรวจสอบสถานะเริ่มต้น
1. สั่งล้าง Flash และ Flash โปรแกรมใหม่:
   ```powershell
   idf.py -p COM24 erase-flash flash monitor
   ```
2. สังเกต Log ใน Serial Monitor:
   ```text
   I (712) wifi_prov_scheme_ble: Starting BLE provisioning
   I (722) app: Starting provisioning
   I (732) app: Scan this QR code from the provisioning application for Provisioning.
   ... [QR Code ASCII & URL] ...
   ```

---

### ขั้นตอนที่ 3: ส่องโครงสร้าง GATT ผ่านแอป nRF Connect (BLE Forensic)
1. เปิดแอป **nRF Connect for Mobile** บนสมาร์ตโฟน
2. แตะปุ่ม **Scan** เพื่อค้นหาอุปกรณ์บลูทูธรอบตัว
3. ค้นหาชื่ออุปกรณ์ที่ขึ้นต้นด้วย `PROV_XXXXXX` (ตรงกับที่ระบุใน Serial Monitor)
4. สังเกตค่า RSSI และแตะปุ่ม **CONNECT** เพื่อเชื่อมต่อ
5. เมื่อเชื่อมต่อสำเร็จ สำรวจดู **GATT Services**:
   - มองหา **Unknown Service** ที่มี Base UUID `021a9004-0382-4aea-bff4-6b3f1c5adfb4`
   - ขยายดูรายการ Characteristics แต่ละตัว
   - สังเกตว่าในแต่ละ Characteristic จะมี Descriptor `Characteristic User Description` (`UUID 0x2901`) แตะดูค่า จะพบชื่อ Endpoint เช่น `"prov-session"`, `"prov-config"`, `"custom-data"`
6. บันทึกภาพหน้าจอและข้อมูล UUIDs ลงในตารางผลการทดลอง
7. กดปุ่ม **DISCONNECT** บนแอป nRF Connect เพื่อปล่อยบอร์ดให้พร้อมรับการ Provision

---

### ขั้นตอนที่ 4: ทำการ Provisioning ด้วยแอป ESP BLE Provisioning
1. เปิดแอป **ESP BLE Provisioning**
2. เลือก "Provision New Device" $\rightarrow$ เลือก "BLE"
3. แตะชื่อบอร์ด `PROV_XXXXXX` (หรือสแกน QR Code)
4. ป้อน PoP เป็น `abcd1234`
5. เลือกเครือข่าย Wi-Fi ในห้องเรียน และป้อนรหัสผ่าน Wi-Fi
6. กด **Provision** และรอจนกระทั่งเชื่อมต่อสำเร็จ

---

### ขั้นตอนที่ 5: สังเกตการปล่อยหน่วยความจำ Bluetooth (Memory Freeing)
สังเกตใน Serial Monitor หลังเชื่อมต่อ Wi-Fi สำเร็จ:
```text
I (24560) app: Provisioning successful
I (24570) wifi_prov_scheme_ble: BT memory released
I (24580) wifi_prov_scheme_ble: BTDM memory released
I (26120) app: Connected with IP Address: 192.168.1.155
```
> **ข้อสังเกต:** บอร์ดจะทำการล้างและคืนหน่วยความจำของ Bluetooth Stack ทั้งหมดคืนสู่ระบบ DRAM ทันที ทำให้ประหยัด RAM ได้มหาศาล!

---

---

## 5. กิจกรรมถอดรหัสซอร์สโค้ดและเขียนผังงาน (Code Deconstruction & BLE GATT Architecture Assignment)

ให้นักศึกษาแกะรอยการทำงานของโมดูล BLE Provisioning ใน `main/main.c` แล้วเขียน **ผังโครงสร้างและลำดับเหตุการณ์**:

### ภารกิจที่ 1: ผังโครงสร้าง GATT Tree & Endpoint Mapping
ให้นักศึกษาวาดโครงสร้างต้นไม้ (Tree Diagram / Block Diagram) แสดงความสัมพันธ์ระหว่าง:
- **Primary Service (128-bit UUID: `021a9004-...`)**
  - **Characteristic UUIDs** แต่ละตัว
  - **Descriptor 0x2901 (User Description)** ที่ผูกเข้ากับ Protocomm Endpoints (`prov-session`, `prov-config`, `prov-scan`, `proto-ver`, `custom-data`)

```mermaid
graph TD
    Service["<b>Primary Service</b><br/>UUID: 021a9004-0382-4aea-bff4-6b3f1b5adfb4"]
    
    Service --> C1["<b>Characteristic: proto-ver</b><br/>UUID: ...ff51"]
    Service --> C2["<b>Characteristic: prov-session</b><br/>UUID: ...ff52"]
    Service --> C3["<b>Characteristic: prov-config</b><br/>UUID: ...ff53"]
    Service --> C4["<b>Characteristic: prov-scan</b><br/>UUID: ...ff54"]
    Service --> C5["<b>Characteristic: custom-data</b><br/>UUID: ...ff55"]

    C1 --> D1["Descriptor 0x2901<br/>User Description: 'proto-ver'"]
    C2 --> D2["Descriptor 0x2901<br/>User Description: 'prov-session'"]
    C3 --> D3["Descriptor 0x2901<br/>User Description: 'prov-config'"]
    C4 --> D4["Descriptor 0x2901<br/>User Description: 'prov-scan'"]
    C5 --> D5["Descriptor 0x2901<br/>User Description: 'custom-data'"]
```

### ภารกิจที่ 2: ผังลำดับการคืนหน่วยความจำ Bluetooth (BLE Lifecycle & Memory Reclaim Flow)
ให้นักศึกษาวาด Flowchart / Sequence แสดงว่า:
1. การเชื่อมต่อ BLE ถูกตรวจพบผ่าน Event `PROTOCOMM_TRANSPORT_BLE_CONNECTED` (LED 2 กระพริบเร็ว 100ms)
2. เมื่อเชื่อมต่อ Wi-Fi สำเร็จ (`WIFI_PROV_CRED_SUCCESS`) $\rightarrow$ เกิด Event `WIFI_PROV_END`
3. Provisioning Manager สั่งเรียก `esp_bt_mem_release()` เพื่อปล่อย DRAM คืนสู่ระบบอย่างไร

```text
[พื้นที่สำหรับแนบรูปภาพ Diagram ที่นักศึกษาเขียนขึ้นด้วย Draw.io / Mermaid / วาดมือ]
```mermaid
flowchart TD
    Start([เริ่มต้น Provisioning Service]) --> Adv[กระจายสัญญาณ BLE Advertising<br/>SSID: 'PROV_0142'<br/>LED 2 กระพริบช้า 500ms]
    Adv --> Connect[สมาร์ตโฟนสแกนเจอและเชื่อมต่อ BLE]
    Connect --> EvtConnect[Event: PROTOCOMM_TRANSPORT_BLE_CONNECTED<br/>LED 2 กระพริบเร็ว 100ms]
    EvtConnect --> Handshake[แลกเปลี่ยน ECDH Session Key & Wi-Fi Credentials]
    Handshake --> ApplyCred[ESP32 Station นำ Credentials ไปเชื่อมต่อ Router]
    ApplyCred --> CheckConn{ผลการเชื่อมต่อ Wi-Fi?}
    CheckConn -- สำเร็จ --> EvtSuccess[Event: WIFI_PROV_CRED_SUCCESS]
    EvtSuccess --> ProvEnd[Event: WIFI_PROV_END<br/>เรียก wifi_prov_mgr_deinit]
    ProvEnd --> ReleaseBT[เรียก esp_bt_mem_release: ESP_BT_MODE_BTDM<br/>ปิด Bluetooth Controller และล้างสแตก BT]
    ReleaseBT --> EndState([คืน DRAM สู่ Heap ~65 KB สำเร็จ<br/>LED 1 สลับเข้าสู่ Heartbeat Mode])
```

---

## 6. ตารางบันทึกผลการทดลอง (Experiment Results)

| รายการตรวจสอบ | ผลการทดลอง / ข้อมูลที่สังเกตได้ |
| :--- | :--- |
| **1. BLE Device Name ที่สแกนเจอ** | `PROV_`.............................. |
| **2. Primary Service UUID (128-bit)** | ..................................................... |
| **3. Characteristic Endpoint ที่พบ (0x2901)** | 1. ..................................................<br/>2. ..................................................<br/>3. .................................................. |
| **4. พฤติกรรมไฟ LED 2 (GPIO 4) ช่วงรอ vs ช่วงต่อ BLE** | ช่วงรอ: .......................................<br/>ช่วงต่อ: ....................................... |
| **5. พฤติกรรมเมื่อต่อ Wi-Fi สำเร็จ** | มี Log คืนหน่วยความจำ Bluetooth หรือไม่? (มี / ไม่มี) |
| **1. BLE Device Name ที่สแกนเจอ** | `PROV_0142` |
| **2. Primary Service UUID (128-bit)** | `021a9004-0382-4aea-bff4-6b3f1b5adfb4` |
| **3. Characteristic Endpoint ที่พบ (0x2901)** | 1. `prov-session`<br/>2. `prov-config`<br/>3. `prov-scan`<br/>4. `proto-ver` |
| **4. พฤติกรรมไฟ LED 2 (GPIO 4) ช่วงรอ vs ช่วงต่อ BLE** | ช่วงรอ: กระพริบช้าเป็นจังหวะ (200ms ON / 800ms OFF) ขณะกำลัง Advertising<br/>ช่วงต่อ: กระพริบเร็วถี่ต่อเนื่อง (100ms ON / 100ms OFF) เมื่อมีอุปกรณ์ BLE เชื่อมต่อเข้ามา |
| **5. พฤติกรรมเมื่อต่อ Wi-Fi สำเร็จ** | **มี** บันทึก Log ยืนยันการคืนแรม: `I (26150) wifi_prov_mgr: Provisioning end` และ `I (26180) bt: Released Bluetooth controller memory to system DRAM` |

---

## 7. คำถามท้ายการทดลอง (Post-Lab Questions)
1. เหตุใด BLE Provisioning จึงไม่ส่งผลให้สัญญาณ Wi-Fi บนสมาร์ตโฟนของผู้ใช้หลุดระหว่างทำรายการ?
2. Descriptor `0x2901` มีความสำคัญอย่างไรต่อการที่แอปพลิเคชันมือถือจะทราบว่า Characteristic แต่ละตัวใช้ทำหน้าที่อะไร?
3. การที่ ESP-IDF มีฟังก์ชัน `esp_bt_mem_release()` มีประโยชน์อย่างไรต่อการทำงานของแอปพลิเคชัน IoT หลังเชื่อมต่อ Wi-Fi สำเร็จ?

1. **เหตุใด BLE Provisioning จึงไม่ส่งผลให้สัญญาณ Wi-Fi บนสมาร์ตโฟนของผู้ใช้หลุดระหว่างทำรายการ?**
   * **ตอบ:** เนื่องจากการสื่อสารระหว่างสมาร์ตโฟนกับ ESP32 ดำเนินการผ่านฮาร์ดแวร์คนละส่วนกันอย่างสิ้นเชิง โดยข้อมูลการตั้งค่าถูกส่งผ่านช่องทางบลูทูธพลังงานต่ำ (Bluetooth Low Energy Interface) จึงไม่กระทบต่ออินเทอร์เฟซ Wi-Fi หลักของสมาร์ตโฟน ผู้ใช้งานจึงยังคงเชื่อมต่ออินเทอร์เน็ตผ่าน Wi-Fi หรือโครงข่าย Cellular 4G/5G ได้อย่างราบรื่นตลอดกระบวนการติดตั้ง

2. **Descriptor `0x2901` มีความสำคัญอย่างไรต่อการที่แอปพลิเคชันมือถือจะทราบว่า Characteristic แต่ละตัวใช้ทำหน้าที่อะไร?**
   * **ตอบ:** Descriptor `0x2901` (Characteristic User Description) เป็นแอตทริบิวต์ตามมาตรฐาน Bluetooth GATT ที่จัดเก็บข้อความสตริงที่มนุษย์และซอฟต์แวร์อ่านเข้าใจได้ เช่น `"prov-session"`, `"prov-config"` ทำให้แอปพลิเคชันมือถือ (ESP BLE Provisioning App) ไม่จำเป็นต้องผูกติดแบบคงที่ (Hardcode) กับรหัส 128-bit UUID ของแต่ละ Characteristic แต่สามารถอ่านข้อความจาก Descriptor 0x2901 เพื่อจับคู่ (Dynamic Mapping) ปลายทางข้อมูลเข้ากับฟังก์ชันของ Protocomm Layer ได้โดยอัตโนมัติ

3. **การที่ ESP-IDF มีฟังก์ชัน `esp_bt_mem_release()` มีประโยชน์อย่างไรต่อการทำงานของแอปพลิเคชัน IoT หลังเชื่อมต่อ Wi-Fi สำเร็จ?**
   * **ตอบ:** ไดรเวอร์และสแตกของระบบ Bluetooth (เช่น BT Controller และ Bluedroid/NimBLE Host) กินพื้นที่หน่วยความจำแรม (DRAM) ภายในชิป ESP32 สูงถึง **60 – 70 กิโลไบต์** เมื่อเสร็จสิ้นกระบวนการ Provisioning แล้ว อุปกรณ์ IoT จะทำงานผ่าน Wi-Fi เป็นหลัก การเรียก `esp_bt_mem_release(ESP_BT_MODE_BTDM)` จะช่วยปลดปล่อยหน่วยความจำส่วนดังกล่าวคืนสู่ Free Heap ของระบบ ส่งผลให้แอปพลิเคชันหลักมีหน่วยความจำคงเหลือเพียงพอสำหรับรองรับการเชื่อมต่อ TLS/HTTPS, บัฟเฟอร์ของคิว FreeRTOS, และโมดูลอื่น ๆ ได้อย่างเสถียร

