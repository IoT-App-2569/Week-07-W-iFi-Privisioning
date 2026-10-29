## 6. กิจกรรมถอดรหัสซอร์สโค้ดและเขียนผังงาน (Code Deconstruction & Security Flow Assignment)

ให้นักศึกษาแกะรอยการทำงานด้านความปลอดภัยและ Custom Handler ใน `main/main.c` แล้วเขียน **ผังการไหลของข้อมูล (Data Flow & Cryptographic Handshake Flow)**:

### ภารกิจที่ 1: ผังขั้นตอนการตรวจสอบ PoP (Security Handshake Decision Flow)
ให้นักศึกษาวาด Flowchart แสดงการแลกเปลี่ยนคีย์และตรวจสอบสิทธิ์:
1. การสร้าง Session Parameters ด้วยค่า PoP (`abcd1234`)
2. เมื่อ Client ส่ง Public Key + Verification Hash มาให้ ESP32
3. จุดแยกทางเลือก (Branching):
   - หาก PoP ไม่ตรง $\rightarrow$ Trigger Event `PROTOCOMM_SECURITY_SESSION_CREDENTIALS_MISMATCH` และปฏิเสธการเชื่อมต่อ
   - หาก PoP ถูกต้อง $\rightarrow$ Trigger Event `PROTOCOMM_SECURITY_SESSION_SETUP_OK` และสร้าง AES Session Key สำเร็จ
### ภารกิจที่ 1: ผังขั้นตอนการตรวจสอบ PoP (Security Handshake Decision Flow)

```mermaid
flowchart TD
    Start([เริ่มสร้าง Session]) --> InitSec[กำหนดค่า PoP: 'abcd1234' ใน Security 1 Structure]
    InitSec --> RecvReq[Client ส่ง Public Key และ Verification Hash มายัง ESP32]
    
    RecvReq --> CalcHash[ESP32 คำนวณ Verification Hash ร่วมกับ PoP]
    CalcHash --> CheckPoP{Verification Hash <br/>ตรงกับ PoP หรือไม่?}
    
    CheckPoP -- ไม่ตรง --> EventFail[Trigger Event: PROTOCOMM_SECURITY_SESSION_CREDENTIALS_MISMATCH]
    EventFail --> Reject[ปฏิเสธการเชื่อมต่อ / แจ้งเตือน PoP Incorrect]
    
    CheckPoP -- ตรง --> EventOK[Trigger Event: PROTOCOMM_SECURITY_SESSION_SETUP_OK]
    EventOK --> GenKey[สร้าง Shared AES-CTR Key สำหรับข้ามผ่าน Session]
    GenKey --> Success([สร้าง Secure Session สำเร็จ พร้อมรับส่งข้อมูลเข้ารหัส])

### ภารกิจที่ 2: ผังการรับส่งข้อมูลผ่าน Custom Endpoint (Custom Data Handler Flow)
ให้นักศึกษาวาด Sequence / Data Flow ของฟังก์ชัน `custom_prov_data_handler()`:
1. ข้อมูล `inbuf` ถูกส่งเข้ามาจากสมาร์ตโฟนผ่าน Protocomm
2. การพิมพ์ข้อความ Log ด้วย `ESP_LOGI()`
3. การจัดสรรหน่วยความจำแบบไดนามิกด้วย `strdup()` ให้กับ `*outbuf`
4. ทำไมตัวแปร `*outbuf` จึงต้องจัดสรรใน Heap Memory (ทำไมจึงใช้ตัวแปร Local Static Array ธรรมดาไม่ได้)?

```text
sequenceDiagram
    autonumber
    actor App as Smartphone App
    participant Proto as Protocomm Layer (ESP32)
    participant Handler as custom_prov_data_handler()
    participant Heap as Dynamic Heap Memory

    App->>Proto: 1. ส่งข้อมูลเข้ารหัสผ่าน Custom Endpoint ('custom-data')
    Proto->>Handler: 2. ถอดรหัสข้อมูลแล้วผ่านเข้า `inbuf` และ `inbuf_len`
    
    Note over Handler: 3. พิมพ์ข้อความ Log ด้วย ESP_LOGI()<br/>("Received Custom Data: %s", inbuf)
    
    Handler->>Heap: 4. เรียก strdup() จัดสรรพื้นที่และคัดลอกข้อความ response
    Heap-->>Handler: คืน Pointer ชี้ไปยังหน่วยความจำ (*outbuf)
    
    Handler-->>Proto: 5. ส่งคืน *outbuf, *outbuf_len และ return ESP_OK
    Proto->>App: 6. เข้ารหัส response จาก *outbuf แล้วส่งกลับไปยังแอป
    
    Note over Proto,Heap: 7. Protocomm Layer เรียก free(*outbuf)<br/>เพื่อคืนหน่วยความจำ DRAM อัตโนมัติ

    Note over Handler: **ทำไมต้องจัดสรร *outbuf ใน Heap Memory?**<br/>เนื่องจากตัวแปร Local (Stack) จะถูกทำลายทิ้งทันทีเมื่อฟังก์ชัน `custom_prov_data_handler()` ทำงานเสร็จ (Return)<br/>หากใช้ Local Static Array หรือ Stack Memory จะทำให้ Protocomm Layer อ่านข้อมูลขยะ (Dangling Pointer)<br/>ส่งผลให้โปรแกรม Crash (DRAM Access Violation) หรือข้อมูลที่ส่งกลับไปผิดพลาด
