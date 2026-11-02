# สรุปเข้มเนื้อหาและประเด็นสำคัญสำหรับสอบ (Exam Preparation Guide)
## วิชา IoT System Development: สัปดาห์ที่ 7 (Week 07)
### หัวข้อ: Wi-Fi Provisioning Architecture, Security Schemes & Memory Forensics

---

## สารบัญเนื้อหา (Table of Contents)
1. **ภาพรวมและปัญหา Headless IoT (The Headless IoT Problem)**
2. **สถาปัตยกรรม Wi-Fi Provisioning และ Protocomm ใน ESP-IDF**
3. **Endpoint มาตรฐานและวงจรชีวิต State Machine**
4. **ตารางเปรียบเทียบเชิงลึก: SoftAP vs. BLE Transport Scheme**
5. **ระดับความปลอดภัยและรหัสวิทยา: Security 0 vs. Security 1 vs. Security 2**
6. **สถาปัตยกรรมหน่วยความจำ Flash และการตรวจพิสูจน์พยานหลักฐาน (NVS Forensics)**
7. **กลไกการ Reset ค่าคอนฟิกทั้ง 3 รูปแบบ และข้อควรระวัง Strapping Pin**
8. **การประยุกต์ใช้งาน Custom Data Endpoint และการจัดการหน่วยความจำ Heap**
9. **แนวข้อสอบและคำถามท้ายการทดลอง (Q&A พร้อมเฉลยละเอียดสำหรับเข้าสอบ)**

---

## 1. ภาพรวมและปัญหา Headless IoT (The Headless IoT Problem)

### 1.1 นิยามของ Headless IoT Device
* **Headless Device** คือ อุปกรณ์ฝังตัวหรืออุปกรณ์ IoT ที่ **"ไม่มีจอแสดงผล (Display) และไม่มีแป้นพิมพ์ (Keyboard)"** สำหรับให้มนุษย์ป้อนข้อมูลโดยตรง เช่น หลอดไฟอัจฉริยะ, ปลั๊กไฟ, เซนเซอร์สิ่งแวดล้อม, IP Camera
* **ปัญหาสำคัญ (The Core Problem):** เมื่อแกะกล่องอุปกรณ์ใหม่ (Out-of-the-Box Experience: OOBE) อุปกรณ์จะต้องเชื่อมต่อเข้ากับ Home/Office Wi-Fi Router เพื่อออกอินเทอร์เน็ต แต่อุปกรณ์ไม่มีทางรู้ชื่อเครือข่าย (**SSID**) และรหัสผ่าน (**Wi-Fi Password / Pre-Shared Key**) ได้ด้วยตนเอง

### 1.2 ความหมายของ Wi-Fi Provisioning
* **Wi-Fi Provisioning** คือ กระบวนการส่งมอบและตั้งค่าข้อมูลเครือข่าย Wi-Fi (รวมถึง Credentials, Security Keys, และข้อมูลตั้งค่าเริ่มต้น) จากอุปกรณ์ผู้ช่วย (Provisioner Client เช่น Smartphone, Tablet หรือ PC) ไปยังอุปกรณ์ IoT เป้าหมาย (Provisioning Target เช่น ESP32) ผ่านช่องทางสื่อสารระยะใกล้ชั่วคราว (เช่น SoftAP หรือ BLE) ภายใต้โปรโตคอลการรักษาความปลอดภัย

---

## 2. สถาปัตยกรรม Wi-Fi Provisioning และ Protocomm ใน ESP-IDF

สถาปัตยกรรม Provisioning ใน ESP-IDF ออกแบบเป็น **Modular & Layered Architecture** แบ่งเป็น 4 เลเยอร์หลัก:

| ลำดับเลเยอร์ | ชื่อเลเยอร์ | องค์ประกอบและหน้าที่สำคัญ |
| :---: | :--- | :--- |
| **1** | **Application Layer** | โค้ดของผู้พัฒนา (`app_main`), Event Handler ดักจับสถานะ, และฟังก์ชัน Callback สำหรับ Custom Data Endpoints |
| **2** | **Provisioning Manager (`wifi_prov_mgr`)** | ควบคุม Finite State Machine (FSM), จัดการวงจรชีวิตการทำงาน, จัดการ Serialization ด้วย **Protobuf (Nanopb)** |
| **3** | **Protocomm Abstraction Layer** | เลเยอร์การสื่อสารนามธรรม ทำหน้าที่: <br/>1) **Endpoint Registration** (สร้างช่องทางรับส่งข้อมูล)<br/>2) **Security Handshake & Encryption** (เข้ารหัส/ถอดรหัสข้อความ)<br/>3) **Message Framing & Dispatching** โดยไม่ขึ้นกับสื่อส่งสัญญาณ (Transport Agnostic) |
| **4** | **Transport & Hardware Layer** | - **Transport Schemes:** SoftAP (HTTP Server), BLE (GATT), หรือ Console (UART)<br/>- **Hardware Drivers:** Wi-Fi Driver, Bluetooth Driver, NVS Flash (Offset 0x9000) |

