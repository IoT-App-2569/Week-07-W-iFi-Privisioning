# ใบงานที่ 7.1  การศึกษากลไก Reset Provisioning 3 รูปแบบ และ NVS Memory Forensics

```
load:0x40078000,len:15916
load:0x40080400,len:3860
--- 0x40080400: _invalid_pc_placeholder at C:/Users/09677/esp/v5.5.1/esp-idf/components/xtensa/xtensa_vectors.S:2235
entry 0x40080638
--- 0x40080638: call_start_cpu0 at C:/Users/09677/esp/v5.5.1/esp-idf/components/bootloader/subproject/main/bootloader_start.c:25
I (29) boot: ESP-IDF v5.5.1-dirty 2nd stage bootloader
I (29) boot: compile time Aug 24 2026 09:11:49
I (29) boot: Multicore bootloader
I (31) boot: chip revision: v3.1
I (34) boot.esp32: SPI Speed      : 40MHz
I (38) boot.esp32: SPI Mode       : DIO
I (41) boot.esp32: SPI Flash Size : 2MB
I (45) boot: Enabling RNG early entropy source...
I (49) boot: Partition Table:
I (52) boot: ## Label            Usage          Type ST Offset   Length
I (58) boot:  0 nvs              WiFi data        01 02 00009000 00006000
I (65) boot:  1 phy_init         RF data          01 01 0000f000 00001000
I (71) boot:  2 factory          factory app      00 00 00010000 00100000
I (78) boot: End of partition table
I (81) esp_image: segment 0: paddr=00010020 vaddr=3f400020 size=1d168h (119144) map
I (129) esp_image: segment 1: paddr=0002d190 vaddr=3ffb0000 size=02e88h ( 11912) load
I (134) esp_image: segment 2: paddr=00030020 vaddr=400d0020 size=921f4h (598516) map
I (339) esp_image: segment 3: paddr=000c221c vaddr=3ffb2e88 size=010a4h (  4260) load
I (340) esp_image: segment 4: paddr=000c32c8 vaddr=40080000 size=17f8ch ( 98188) load
I (381) esp_image: segment 5: paddr=000db25c vaddr=50000000 size=00020h (    32) load
I (393) boot: Loaded app from partition at offset 0x10000
I (393) boot: Disabling RNG early entropy source...
I (403) cpu_start: Multicore app
I (412) cpu_start: Pro cpu start user code
I (412) cpu_start: cpu freq: 160000000 Hz
I (412) app_init: Application information:
I (412) app_init: Project name:     lab7_1_reset_nvs_forensics
I (417) app_init: App version:      18ddd5a-dirty
I (422) app_init: Compile time:     Aug 24 2026 09:10:17
I (427) app_init: ELF file SHA256:  71fa2eca6...
I (431) app_init: ESP-IDF:          v5.5.1-dirty
I (435) efuse_init: Min chip rev:     v0.0
I (439) efuse_init: Max chip rev:     v3.99 
I (443) efuse_init: Chip rev:         v3.1
I (447) heap_init: Initializing. RAM available for dynamic allocation:
I (454) heap_init: At 3FFAE6E0 len 00001920 (6 KiB): DRAM
I (458) heap_init: At 3FFB8028 len 00027FD8 (159 KiB): DRAM
I (464) heap_init: At 3FFE0440 len 00003AE0 (14 KiB): D/IRAM
I (469) heap_init: At 3FFE4350 len 0001BCB0 (111 KiB): D/IRAM
I (475) heap_init: At 40097F8C len 00008074 (32 KiB): IRAM
W (481) spi_flash: Detected boya flash chip but using generic driver. For optimal functionality, enable `SPI_FLASH_SUPPORT_BOYA_CHIP` in menuconfig
I (493) spi_flash: detected chip: generic
I (496) spi_flash: flash io: dio
W (499) spi_flash: Detected size(4096k) larger than the size in the binary image header(2048k). Using the size in the binary image header.
I (513) main_task: Started on CPU0
I (523) main_task: Calling app_main()
I (523) LAB7_1_RESET: Hold GPIO 18 button for 3 seconds to trigger Factory Reset...
I (543) wifi:wifi driver task: 3ffc0794, prio:23, stack:6656, core=0
I (543) wifi:wifi firmware version: 14da9b7
I (543) wifi:wifi certification version: v7.0
I (543) wifi:config NVS flash: enabled
I (543) wifi:config nano formatting: disabled
I (553) wifi:Init data frame dynamic rx buffer num: 32
I (553) wifi:Init static rx mgmt buffer num: 5
I (563) wifi:Init management short buffer num: 32
I (563) wifi:Init dynamic tx buffer num: 32
I (563) wifi:Init static rx buffer size: 1600
I (573) wifi:Init static rx buffer num: 10
I (573) wifi:Init dynamic rx buffer num: 32
I (583) wifi_init: rx ba win: 6
I (583) wifi_init: accept mbox: 6
I (583) wifi_init: tcpip mbox: 32
I (583) wifi_init: udp mbox: 6
I (593) wifi_init: tcp mbox: 6
I (593) wifi_init: tcp tx win: 5760
I (593) wifi_init: tcp rx win: 5760
I (603) wifi_init: tcp mss: 1440
I (603) wifi_init: WiFi IRAM OP enabled
I (603) wifi_init: WiFi RX IRAM OP enabled
W (613) LAB7_1_RESET: --------------------------------------------------
W (613) LAB7_1_RESET: [STATUS]: Device is NOT provisioned (NVS is empty)
W (623) LAB7_1_RESET: Ready for Provisioning Lab 7-2 (SoftAP) or 7-3 (BLE)!
W (633) LAB7_1_RESET: --------------------------------------------------

```

