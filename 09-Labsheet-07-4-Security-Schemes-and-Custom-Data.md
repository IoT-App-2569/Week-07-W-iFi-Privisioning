# ใบงานที่ 7.4: การทดสอบ Security Schemes (PoP) และการรับส่ง Custom Data Endpoints

## 0. กล่าวนำ (Introduction)
ในใบงานนี้ นักศึกษาจะได้ทดสอบเจาะลึกด้านความปลอดภัยของกระบวนการ Provisioning โดยทำการทดลองจำลองสถานการณ์ที่มีผู้ไม่หวังดีพยายามเชื่อมต่อด้วย **รหัส Proof-of-Possession (PoP) ที่ไม่ถูกต้อง** เพื่อสังเกตกลไกการปฏิเสธการเชื่อมต่อของ Protocomm Security Layer

นอกจากนี้ นักศึกษาจะได้เรียนรู้การเพิ่ม **Custom Data Endpoint (`custom-data`)** เพื่อรับส่งข้อมูลเฉพาะของแอปพลิเคชัน (เช่น Device ID, Owner Email, MQTT Broker URL หรือ Activation Code) ระหว่างมือถือและ ESP32 ในระหว่างขั้นตอน Provisioning

---

## 1. วัตถุประสงค์ (Objectives)
1. เข้าใจบทบาทและการทำงานของ **Proof-of-Possession (PoP)** ในการป้องกันการโจมตีแบบสวมรอย (Rogue Provisioning)
2. ทดลองจำลองกรณีป้อน PoP ผิด และสังเกต Event `PROTOCOMM_SECURITY_SESSION_CREDENTIALS_MISMATCH`
3. เข้าใจการลงทะเบียน Custom Endpoint ด้วย `wifi_prov_mgr_endpoint_create()` และ `wifi_prov_mgr_endpoint_register()`
4. สังเกตและวิเคราะห์การรับส่งข้อมูลผ่าน Custom Handler (`custom_prov_data_handler`)

---

## 2. อุปกรณ์และซอฟต์แวร์ที่ใช้ในการทดลอง
1. บอร์ดไมโครคอนโทรลเลอร์ ESP32 พร้อมสาย USB
2. สมาร์ตโฟนที่ติดตั้งแอปพลิเคชัน **ESP BLE Provisioning** หรือ **ESP SoftAP Provisioning**
3. Serial Monitor Tool

---

## 3. สถาปัตยกรรม Custom Endpoint & PoP Security Handshake

```mermaid
sequenceDiagram
    autonumber
    participant App as 📱 Mobile App (Provisioner)
    participant Sec as 🔒 Protocomm Security 1
    participant Cust as 📦 Custom Data Handler
    participant ESP as ⚡ Application (main)

    rect rgb(240, 248, 255)
    note over App,Sec: 1. ทดสอบการใส่ PoP ถูกต้อง vs ผิด
    App->>Sec: ส่ง Key Exchange + PoP
    alt กรณี PoP ผิด (เช่น "wrongpass")
        Sec-->>App: ปฏิเสธ Handshake (Session Failed!)
        Sec->>ESP: Post Event: PROTOCOMM_SECURITY_SESSION_CREDENTIALS_MISMATCH
    else กรณี PoP ถูกต้อง ("abcd1234")
        Sec-->>App: แลกเปลี่ยนคีย์สำเร็จ (Session Established)
        Sec->>ESP: Post Event: PROTOCOMM_SECURITY_SESSION_SETUP_OK
    end
    end

    rect rgb(255, 250, 240)
    note over App,Cust: 2. การรับส่งข้อมูล Custom Endpoint
    App->>Cust: ส่งข้อความ Payload ไปยัง Endpoint: "custom-data"<br/>เช่น "USER_ID:65010099"
    Cust->>ESP: พิมพ์ Log: "Received data: USER_ID:65010099"
    Cust-->>App: ตอบกลับข้อความ: "SUCCESS"
    end
```