### 2.1 ความสำคัญของ Nanopb (Google Protocol Buffers)
* ทำไมไม่ใช้ JSON หรือ XML ในเลเยอร์ Protocomm?
  * **ประหยัด Bandwidth:** Protobuf แปลงโครงสร้างข้อมูลเป็นไบนารีขนาดเล็กมาก (Compact Binary Format)
  * **ประหยัด RAM/CPU:** ใช้ไลบรารี `nanopb` ซึ่งเป็นโค้ดภาษา C ขนาดเล็กมาก ออกแบบมาสำหรับไมโครคอนโทรลเลอร์ที่มีทรัพยากรจำกัด ป้องกันปัญหา RAM ไม่พอ (Out of Memory)

### 2.2 การสร้าง Dynamic Device Name
* เพื่อป้องกันปัญหาชื่อชนกันในห้องเรียนหรือจุดติดตั้งหนาแน่น ESP32 ใช้วิธีนำ **MAC Address 3 ไบต์สุดท้าย** มาต่อท้าย Prefix เช่น `PROV_0ED054`
  * ในโหมด SoftAP $\rightarrow$ ใช้เป็นชื่อ **Wi-Fi SSID**
  * ในโหมด BLE $\rightarrow$ ใช้เป็นชื่อ **BLE Device / Advertising Name**

---

## 3. Endpoint มาตรฐานและวงจรชีวิต State Machine

### 3.1 Provisioning Endpoints มาตรฐาน (หัวใจสำคัญในการออกสอบ)
ในระบบ Protocomm ข้อมูลจะถูกส่งผ่านช่องทางที่เรียกว่า **Endpoint**:

| ชื่อ Endpoint | หน้าที่และข้อมูลที่รับส่ง | หมายเหตุ / โปรโตคอล |
| :--- | :--- | :--- |
| `prov-session` | ใช้สำหรับเจรจาและสร้าง Security Session แลกเปลี่ยนคีย์ (ECDH / X25519) และตรวจสอบสิทธิ์ผ่าน PoP / SRP6a | ต้องทำสำเร็จก่อน Endpoint อื่นจึงจะเปิดให้ส่งข้อมูลได้ |
| `prov-config` | ใช้ส่งมอบข้อมูล Wi-Fi Configuration (SSID, Password, BSSID) และส่งคำสั่งสั่งให้ ESP32 ทดลองเชื่อมต่อไปยัง Router | ข้อมูลถูกเข้ารหัสทั้งหมด (หากเปิด Sec1 หรือ Sec2) |
| `prov-scan` | สั่งให้ ESP32 สแกนหา Wi-Fi AP รอบตัว และส่งรายชื่อ SSID พร้อมค่าความแรงสัญญาณ (RSSI) กลับไปแสดงบนมือถือ | ช่วยให้ผู้ใช้เลือกกดเชื่อมต่อได้โดยไม่ต้องพิมพ์ SSID เอง |
| `proto-ver` | ตรวจสอบหมายเลขเวอร์ชันของ Provisioning Protocol และความสามารถที่อุปกรณ์รองรับ | ให้แอปพลิเคชันมือถือทราบขีดความสามารถของเฟิร์มแวร์ |
| `custom-data` *(Optional)* | ส่งข้อมูลเฉพาะของแอปพลิเคชัน เช่น Device Serial, MQTT Broker URL, Cloud Auth Token, User/Tenant ID | ผู้พัฒนาสามารถสร้างและผูกฟังก์ชัน Callback เองได้ |

### 3.2 วงจรชีวิตของ Provisioning State Machine (FSM Lifecycle)
1. **Bootup:** บูตเครื่อง $\rightarrow$ เรียก `wifi_prov_mgr_is_provisioned(&provisioned)`
   * ถ้า `provisioned == true` (เคยตั้งค่าแล้ว) $\rightarrow$ ข้ามไปโหมด Station เชื่อมต่อ Wi-Fi ทันที
   * ถ้า `provisioned == false` (อุปกรณ์ใหม่/เพิ่งรีเซ็ต) $\rightarrow$ เริ่มต้นเปิด Provisioning Service