```

---

## 7. ตารางบันทึกผลการทดลอง (Experiment Results)

| สถานการณ์ทดสอบ | ค่า PoP ที่ป้อน | ผลลัพธ์บนแอปมือถือ | ข้อความ Log ใน Serial Monitor |
| :--- | :--- | :--- | :--- |
| **1. ป้อน PoP ผิดพลาด** | `wrong1234` | แสดงแจ้งเตือน **"Invalid PoP"** หรือ **"Authentication Failed"** | `E (xxxx) protocomm_sec1: Invalid PoP/Credentials`<br/>`W (xxxx) wifi_prov_mgr: Event: PROTOCOMM_SECURITY_SESSION_CREDENTIALS_MISMATCH` |
| **2. ป้อน PoP ถูกต้อง** | `abcd1234` | ผ่านขั้นตอนยืนยันตัวตน เข้าสู่หน้าสแกน/เลือก Wi-Fi AP | `I (xxxx) wifi_prov_mgr: Event: PROTOCOMM_SECURITY_SESSION_SETUP_OK`<br/>`I (xxxx) protocomm: Secure session established successfully` |
| **3. ส่ง Custom Data** | `TEST_DATA_999` | แอปได้รับการตอบกลับ (Acknowledge) หรือข้อความ Echo ตามที่ตั้งไว้ | `I (xxxx) app_main: Received Custom Data: TEST_DATA_999`<br/>`I (xxxx) app_main: Custom data processed successfully` |

---

## 8. คำถามท้ายการทดลอง (Post-Lab Questions)
1. การใช้ Proof-of-Possession (PoP) ช่วยป้องกันการโจมตีประเภทใดได้บ้าง?
คำตอบ: ช่วยป้องกันการโจมตีประเภท Man-in-the-Middle (MitM) Attack (การแอบดักจับหรือปลอมตัวเป็นอุปกรณ์เพื่อขโมย Wi-Fi Credentials) และ Unauthorized Access / Rogue Device Provisioning (การที่ผู้ไม่หวังดีในรัศมีสัญญาณแอบส่ง Wi-Fi Credentials ปลอมหรือสั่ง Reset อุปกรณ์โดยไม่มีสิทธิ์ทางกายภาพเหนืออุปกรณ์นั้น)

2. หากไม่มีการใช้ PoP (เช่น ใน Security 0) ผู้โจมตีที่อยู่ในรัศมีสัญญาณบลูทูธสามารถทำสิ่งใดกับอุปกรณ์ได้บ้าง?
คำตอบ: ผู้โจมตีสามารถเชื่อมต่อเข้าหา ESP32 ได้ทันทีโดยไม่ต้องยืนยันตัวตน สามารถดักจับข้อมูล Wi-Fi SSID/Password ที่ส่งผ่านอากาศ (หากไม่มีการเข้ารหัส) สั่งเปลี่ยน Wi-Fi Credentials ให้ ESP32 ไปเกาะ AP ปลอม (Rogue AP) ของผู้โจมตี หรือสั่ง DoS (Denial of Service) เพื่อก่อกวนระบบได้

3. ในการประยุกต์ใช้งานเชิงพาณิชย์จริง เราสามารถนำ Custom Data Endpoint ไปใช้ส่งข้อมูลประเภทใดได้อีกบ้าง (ยกตัวอย่าง 2 กรณี)?
คำตอบ:กรณีที่ 1: การลงทะเบียนกับ Cloud / MQTT Broker Key: ส่งข้อมูล Device Serial Number, Client Certificate หรือ Device Token เพื่อเชื่อมต่อกับ AWS IoT / Firebase / MQTT Broker

กรณีที่ 2: การตั้งค่าพารามิเตอร์เริ่มต้นของอุปกรณ์ (Initial Device Configuration): เช่น การตั้งชื่ออุปกรณ์ (Device Name/Room Location), Timezone/NTP Server หรือการกำหนดโหมดการทำงานของเซ็นเซอร์

3. ในฟังก์ชัน custom_prov_data_handler() เหตุใดหน่วยความจำที่จัดสรรให้ *outbuf จึงถูก Free โดย Protocomm Layer อัตโนมัติหลังจากส่งข้อมูลเสร็จ?
คำตอบ: เป็นการออกแบบสถาปัตยกรรมซอฟต์แวร์แบบ Ownership Transfer ของ ESP-IDF Protocomm Framework โดย custom_prov_data_handler() มีหน้าที่เพียงสร้างและส่งมอบ (Allocate) ข้อมูล response ให้กับ Protocomm จากนั้น Protocomm จะรับช่วงดูแล (Take Ownership) เพื่อเข้ารหัสและยิงข้อมูลออกทาง Transport (BLE/SoftAP) เมื่อการส่งข้อมูลเสร็จสมบูรณ์ Protocomm จะทำหน้าที่สั่ง free(*outbuf) เพื่อป้องกันปัญหา Memory Leak ในระบบ