---

## 4. โค้ดส่วน Custom Data Handler ในตัวอย่าง `main.c`

พิจารณาการทำงานของฟังก์ชันจัดการข้อมูล Custom Endpoint:

```c
/* Handler สำหรับ Custom Endpoint ที่แอปพลิเคชันลงทะเบียนไว้ */
esp_err_t custom_prov_data_handler(uint32_t session_id, const uint8_t *inbuf, ssize_t inlen,
                                   uint8_t **outbuf, ssize_t *outlen, void *priv_data)
{
    if (inbuf) {
        ESP_LOGI(TAG, "Received custom data: %.*s", (int)inlen, (char *)inbuf);
    }
    
    // จัดเตรียมข้อความตอบกลับไปยังสมาร์ตโฟน
    char response[] = "ACK_FROM_ESP32";
    *outbuf = (uint8_t *)strdup(response);
    if (*outbuf == NULL) {
        ESP_LOGE(TAG, "System out of memory");
        return ESP_ERR_NO_MEM;
    }
    *outlen = strlen(response) + 1;

    return ESP_OK;
}
```

และขั้นตอนการลงทะเบียนใน `app_main()`:
```c
// 1. สร้าง Endpoint ก่อนเริ่ม Provisioning Service
wifi_prov_mgr_endpoint_create("custom-data");

// 2. เริ่มต้น Service
ESP_ERROR_CHECK(wifi_prov_mgr_start_provisioning(security, (const void *) sec_params, service_name, service_key));

// 3. ผูกฟังก์ชัน Callback เข้ากับ Endpoint หลังเริ่ม Service แล้ว
wifi_prov_mgr_endpoint_register("custom-data", custom_prov_data_handler, NULL);
```

---

## 5. ขั้นตอนการทดลอง (Step-by-Step Procedures)

### ตอนที่ 1: การเปิดโปรเจกต์และทดสอบ Security Handshake ด้วย PoP
1. เปิด Terminal ในโฟลเดอร์โปรเจกต์ `Week-07-W-iFi-Privisioning/Example_codes/Lab7-4-Custom-Data-and-Security`
2. สั่งล้าง Flash และรันโปรแกรม:
   ```powershell
   idf.py -p COM24 erase-flash flash monitor
   ```
3. เปิดแอป **ESP BLE Provisioning** สแกนหาบอร์ด ESP32
3. **การทดสอบที่ 1 (ป้อน PoP ผิด):**
   - เมื่อแอปถามรหัส PoP ให้พิมพ์รหัสผ่านมั่ว เช่น `wrong1234`
   - สังเกตปฏิกิริยาบนแอปมือถือและใน Serial Monitor:
     ```text
     E (15600) app: Received incorrect username and/or PoP for establishing secure session!
     ```
4. **การทดสอบที่ 2 (ป้อน PoP ถูกต้อง):**
   - สั่งรีเซ็ตบอร์ดใหม่ และเปิดแอปป้อน PoP เป็น `abcd1234` (ตรงกับค่าในโค้ด)
   - สังเกต Log:
     ```text
     I (18200) app: Secured session established!
     ```

---

### ตอนที่ 2: การรับส่งข้อมูลผ่าน Custom Endpoint
1. ในหน้าแอป **ESP BLE Provisioning** หลังผ่านขั้นตอนความปลอดภัยแล้ว ให้เข้าไปที่เมนู **Custom Data** หรือส่งข้อมูลผ่านแอปที่รองรับการเขียน Custom Endpoint
2. ป้อนข้อความ เช่น `STUDENT_ID:65010099` และกดส่ง
3. สังเกต Serial Monitor จะปรากฏข้อความที่ได้รับจากสมาร์ตโฟน:
   ```text
   I (22150) app: Received custom data: STUDENT_ID:65010099
   ```

---

---

## 6. กิจกรรมถอดรหัสซอร์สโค้ดและเขียนผังงาน (Code Deconstruction & Security Flow Assignment)