2. **Start Service:** เรียก `wifi_prov_mgr_start_provisioning()` เริ่มกระจายสัญญาณ SoftAP หรือ BLE Advertising
3. **Session Handshake:** ฝั่งมือถือเชื่อมต่อเข้ามาและยิงข้อมูลมาที่ `prov-session` เพื่อแลกเปลี่ยนกุญแจและตรวจสอบ PoP
4. **Send Credentials:** มือถือส่ง SSID + Password ผ่าน `prov-config`
5. **Connect AP:** ESP32 ทดลองต่อเข้า Wi-Fi Router
   * **เชื่อมต่อสำเร็จ:** Trigger `WIFI_PROV_CRED_SUCCESS` $\rightarrow$ บันทึก Credentials ลง NVS Flash $\rightarrow$ Trigger `WIFI_PROV_END` $\rightarrow$ คืนหน่วยความจำ Bluetooth/SoftAP ให้ระบบ $\rightarrow$ ได้รับ IP Address (`IP_EVENT_STA_GOT_IP`)
   * **เชื่อมต่อล้มเหลว:** Trigger `WIFI_PROV_CRED_FAIL` (เช่น พิมพ์รหัสผิด หรือสัญญาณหลุด) $\rightarrow$ รายงานสถานะกลับไปยังแอปมือถือเพื่อรอให้ผู้ใช้ป้อนรหัสใหม่

### 3.3 รายการ Event สำคัญของระบบ Provisioning

| ชื่อ Event | ความหมายและสิ่งที่เกิดขึ้น |
| :--- | :--- |
| `WIFI_PROV_START` | Provisioning Service เริ่มทำงาน (กำลังปล่อย SoftAP หรือ BLE Advertising) |
| `PROTOCOMM_SECURITY_SESSION_SETUP_OK` | การแลกเปลี่ยนคีย์และยืนยันตัวตนสำเร็จ Security Session เปิดพร้อมใช้งาน |
| `PROTOCOMM_SECURITY_SESSION_CREDENTIALS_MISMATCH` | ผู้ใช้ป้อนรหัส **PoP ผิดพลาด** การสร้าง Session ล้มเหลว |
| `WIFI_PROV_CRED_RECV` | ได้รับข้อมูล SSID และ Wi-Fi Password จากสมาร์ตโฟนแล้ว |
| `WIFI_PROV_CRED_FAIL` | การทดลองเชื่อมต่อ Wi-Fi ล้มเหลว (รหัสผ่านผิด หรือค้นหา AP ไม่เจอ) |
| `WIFI_PROV_CRED_SUCCESS` | เชื่อมต่อ Router สำเร็จ และระบบบันทึกค่าลง NVS Flash เรียบร้อย |
| `WIFI_PROV_END` | ปิดระบบ Provisioning สิ้นสุดการทำงาน และเรียกคืนหน่วยความจำ (De-init) |

---

## 4. ตารางเปรียบเทียบเชิงลึก: SoftAP vs. BLE Transport Scheme

| คุณลักษณะ (Attributes) | SoftAP Scheme (`scheme_softap`) | BLE Scheme (`scheme_ble`) |
| :--- | :--- | :--- |
| **การรองรับชิปฮาร์ดแวร์** | ESP32, ESP32-S2 (ไม่มี BT), ESP8266 | ESP32, ESP32-C3, ESP32-S3, ESP32-C6 |
| **โปรโตคอลระดับ Network** | HTTP Server (IP: `192.168.4.1`, Port 80) | Bluetooth GATT Profile (Services & Characteristics) |
| **ประสบการณ์ผู้ใช้ (UX)** | **ยุ่งยากกว่า:** ผู้ใช้ต้องออกจากแอป ไปเมนู Wi-Fi ในมือถือ สลับไปต่อ SSID ของ ESP32 (เน็ตมือถือจะหลุดชั่วคราว) | **ดีกว่ามาก (Seamless):** มือถือเชื่อมต่อเบื้องหลังผ่านบลูทูธอัตโนมัติ Wi-Fi และเน็ตมือถือไม่หลุด |
| **การใช้หน่วยความจำ RAM** | ปานกลาง (รัน HTTP Server และ Wi-Fi AP Stack) | สูงขณะ Provisioning (รัน Controller และ Host Stack เช่น NimBLE/Bluedroid) |
| **การคืนหน่วยความจำ (RAM Freeing)** | สั่งปิด SoftAP (`esp_wifi_set_mode(WIFI_MODE_STA)`) | มีประสิทธิภาพสูงมาก สามารถปล่อย RAM ของ BT คืนระบบได้ทั้งหมดด้วย `esp_bt_mem_release()` (~40-80 KB) |
| **ความเร็วในกระบวนการ** | ช้ากว่า (เสียเวลาตอนมือถือสลับ Access Point และรอ DHCP) | รวดเร็วกว่ามาก (เชื่อมต่อ BLE GATT ทันที) |
| **โครงสร้างการระบุ Endpoint** | แมปเป็น **HTTP URI Path** เช่น: <br/>- `POST /prov-session`<br/>- `POST /prov-config` | แมปเป็น **GATT Characteristic UUID**: <br/>- Base Service: `021a9004-...`<br/>- ดูชื่อผ่าน Descriptor `0x2901` (User Description) |
| **การตรวจสอบเชิง Forensic** | ดักจับแพ็กเก็ต HTTP POST ผ่าน Wi-Fi Sniffer (เช่น Wireshark) | ใช้แอป **nRF Connect for Mobile** หรือ BLE Packet Sniffer |

---