## 5. กิจกรรมถอดรหัสซอร์สโค้ดและเขียนผังงาน (Code Deconstruction & Flowchart Assignment)

ให้นักศึกษาศึกษาโค้ดใน `main/main.c` และ `main/led_indicator.c` แล้วเขียน **ผังงาน (Flowchart / State Diagram)** เพื่ออธิบายการตัดสินใจและการทำงานของระบบ:

### ภารกิจที่ 1  ผังงานการตัดสินใจช่วง Bootstrapping & Reset Decision
ให้นักศึกษาวาด Flowchart แสดงลำดับตรรกะการตรวจสอบเงื่อนไขตั้งแต่เริ่มต้นรันฟังก์ชัน `app_main()` โดยต้องครอบคลุม:
1. การตรวจสอบสถานะปุ่ม **GPIO 18** (ตรวจจับการกดค้าง 3 วินาที)
2. การทำงานของ `nvs_flash_init()` และกรณีที่ต้อง `nvs_flash_erase()`
3. การตรวจสอบ Macro `#ifdef CONFIG_EXAMPLE_RESET_PROVISIONED`
4. การเรียกฟังก์ชัน `wifi_prov_mgr_is_provisioned(&provisioned)`
5. จุดแยกสายการทำงานเข้าสู่โหมด **Provisioning Mode** หรือ **Station Mode**

```text
[graph TD
    A[เริ่มต้น app_main] --> B[ตั้งค่า GPIO 18 อ่านค่าปุ่ม]
    B --> C{กด GPIO 18 ค้าง >= 3s?}
    C -- ใช่ --> D[สั่ง ลบ/Reset ข้อมูล NVS Flash]
    C -- ไม่ใช่ --> E[เรียก nvs_flash_init]
    D --> E
    E --> F{#ifdef CONFIG_EXAMPLE_RESET_PROVISIONED}
    F -- กำหนดไว้ --> G[สั่ง ลบ/Reset ข้อมูล NVS ทันที]
    F -- ไม่ได้กำหนด --> H[เรียก wifi_prov_mgr_is_provisioned]
    G --> H
    H --> I{provisioned == true?}
    I -- ใช่ --> J[เข้าสู่ Station Mode]
    I -- ไม่ใช่ --> K[เข้าสู่ Provisioning Mode]

```

### ภารกิจที่ 2 ผังสถานะการเปลี่ยนจังหวะไฟ LED 1 (Wi-Fi STA Indicator)
ให้นักศึกษาวาด State Diagram แสดงการเปลี่ยนสถานะของ **LED 1 (GPIO 2)**:
- เงื่อนไขใดทำให้ LED 1 เข้าสู่สถานะ `LED_STA_MODE_DISCONNECTED` (กระพริบ 200ms Mark / 200ms Space)
- เงื่อนไขหรือ Event ใดทำให้เปลี่ยนเป็น `LED_STA_MODE_CONNECTED` (Heartbeat 200ms ทุก 1s)
```
stateDiagram-v2
    [*] --> LED_STA_MODE_DISCONNECTED : เริ่มต้นระบบ / ยังไม่เชื่อมต่อ Wi-Fi
    LED_STA_MODE_DISCONNECTED --> LED_STA_MODE_CONNECTED : เกิด Event IP_EVENT_STA_GOT_IP (เชื่อมต่อสำเร็จ)
    LED_STA_MODE_CONNECTED --> LED_STA_MODE_DISCONNECTED : เกิด Event WIFI_EVENT_STA_DISCONNECTED (หลุดการเชื่อมต่อ)
```