ให้นักศึกษาแกะรอยการทำงานด้านความปลอดภัยและ Custom Handler ใน `main/main.c` แล้วเขียน **ผังการไหลของข้อมูล (Data Flow & Cryptographic Handshake Flow)**:

### ภารกิจที่ 1: ผังขั้นตอนการตรวจสอบ PoP (Security Handshake Decision Flow)
ให้นักศึกษาวาด Flowchart แสดงการแลกเปลี่ยนคีย์และตรวจสอบสิทธิ์:
1. การสร้าง Session Parameters ด้วยค่า PoP (`abcd1234`)
2. เมื่อ Client ส่ง Public Key + Verification Hash มาให้ ESP32
3. จุดแยกทางเลือก (Branching):
   - หาก PoP ไม่ตรง $\rightarrow$ Trigger Event `PROTOCOMM_SECURITY_SESSION_CREDENTIALS_MISMATCH` และปฏิเสธการเชื่อมต่อ
   - หาก PoP ถูกต้อง $\rightarrow$ Trigger Event `PROTOCOMM_SECURITY_SESSION_SETUP_OK` และสร้าง AES Session Key สำเร็จ
   ![[Untitled Diagram.drawio (9).png]]

### ภารกิจที่ 2: ผังการรับส่งข้อมูลผ่าน Custom Endpoint (Custom Data Handler Flow)
ให้นักศึกษาวาด Sequence / Data Flow ของฟังก์ชัน `custom_prov_data_handler()`:
1. ข้อมูล `inbuf` ถูกส่งเข้ามาจากสมาร์ตโฟนผ่าน Protocomm
2. การพิมพ์ข้อความ Log ด้วย `ESP_LOGI()`
3. การจัดสรรหน่วยความจำแบบไดนามิกด้วย `strdup()` ให้กับ `*outbuf`
4. ทำไมตัวแปร `*outbuf` จึงต้องจัดสรรใน Heap Memory (ทำไมจึงใช้ตัวแปร Local Static Array ธรรมดาไม่ได้)?

![[Untitled Diagram.drawio (10).png]]

---

## 7. ตารางบันทึกผลการทดลอง (Experiment Results)

| สถานการณ์ทดสอบ          | ค่า PoP ที่ป้อน | ผลลัพธ์บนแอปมือถือ                                                                                                     | ข้อความ Log ใน Serial Monitor                                                                                                                              |
| :---------------------- | :-------------- | :--------------------------------------------------------------------------------------------------------------------- | :--------------------------------------------------------------------------------------------------------------------------------------------------------- |
| **1. ป้อน PoP ผิดพลาด** | `wrong1234`     | แสดงแจ้งเตือนล้มเหลว (เช่น _Session Setup Failed_ หรือ _Verification Failed_) และไม่สามารถเข้าสู่ขั้นตอนสแกน Wi-Fi ได้ | `E (...) app: Received incorrect username and/or PoP for establishing secure session!`_(Trigger Event: `PROTOCOMM_SECURITY_SESSION_CREDENTIALS_MISMATCH`)_ |
| **2. ป้อน PoP ถูกต้อง** | `abcd1234`      | ผ่านขั้นตอนการยืนยันตัวตนสำเร็จ และเข้าสู่หน้ารายการสแกนค้นหาเครือข่าย Wi-Fi                                           | `I (...) app: Secured session established!`_(Trigger Event: `PROTOCOMM_SECURITY_SESSION_SETUP_OK`)_                                                        |
| **3. ส่ง Custom Data**  | `TEST_DATA_999` | แสดงข้อความตอบกลับจาก ESP32 ว่า **`ACK_FROM_ESP32`**                                                                   | \|   \|<br>\|---\|<br>\|`I (...) app: Received custom data: TEST_DATA_999`\|                                                                               |

---