## 5. ระดับความปลอดภัยและรหัสวิทยา: Security 0 vs. Security 1 vs. Security 2

| คุณสมบัติ (Features) | Security 0 (Sec0) | Security 1 (Sec1) | Security 2 (Sec2) |
| :--- | :--- | :--- | :--- |
| **ระดับความปลอดภัย** | ❌ **ไม่มีความปลอดภัย (Plaintext)** | 🛡️ **ปานกลาง-สูง (Commercial Standard)** | 🔒 **สูงสุด (Industrial/Financial Grade)** |
| **การแลกเปลี่ยนคีย์ (Key Exchange)** | ไม่มี (ส่ง Plaintext) | **Curve25519 ECDH (X25519)** | **SRP6a (Secure Remote Password - RFC 5054)** |
| **อัลกอริทึมการเข้ารหัส** | ไม่มีการเข้ารหัส | **AES-CTR-128** (Stream Cipher) | **AES-GCM-128** (Authenticated Encryption) |
| **กลไกยืนยันตัวตน (Authentication)**| ไม่มี | **Proof-of-Possession (PoP)** | **Username/Password ผ่าน SRP Verifier & Salt** |
| **การป้องกันดักฟัง (Eavesdropping)** | ❌ ดักจับเห็น SSID/Password ทันที | ✅ ป้องกันได้ ข้อมูลถูกเข้ารหัสทั้งหมด | ✅ ป้องกันได้ ข้อมูลถูกเข้ารหัสทั้งหมด |
| **การป้องกัน MITM Attack** | ❌ ป้องกันไม่ได้ | ✅ ป้องกันได้ด้วย PoP | ✅ ป้องกันได้สมบูรณ์ (Zero-Knowledge Proof) |
| **การเก็บความลับใน Flash Firmware** | ไม่มีการเก็บ | ฝังค่า PoP แบบ Hardcoded String | ฝังเฉพาะ **Salt (16B) และ SRP Verifier (384B)** โดย **ไม่มีรหัสผ่านจริงอยู่ในตัวชิป** |
| **กรณีการใช้งานที่เหมาะสม** | ใช้เพื่อศึกษาโครงสร้าง Packet ในแล็บเท่านั้น **(ห้ามใช้จริง!)** | ผลิตภัณฑ์ Smart Home เชิงพาณิชย์ทั่วไป | อุปกรณ์เกรดอุตสาหกรรม, ระบบล็อกดิจิทัล, การเงิน |

### 5.1 Proof-of-Possession (PoP) คืออะไร?
* **PoP** คือ รหัสลับร่วมเฉพาะเครื่อง (Shared Secret) ซึ่งมักถูกพิมพ์ไว้บนตัวเครื่อง, สติ๊กเกอร์, หรือฝังอยู่ใน QR Code
* **วัตถุประสงค์:** เพื่อยืนยันว่าผู้สั่ง Provisioning เป็น **"ผู้ที่ถืออุปกรณ์อยู่จริงทางกายภาพ"** ป้องกันไม่ให้เพื่อนบ้านหรือบุคคลภายนอกในรัศมีบลูทูธแอบสั่ง Provisioning บิดเบือนการตั้งค่าของอุปกรณ์ (ป้องกัน Rogue Provisioning / MITM Attack)

### 5.2 ทำไม Security 2 (SRP6a) จึงปลอดภัยกว่า Security 1 เมื่อถูก Dump Flash?
* ใน **Sec1:** ค่า PoP ถูกฝังเป็นข้อความ Plaintext ใน Firmware หากชิปไม่มีการเข้ารหัส Flash และผู้โจมตีทำ Flash Dump จะสามารถแกะรหัส PoP ออกมาได้
* ใน **Sec2:** ใช้หลักการ **Zero-Knowledge Proof (ZKP)** โดยบอร์ด ESP32 จะเก็บเฉพาะค่า **Salt** และ **SRP Verifier** ทางคณิตศาสตร์เท่านั้น **ตัวรหัสผ่านจริงจะไม่ถูกบันทึกไว้ในอุปกรณ์เลย** ดังนั้น ต่อให้ขโมยชิปไป Dump Flash ออกมา ก็ไม่สามารถรู้รหัสผ่านดั้งเดิมได้ และยังป้องกัน Offline Dictionary Attack ได้อีกด้วย

### 5.3 โครงสร้างข้อมูล QR Code Payload (JSON Format)
```json
{
  "ver": "v1",
  "name": "PROV_0ED054",
  "pop": "abcd1234",
  "transport": "ble"
}
```
* `ver`: เวอร์ชันของ Payload รูปแบบ
* `name`: ชี้ระบุชื่อ Wi-Fi SSID (โหมด SoftAP) หรือ Advertising Name (โหมด BLE)
* `pop`: รหัส Proof-of-Possession ที่ใช้เจรจา Security Handshake
* `transport`: ช่องทางสื่อสารที่อุปกรณ์กำลังเปิดรอรับ (`"softap"` หรือ `"ble"`)