---

## 6. ตารางบันทึกผลการทดลอง (Experiment Results)

| รูปแบบการ Reset                  | คำสั่ง / พฤติกรรมที่ทำ               | พฤติกรรมของ LED แต่ละดวงหลังเปิดเครื่อง | สถานะใน Serial Monitor                                           |
| :------------------------------- | :----------------------------------- | :-------------------------------------- | :--------------------------------------------------------------- |
| **1. CLI Erase**                 | `idf.py erase-flash`                 | ดับทุกดวง / ไม่ทำงานค้าง                | ที่ Bootloader (หา Partition/App ไม่เจอ                          |
| **2. Menuconfig Flag**           | `CONFIG_EXAMPLE_RESET_PROVISIONED=y` | กระพริบถี่ (เข้า Provisioning Mode)     | ลบข้อมูล NVS อัตโนมัติ แล้วแจ้งเตือน `Device is NOT provisioned` |
| **3. Hardware Button (GPIO 18)** | กดปุ่ม GPIO 18 ค้าง 3 วินาที         | เปลี่ยนจากไฟค้างเป็นกระพริบถี่          | แสดง Log ลบ NVS สำเร็จ แล้วรีบูตเข้าสู่ Provisioning Mode        |

---

## 7. คำถามท้ายการทดลอง (Post-Lab Questions)
1. เพราะเหตุใดการกดปุ่ม BOOT (GPIO 0) ค้างไว้ในจังหวะรีเซ็ตบอร์ด จึงทำให้โปรแกรมค้างอยู่ที่ ROM Bootloader และไม่ยอมทำงานต่อ?
		GPIO 0 ค้างทำให้โปรแกรมค้างที่ ROM Bootloader: เพราะ GPIO 0 เป็น Strapping Pin ที่ใช้เลือก Boot Mode เมื่อกดค้างตอน Reset แรงดันจะเป็น LOW ซึ่งบังคับให้ ESP32 เข้าสู่ **Flashing Mode** เพื่อรอรับโค้ดใหม่ผ่าน Serial แทนการรันโปรแกรมใน Flash
2. เพราะเหตุใดคำสั่ง `idf.py erase-flash` จึงทำให้ข้อมูลเฟิร์มแวร์ Application หายไปด้วย ในขณะที่ `nvs_flash_erase()` ไม่ทำให้เฟิร์มแวร์หาย?
		`idf.py erase-flash` vs `nvs_flash_erase()`: `erase-flash` สั่งลบข้อมูลทิ้ง **ทั้งชิป (Entire Chip)** รวม Partition Table และ App Firmware ส่วน `nvs_flash_erase()` สั่งลบเฉพาะ **Partition NVS** ที่เก็บค่าการตั้งค่าเท่านั้น เฟิร์มแวร์จึงไม่หาย
3. การออกแบบปุ่ม Factory Reset บนอุปกรณ์ IoT เชิงพาณิชย์ เหตุใดจึงต้องกำหนดให้ผู้ใช้กดปุ่มค้างไว้ 3-5 วินาที แทนที่จะสั่งลบข้อมูลทันทีที่แตะปุ่มเพียงเสี้ยววินาที?
		หน่วงเวลา 3-5 วินาทีในการ Factory Reset: เพื่อป้องกัน **Accidental Trigger** จากการมือไปโดนปุ่มโดยไม่ได้ตั้งใจ สัญญาณรบกวนทางไฟฟ้า (Debounce) หรือแรงสั่นสะเทือน
4. หากอุปกรณ์ IoT ถูกติดตั้งอยู่บนเสาสูงหรือฝังอยู่ในผนัง วิธีการ Reset ทางกายภาพรูปแบบใดเหมาะสมที่สุด?
		การ Reset อุปกรณ์ที่ติดตั้งบนเสาสูง/ฝังผนัง: ใช้ **Remote Software Reset** เช่น การสั่งล้างค่าผ่าน Cloud/MQTT, การส่งคำสั่งผ่านแอปพลิเคชัน (Web API/Mobile App), หรือการออกแบบ **Power-Cycle Pattern** (เช่น ถอด-เสียบปลั๊กติดต่อกัน 5 ครั้ง)
-----------------------------------

# ใบงานที่ 7.2 การคอนฟิก Wi-Fi ผ่าน SoftAP Scheme และการวิเคราะห์ Protocomm Endpoints

```

```
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

```text
[พื้นที่สำหรับแนบรูปภาพ Sequence Diagram ที่นักศึกษาเขียนขึ้นด้วย Draw.io / Mermaid / วาดมือ]
```

---

## 6. ตารางบันทึกผลการทดลอง (Experiment Results)

| รายการตรวจสอบ | ค่าที่บันทึกได้จากการทดลอง |
| :--- | :--- |
| **1. ชื่อ SoftAP SSID ของ ESP32** | `PROV_`.............................. |
| **2. รหัส PoP (Proof of Possession)** | ..................................................... |
| **3. ข้อความใน QR Code Payload (JSON)** | ..................................................... |
| **4. พฤติกรรมไฟ LED 3 (GPIO 5) ช่วงรอ vs ช่วงส่งข้อมูล** | ช่วงรอ: .......................................<br/>ช่วงส่ง: ....................................... |
| **5. IP Address ที่ ESP32 ได้รับจาก Router** | ..................................................... |
| **6. เวลาที่ใช้ตั้งแต่เริ่มจนจบกระบวนการ (วินาที)** | ..................................................... |

---

## 7. คำถามท้ายการทดลอง (Post-Lab Questions)
1. ในโหมด SoftAP Scheme สมาร์ตโฟนส่งข้อมูลหา ESP32 ผ่านโปรโตคอลและ IP Address ใด?
2. หากผู้ใช้ป้อนรหัสผ่าน Wi-Fi ผิดในแอปมือถือ จะเกิด Event ใดขึ้นบน ESP32 (`WIFI_PROV_CRED_FAIL` หรือไม่) และ ESP32 มีพฤติกรรมอย่างไร?
3. ทำไมผู้ผลิต IoT ส่วนใหญ่จึงมองว่ากระบวนการเชื่อมต่อแบบ SoftAP มีขั้นตอนที่ยุ่งยากสำหรับผู้ใช้ทั่วไปเมื่อเทียบกับ BLE?

---
# ใบงานที่ 7.3 การคอนฟิก Wi-Fi ผ่าน BLE Scheme และการสืบสวน GATT Services (BLE Forensics)
```

```
## 5. กิจกรรมถอดรหัสซอร์สโค้ดและเขียนผังงาน (Code Deconstruction & BLE GATT Architecture Assignment)

ให้นักศึกษาแกะรอยการทำงานของโมดูล BLE Provisioning ใน `main/main.c` แล้วเขียน **ผังโครงสร้างและลำดับเหตุการณ์**:

### ภารกิจที่ 1: ผังโครงสร้าง GATT Tree & Endpoint Mapping
ให้นักศึกษาวาดโครงสร้างต้นไม้ (Tree Diagram / Block Diagram) แสดงความสัมพันธ์ระหว่าง:
- **Primary Service (128-bit UUID: `021a9004-...`)**
  - **Characteristic UUIDs** แต่ละตัว
  - **Descriptor 0x2901 (User Description)** ที่ผูกเข้ากับ Protocomm Endpoints (`prov-session`, `prov-config`, `prov-scan`, `proto-ver`, `custom-data`)

### ภารกิจที่ 2: ผังลำดับการคืนหน่วยความจำ Bluetooth (BLE Lifecycle & Memory Reclaim Flow)
ให้นักศึกษาวาด Flowchart / Sequence แสดงว่า:
1. การเชื่อมต่อ BLE ถูกตรวจพบผ่าน Event `PROTOCOMM_TRANSPORT_BLE_CONNECTED` (LED 2 กระพริบเร็ว 100ms)
2. เมื่อเชื่อมต่อ Wi-Fi สำเร็จ (`WIFI_PROV_CRED_SUCCESS`) $\rightarrow$ เกิด Event `WIFI_PROV_END`
3. Provisioning Manager สั่งเรียก `esp_bt_mem_release()` เพื่อปล่อย DRAM คืนสู่ระบบอย่างไร

```text
[พื้นที่สำหรับแนบรูปภาพ Diagram ที่นักศึกษาเขียนขึ้นด้วย Draw.io / Mermaid / วาดมือ]
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

---

## 7. คำถามท้ายการทดลอง (Post-Lab Questions)
1. เหตุใด BLE Provisioning จึงไม่ส่งผลให้สัญญาณ Wi-Fi บนสมาร์ตโฟนของผู้ใช้หลุดระหว่างทำรายการ?
2. Descriptor `0x2901` มีความสำคัญอย่างไรต่อการที่แอปพลิเคชันมือถือจะทราบว่า Characteristic แต่ละตัวใช้ทำหน้าที่อะไร?
3. การที่ ESP-IDF มีฟังก์ชัน `esp_bt_mem_release()` มีประโยชน์อย่างไรต่อการทำงานของแอปพลิเคชัน IoT หลังเชื่อมต่อ Wi-Fi สำเร็จ?

---
# ใบงานที่ 7.4: การทดสอบ Security Schemes (PoP) และการรับส่ง Custom Data Endpoints
```

```
## 6. กิจกรรมถอดรหัสซอร์สโค้ดและเขียนผังงาน (Code Deconstruction & Security Flow Assignment)

ให้นักศึกษาแกะรอยการทำงานด้านความปลอดภัยและ Custom Handler ใน `main/main.c` แล้วเขียน **ผังการไหลของข้อมูล (Data Flow & Cryptographic Handshake Flow)**:

### ภารกิจที่ 1: ผังขั้นตอนการตรวจสอบ PoP (Security Handshake Decision Flow)
ให้นักศึกษาวาด Flowchart แสดงการแลกเปลี่ยนคีย์และตรวจสอบสิทธิ์:
1. การสร้าง Session Parameters ด้วยค่า PoP (`abcd1234`)
2. เมื่อ Client ส่ง Public Key + Verification Hash มาให้ ESP32
3. จุดแยกทางเลือก (Branching):
   - หาก PoP ไม่ตรง $\rightarrow$ Trigger Event `PROTOCOMM_SECURITY_SESSION_CREDENTIALS_MISMATCH` และปฏิเสธการเชื่อมต่อ
   - หาก PoP ถูกต้อง $\rightarrow$ Trigger Event `PROTOCOMM_SECURITY_SESSION_SETUP_OK` และสร้าง AES Session Key สำเร็จ

### ภารกิจที่ 2: ผังการรับส่งข้อมูลผ่าน Custom Endpoint (Custom Data Handler Flow)
ให้นักศึกษาวาด Sequence / Data Flow ของฟังก์ชัน `custom_prov_data_handler()`:
1. ข้อมูล `inbuf` ถูกส่งเข้ามาจากสมาร์ตโฟนผ่าน Protocomm
2. การพิมพ์ข้อความ Log ด้วย `ESP_LOGI()`
3. การจัดสรรหน่วยความจำแบบไดนามิกด้วย `strdup()` ให้กับ `*outbuf`
4. ทำไมตัวแปร `*outbuf` จึงต้องจัดสรรใน Heap Memory (ทำไมจึงใช้ตัวแปร Local Static Array ธรรมดาไม่ได้)?

```text
[พื้นที่สำหรับแนบรูปภาพ Diagram ที่นักศึกษาเขียนขึ้นด้วย Draw.io / Mermaid / วาดมือ]
```

---

## 7. ตารางบันทึกผลการทดลอง (Experiment Results)

| สถานการณ์ทดสอบ | ค่า PoP ที่ป้อน | ผลลัพธ์บนแอปมือถือ | ข้อความ Log ใน Serial Monitor |
| :--- | :--- | :--- | :--- |
| **1. ป้อน PoP ผิดพลาด** | `wrong1234` | | |
| **2. ป้อน PoP ถูกต้อง** | `abcd1234` | | |
| **3. ส่ง Custom Data** | `TEST_DATA_999` | | |

---

## 8. คำถามท้ายการทดลอง (Post-Lab Questions)
1. การใช้ **Proof-of-Possession (PoP)** ช่วยป้องกันการโจมตีประเภทใดได้บ้าง?
2. หากไม่มีการใช้ PoP (เช่น ใน Security 0) ผู้โจมตีที่อยู่ในรัศมีสัญญาณบลูทูธสามารถทำสิ่งใดกับอุปกรณ์ได้บ้าง?
3. ในการประยุกต์ใช้งานเชิงพาณิชย์จริง เราสามารถนำ **Custom Data Endpoint** ไปใช้ส่งข้อมูลประเภทใดได้อีกบ้าง (ยกตัวอย่าง 2 กรณี)?
4. ในฟังก์ชัน `custom_prov_data_handler()` เหตุใดหน่วยความจำที่จัดสรรให้ `*outbuf` จึงถูก Free โดย Protocomm Layer อัตโนมัติหลังจากส่งข้อมูลเสร็จ?

--- 