## 8. คำถามท้ายการทดลอง (Post-Lab Questions)
1. การใช้ **Proof-of-Possession (PoP)** ช่วยป้องกันการโจมตีประเภทใดได้บ้าง?
ตอบ  ช่วยป้องกันการโจมตีแบบ **Rogue Provisioning** (บุคคลภายนอกแอบเชื่อมต่อเพื่อแย่งยึดสิทธิ์หรือแก้ไขการตั้งค่าอุปกรณ์) และป้องกันการโจมตีแบบ **Man-in-the-Middle (MitM)** ในระหว่างขั้นตอนการแลกเปลี่ยนกุญแจเข้ารหัส (Key Exchange) เนื่องจากทั้งสองฝั่งจำเป็นต้องใช้ PoP เป็นความลับร่วม (Shared Secret) ในการยืนยันตัวตน

2. หากไม่มีการใช้ PoP (เช่น ใน Security 0) ผู้โจมตีที่อยู่ในรัศมีสัญญาณบลูทูธสามารถทำสิ่งใดกับอุปกรณ์ได้บ้าง?
ตอบ ผู้โจมตีสามารถเชื่อมต่อเข้า ESP32 ได้ทันทีโดยไม่ต้องยืนยันตัวตน สามารถดักอ่านข้อมูล Wi-Fi SSID และ Password ที่ส่งผ่านอากาศในรูปแบบข้อความธรรมดา (Plaintext) ได้โดยตรง และสามารถสั่งให้อุปกรณ์เปลี่ยนไปเชื่อมต่อกับ Access Point ปลอมของผู้โจมตี (Rogue AP) เพื่อดักจับข้อมูลทราฟฟิกทั้งหมดในอนาคต

3. ในการประยุกต์ใช้งานเชิงพาณิชย์จริง เราสามารถนำ **Custom Data Endpoint** ไปใช้ส่งข้อมูลประเภทใดได้อีกบ้าง (ยกตัวอย่าง 2 กรณี)?
ตอบ 
- **กรณีที่ 1 (Device-to-Cloud Binding):** ส่ง User Account Token หรือ Owner ID จากสมาร์ตโฟน เพื่อผูกกรรมสิทธิ์อุปกรณ์เข้ากับบัญชีคลาวด์ของผู้ใช้งานทันทีหลังเชื่อมต่ออินเทอร์เน็ตสำเร็จ
    
- **กรณีที่ 2 (Broker / Server Configuration):** ส่งพารามิเตอร์โครงสร้างพื้นฐาน เช่น MQTT Broker URL, หมายเลขพอร์ต, หรือ Device Activation License Key ประจำเครื่อง เพื่อกำหนดเซิร์ฟเวอร์ปลายทางในการรับส่งข้อมูล Telemetry

4. ในฟังก์ชัน `custom_prov_data_handler()` เหตุใดหน่วยความจำที่จัดสรรให้ `*outbuf` จึงถูก Free โดย Protocomm Layer อัตโนมัติหลังจากส่งข้อมูลเสร็จ?
ตอบ เป็นไปตามหลักการ **Transfer of Ownership** (การส่งมอบกรรมสิทธิ์) เนื่องจากข้อมูลตอบกลับต้องจัดสรรบน Heap Memory (`strdup()` หรือ `malloc()`) เพื่อไม่ให้ข้อมูลถูกทำลายเมื่อฟังก์ชัน Handler สิ้นสุดการทำงาน แต่ Handler เองไม่สามารถสั่ง `free()` ทิ้งได้ทันทีเพราะข้อมูลต้องรอให้ Protocomm นำไปเข้ารหัสและส่งแพ็กเก็ตผ่านฮาร์ดแวร์ก่อน ตัว Protocomm Layer จึงต้องรับหน้าที่เข้ามาเรียกคำสั่ง `free(*outbuf)` คืนหน่วยความจำสู่ระบบเมื่อส่งข้อมูลเสร็จ เพื่อป้องกันปัญหา Memory Leak ในตัวไมโครคอนโทรลเลอร์