---

## 6. สถาปัตยกรรมหน่วยความจำ Flash และการตรวจพิสูจน์พยานหลักฐาน (NVS Forensics)

### 6.1 ผังโครงสร้าง SPI Flash Memory (4 MB / 0x400000)
โดยทั่วไป Partition Table ของ ESP-IDF กำหนดไว้ดังนี้:

| Offset Address | Size | Partition Name | Usage / Description |
| :--- | :--- | :--- | :--- |
| `0x00000 - 0x00FFF` | 4 KB | Bootloader Padding | เก็บ Header / Padding ของ Bootloader |
| `0x01000 - 0x07FFF` | 28 KB | 2nd Stage Bootloader | เฟิร์มแวร์ Bootloader สำหรับบูตระบบ |
| `0x08000 - 0x08FFF` | 4 KB | Partition Table | ตารางระบุตำแหน่งของแต่ละพาร์ติชันในชิป |
| **`0x09000 - 0x0EFFF`** | **24 KB (0x6000)** | **`nvs`** | **Non-Volatile Storage (เก็บ Wi-Fi Credentials ที่นี่!)** |
| `0x0F000 - 0x0FFFF` | 4 KB | `phy_init` | ข้อมูลปรับเทียบคลื่นวิทยุ RF Calibration |
| `0x10000 - 0x1FFFFF` | 1 MB+ | `factory` | Main Application Code (`app_main`) |

### 6.2 โครงสร้างภายในของ NVS Partition
* พื้นที่ขนาด 24 KB (0x6000) ถูกแบ่งออกเป็น **6 Pages (หน้าละ 4,096 ไบต์)**
* NVS เป็นระบบจัดเก็บข้อมูลแบบ **Key-Value Store** ที่มีคุณสมบัติ:
  * **Power-cut Resilient:** ทนทานต่อกรณีไฟดับกะทันหันขณะเขียนข้อมูล ข้อมูลไม่พัง
  * **Wear-Leveling:** กระจายการเขียนลบลงบล็อก Flash อย่างสม่ำเสมอ ยืดอายุการใช้งาน Flash Memory
* **โครงสร้างภายใน 1 Page (4 KB):**
  1. **Page Header (32 ไบต์):** เก็บ State (Empty, Active, Full), Sequence Number, CRC32
  2. **Entry State Bitmap (32 ไบต์):** ตารางบิตแมป 2 บิตต่อ 1 Entry (Empty, Written, Erased)
  3. **Entry Slots (126 สล็อต x 32 ไบต์ = 4,032 ไบต์):** บันทึกค่าคู่ Key-Value
* **ข้อมูล Wi-Fi ที่ถูกบันทึกใน NVS:**
  * Namespace: `nvs.net80211`
  * Keys สำคัญ: `sta.ssid` (ชื่อ Wi-Fi), `sta.pswd` (รหัสผ่าน Wi-Fi), `sta.authmode`

### 6.3 เครื่องมือและคำสั่งการ Dump ข้อมูล Flash (Forensic Analysis)
* ดึงข้อมูลเฉพาะพาร์ติชัน NVS (Offset `0x9000`, Length `0x6000`) ออกมาเป็นไบนารีไฟล์:
  ```powershell
  esptool.py -p COMxx -b 921600 read_flash 0x9000 0x6000 nvs_dump.bin
  ```
* เมื่อนำไฟล์ `nvs_dump.bin` มาเปิดด้วยโปรแกรม Hex Editor หรือใช้คำสั่ง `strings nvs_dump.bin` จะพบข้อความ SSID และรหัสผ่าน Wi-Fi ชัดเจน
* **ช่องโหว่ความปลอดภัยทางกายภาพ (Physical Threat):** หากไม่ได้เปิดใช้งาน **Flash Encryption (AES-XTS)** หรือ **NVS Encryption** ใครก็ตามที่มีอุปกรณ์ Serial เชื่อมต่อกับชิป สามารถดูดข้อมูลรหัสผ่าน Wi-Fi ออกไปได้ทั้งหมด

---

## 7. กลไกการ Reset ค่าคอนฟิกทั้ง 3 รูปแบบ และข้อควรระวัง Strapping Pin

เมื่ออุปกรณ์ผ่านการตั้งค่าแล้ว `wifi_prov_mgr_is_provisioned()` จะคืนค่า `true` เสมอ ทำให้ข้ามโหมด Provisioning การสั่งล้างค่าคอนฟิกทำได้ 3 รูปแบบหลัก:

| รูปแบบการ Reset | วิธีดำเนินการ | ผลกระทบต่อ Flash Memory | สถานะการกู้คืนข้อมูล | เหมาะสำหรับ |
| :--- | :--- | :--- | :--- | :--- |
| **1. Developer CLI Erase** | `idf.py erase-flash` | ลบข้อมูลทุกเซกเตอร์ในชิป Flash ให้กลายเป็น `0xFF` ทั้งหมด (Bootloader, Partition Table, NVS, และ App หายเกลี้ยง) | ❌ กู้คืนไม่ได้ ต้อง Flash โค้ดใหม่ทั้งหมด | ผู้พัฒนาที่ต้องการล้างบอร์ดเริ่มต้นใหม่หมดจด |
| **2. Build-time Flag (Menuconfig)** | กำหนดค่า `CONFIG_EXAMPLE_RESET_PROVISIONED=y` หรือเรียก `wifi_prov_mgr_reset_provisioning()` ในโค้ด | สั่งลบเฉพาะ Keys ข้อมูล Wi-Fi ใน Namespace `nvs.net80211` ใน NVS เท่านั้น ส่วนโค้ด App ยังอยู่คงเดิม | ⚠️ อาจยังมีข้อมูลเก่าค้างใน Erased Entry ถ้ายังไม่เกิด Flash Garbage Collection | การทดสอบโค้ดซ้ำๆ ในขั้นตอนการพัฒนา |
| **3. Hardware Pushbutton (Consumer Factory Reset)** | ต่อสวิตช์ปุ่มกดภายนอกบน **GPIO 18** กดค้าง 3 วินาที สั่งรัน `nvs_flash_erase()` | ล้างเฉพาะข้อมูลใน Partition `nvs` ทั้งหมด โค้ดโปรแกรมหลักยังทำงานต่อได้ตามปกติ | ❌ ข้อมูลใน NVS ถูกล้างหมด และบอร์ดจะกลับสู่โหมด Provisioning ใหม่ | **ผลิตภัณฑ์จริงสำหรับผู้บริโภค (Consumer IoT)** |

### 7.1 ประเด็นสำคัญที่มักออกสอบ: ทำไมจึงห้ามใช้ปุ่ม BOOT (GPIO 0) ทำ Factory Reset?
* ขา **GPIO 0** บน ESP32 ทำหน้าที่เป็น **Strapping Pin** ในระดับฮาร์ดแวร์สำหรับกำหนดโหมดการบูตชิป
* หากขา GPIO 0 มีสถานะเป็น **`LOW (0)`** ในจังหวะที่บอร์ดเริ่มทำงาน (Reset หรือ Power-on) ชิป ESP32 จะถูกฮาร์ดแวร์ดักให้เข้าสู่ **ROM Download Bootloader Mode** ทันที เพื่อรอรับการแฟลชโปรแกรมใหม่ผ่าน Serial
* ผลคือ **บอร์ดจะไม่ยอมรันโค้ดโปรแกรมในแฟลช (`app_main`)** ทำให้ฟังก์ชันตรวจสอบปุ่มกดที่เขียนไว้ไม่มีโอกาสได้ทำงานเลย
* **ทางแก้ในเชิงวิศวกรรม:** จึงต้องใช้ขา GPIO ทั่วไป เช่น **GPIO 18** พร้อมเปิดใช้งาน **Internal Pull-up Resistor** แทน

---

## 8. การประยุกต์ใช้งาน Custom Data Endpoint และการจัดการหน่วยความจำ Heap

### 8.1 วัตถุประสงค์ของ Custom Endpoint
ในทางธุรกิจ นอกจากการส่ง Wi-Fi SSID และ Password แล้ว ระบบ IoT มักต้องส่งข้อมูลตั้งค่าอื่นๆ ตั้งแต่วันแรก เช่น:
* รหัสยืนยันการเป็นเจ้าของ (Device Activation Code / Claim Token)
* รหัสประจำตัวผู้ใช้งาน (User ID / Customer Tenant ID)
* ชื่อโฮสต์ของเซิร์ฟเวอร์คลาวด์ (Custom MQTT Broker URL / Port)
* ใบรับรองดิจิทัลหรือคีย์ความปลอดภัยประจำอุปกรณ์ (Device Certificate)

### 8.2 ลำดับคำสั่งการสร้าง Custom Endpoint ใน C
```c
// ขั้นที่ 1: ต้องสร้าง Endpoint Name ก่อนเริ่ม Service เสมอ
wifi_prov_mgr_endpoint_create("custom-data");

// ขั้นที่ 2: เริ่มต้นระบบ Provisioning
ESP_ERROR_CHECK(wifi_prov_mgr_start_provisioning(security, sec_params, service_name, service_key));

// ขั้นที่ 3: ลงทะเบียน Callback Handler หลัง Service เริ่มแล้ว
wifi_prov_mgr_endpoint_register("custom-data", custom_prov_data_handler, NULL);
```

### 8.3 ประเด็นสำคัญเรื่องหน่วยความจำใน `custom_prov_data_handler()`
พิจารณาโครงสร้าง Callback:
```c
esp_err_t custom_prov_data_handler(uint32_t session_id, const uint8_t *inbuf, ssize_t inlen,
                                   uint8_t **outbuf, ssize_t *outlen, void *priv_data)
{
    if (inbuf) {
        ESP_LOGI(TAG, "Received custom data: %.*s", (int)inlen, (char *)inbuf);
    }
    
    char response[] = "ACK_FROM_ESP32";
    // ข้อบังคับสำคัญ: ต้องจัดสรรหน่วยความจำบน Heap ด้วย strdup() หรือ malloc()
    *outbuf = (uint8_t *)strdup(response);
    if (*outbuf == NULL) {
        return ESP_ERR_NO_MEM;
    }
    *outlen = strlen(response) + 1;
    return ESP_OK;
}
```
> [!IMPORTANT]
> **ทำไม `*outbuf` จึงต้องจัดสรรบน Heap Memory (`malloc`/`strdup`) และห้ามใช้ Local Stack Array?**
> * เพราะเมื่อฟังก์ชัน Callback นี้ทำงานเสร็จและ Return กลับออกไป ตัวแปรแบบ Local Stack จะถูกทำลายทิ้งทันที
> * ตัวสถาปัตยกรรม **Protocomm Subsystem** ได้รับการออกแบบให้เป็นผู้รับผิดชอบการเรียกฟังก์ชัน `free(*outbuf)` เองโดยอัตโนมัติ หลังจากที่มันส่งข้อมูลกลับไปยังสมาร์ตโฟนผ่านบลูทูธ/Wi-Fi เสร็จเรียบร้อย
> * หากเราส่งพอยน์เตอร์ของ Local Array ไปให้ Protocomm จะเกิดข้อผิดพลาดรุนแรงประเภท **Memory Corruption (Dangling Pointer / Free Invalid Pointer Crash)** ทำให้ระบบล่มทันที

---

## 9. รวมแนวข้อสอบและคำถามท้ายการทดลอง (Q&A สำหรับเตรียมสอบ)

### ข้อ 1: อธิบายเหตุผลว่าทำไมกระบวนการ Wi-Fi Provisioning จึงจำเป็นต่ออุปกรณ์ IoT เชิงพาณิชย์?
**แนวคำตอบ:** อุปกรณ์ IoT ส่วนใหญ่เป็น Headless Devices ที่ไม่มีหน้าจอและแป้นพิมพ์ จึงไม่สามารถให้ผู้ใช้ป้อน SSID และรหัสผ่าน Wi-Fi ลงบนตัวเครื่องได้โดยตรง จึงต้องอาศัยกลไก Wi-Fi Provisioning ในการส่งมอบข้อมูลการตั้งค่าเครือข่ายจากสมาร์ตโฟนไปยังอุปกรณ์ผ่านสื่อสัญญาณระยะใกล้ชั่วคราวอย่างปลอดภัย ก่อนที่อุปกรณ์จะนำค่านั้นไปบันทึกลงหน่วยความจำถาวรและต่อออกสู่อินเทอร์เน็ต

### ข้อ 2: เปรียบเทียบข้อดีและข้อเสียระหว่าง SoftAP Scheme และ BLE Scheme?
**แนวคำตอบ:**
* **SoftAP Scheme:** ข้อดีคือรองรับไมโครคอนโทรลเลอร์รุ่นราคาประหยัดที่ไม่มีบลูทูธ (เช่น ESP8266, ESP32-S2) แต่ข้อเสียคือประสบการณ์ผู้ใช้งาน (UX) ด้อยกว่า ผู้ใช้ต้องสลับการเชื่อมต่อ Wi-Fi บนมือถือมาต่อที่บอร์ด ทำให้เน็ตมือถือหลุดชั่วคราว และกระบวนการเชื่อมต่อช้ากว่า
* **BLE Scheme:** ข้อดีคือ UX ดีเยี่ยม ผู้ใช้ไม่ต้องสลับการเชื่อมต่อ Wi-Fi บนมือถือ เชื่อมต่อรวดเร็ว และสามารถเรียกคืนหน่วยความจำ Bluetooth RAM ทั้งหมด (`esp_bt_mem_release()`) ได้หลังจาก Provisioning เสร็จสิ้น แต่ข้อเสียคือใช้ RAM ชั่วคราวสูงกว่า และชิปต้องมีภาครับส่ง Bluetooth ในตัว

### ข้อ 3: Proof-of-Possession (PoP) คืออะไร และทำหน้าที่ป้องกันภัยคุกคามประเภทใด?
**แนวคำตอบ:** PoP คือรหัสลับเฉพาะเครื่อง (Shared Secret) ที่ระบุอยู่บนตัวอุปกรณ์หรือใน QR Code ทำหน้าที่เป็นเครื่องยืนยันว่าผู้ที่กำลังสั่ง Provisioning อยู่หน้าอุปกรณ์จริงทางกายภาพ ช่วยป้องกันการโจมตีแบบ **Rogue Provisioning / Unauthorized Provisioning** (บุคคลแปลกหน้าหรือเพื่อนบ้านที่อยู่ในระยะสัญญาณวิทยุแอบสั่งตั้งค่าอุปกรณ์) รวมถึงช่วยป้องกันการโจมตีแบบ **Man-in-the-Middle (MITM)** ร่วมกับการทำ Key Exchange

### ข้อ 4: ในระดับ Security 1 และ Security 2 ใช้อัลกอริทึมใดในการแลกเปลี่ยนกุญแจและเข้ารหัสข้อมูล?
**แนวคำตอบ:**
* **Security 1:** ใช้ **Curve25519 ECDH (X25519)** ร่วมกับ PoP ในการแลกเปลี่ยนคีย์ และใช้ **AES-CTR-128** ในการเข้ารหัสข้อมูล Payload
* **Security 2:** ใช้ **SRP6a (Secure Remote Password - RFC 5054)** ร่วมกับ Salt และ Verifier ในการแลกเปลี่ยนคีย์ และใช้ **AES-GCM-128** (Authenticated Encryption) ในการเข้ารหัสและตรวจสอบความถูกต้องของข้อมูล

### ข้อ 5: เหตุใด Security 2 จึงปลอดภัยจากการตรวจสอบเชิงนิติวิทยาศาสตร์ (Flash Dump Attack) มากกว่า Security 1?
**แนวคำตอบ:** ใน Security 1 รหัส PoP มักถูกฝังไว้เป็นข้อความธรรมดา (Plaintext) ในเฟิร์มแวร์ หากผู้โจมตีทำ Flash Dump จะสามารถค้นหารหัสผ่านได้โดยง่าย แต่ใน Security 2 มีการใช้ Zero-Knowledge Proof ข้อมูลที่ฝังอยู่ใน ESP32 จะมีเพียง **Cryptographic Salt และ SRP Verifier** เท่านั้น **ไม่มีรหัสผ่านจริงถูกเก็บไว้ในชิปเลย** ผู้โจมตีจึงไม่สามารถนำค่า Verifier ไปย้อนหารหัสผ่านเดิมได้โดยตรง

### ข้อ 6: เหตุใดจึงไม่ควรใช้ปุ่ม BOOT (GPIO 0) ในการทำปุ่ม Hardware Factory Reset?
**แนวคำตอบ:** ขา GPIO 0 เป็น **Strapping Pin** หากขานี้ถูกต่อลงกราวด์ (Logic 0) ในระหว่างที่เปิดเครื่องหรือรีเซ็ต ชิป ESP32 จะเข้าสู่ **ROM Download Bootloader Mode** ทันที ทำให้ตัวประมวลผลหยุดรอการแฟลชโปรแกรมใหม่และไม่รันโค้ด `app_main()` ผู้พัฒนาจึงควรเลือกใช้ขา GPIO ทั่วไป (เช่น GPIO 18) ร่วมกับตัวต้านทาน Pull-up ภายในแทน

### ข้อ 7: ในการเขียนฟังก์ชัน Custom Data Handler เหตุใดค่า `*outbuf` จึงต้องจัดสรรผ่าน `malloc()` หรือ `strdup()`?
**แนวคำตอบ:** เนื่องจากตัวแปรแบบ Local Stack จะถูกทำลายทันทีเมื่อฟังก์ชันทำงานเสร็จสิ้น สถาปัตยกรรม Protocomm จึงถูกออกแบบให้ตัว Handler ต้องจัดสรรหน่วยความจำแบบไดนามิกบน Heap เพื่อให้ Protocomm สามารถส่งข้อมูลไปยัง Client ให้สมบูรณ์ก่อน จากนั้น Protocomm จะเป็นผู้สั่ง `free(*outbuf)` คืนหน่วยความจำให้ระบบเองโดยอัตโนมัติ หากใช้ Local Variable จะส่งผลให้โปรแกรม Crash จากการพยายาม Free หน่วยความจำที่ไม่ถูกต้อง

### ข้อ 8: การทำงานของคำสั่ง `esp_bt_mem_release(ESP_BT_MODE_BTDM)` มีประโยชน์อย่างไรต่อระบบ IoT?
**แนวคำตอบ:** ช่วยคืนพื้นที่หน่วยความจำ RAM ของ Bluetooth Stack (ทั้ง Controller และ Host) ที่ใช้ไปประมาณ 40 KB – 80 KB กลับคืนสู่ระบบ DRAM เพื่อให้หน่วยความจำส่วนนี้พร้อมสำหรับการใช้งานในโหมดปกติ เช่น การเชื่อมต่อ Wi-Fi Station, การสร้าง TLS Session, หรือการประมวลผลข้อมูล Application

---
*เอกสารสรุปนี้จัดทำขึ้นโดยครอบคลุมเนื้อหาทั้งภาคทฤษฎีและภาคปฏิบัติการ เพื่อความพร้อมสูงสุดในการสอบวิชา IoT System Development*
