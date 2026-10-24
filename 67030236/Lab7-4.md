### ภารกิจที่ 1: ผังขั้นตอนการตรวจสอบ PoP (Security Handshake Decision Flow)
ให้นักศึกษาวาด Flowchart แสดงการแลกเปลี่ยนคีย์และตรวจสอบสิทธิ์:
1. การสร้าง Session Parameters ด้วยค่า PoP (`abcd1234`)
2. เมื่อ Client ส่ง Public Key + Verification Hash มาให้ ESP32
3. จุดแยกทางเลือก (Branching):
   - หาก PoP ไม่ตรง $\rightarrow$ Trigger Event `PROTOCOMM_SECURITY_SESSION_CREDENTIALS_MISMATCH` และปฏิเสธการเชื่อมต่อ
   - หาก PoP ถูกต้อง $\rightarrow$ Trigger Event `PROTOCOMM_SECURITY_SESSION_SETUP_OK` และสร้าง AES Session Key สำเร็จ

<img width="455" height="608" alt="image" src="https://github.com/user-attachments/assets/1ca452b8-333e-41f2-9354-11393fccce24" />

  

### ภารกิจที่ 2: ผังการรับส่งข้อมูลผ่าน Custom Endpoint (Custom Data Handler Flow)
ให้นักศึกษาวาด Sequence / Data Flow ของฟังก์ชัน `custom_prov_data_handler()`:
1. ข้อมูล `inbuf` ถูกส่งเข้ามาจากสมาร์ตโฟนผ่าน Protocomm
2. การพิมพ์ข้อความ Log ด้วย `ESP_LOGI()`
3. การจัดสรรหน่วยความจำแบบไดนามิกด้วย `strdup()` ให้กับ `*outbuf`
4. ทำไมตัวแปร `*outbuf` จึงต้องจัดสรรใน Heap Memory (ทำไมจึงใช้ตัวแปร Local Static Array ธรรมดาไม่ได้)?

<img width="1272" height="396" alt="image" src="https://github.com/user-attachments/assets/6e12fa0f-a3f9-48b4-a84b-ad486b453b39" />


---

## 7. ตารางบันทึกผลการทดลอง (Experiment Results)

| สถานการณ์ทดสอบ | ค่า PoP ที่ป้อน | ผลลัพธ์บนแอปมือถือ | ข้อความ Log ใน Serial Monitor |
| :--- | :--- | :--- | :--- |
| **1. ป้อน PoP ผิดพลาด** | `wrong1234` | ขึ้นแจ้งเตือนข้อผิดพลาด (Authentication Failed / Incorrect PoP) ไม่สามารถผ่านเข้าหน้าตั้งค่า Wi-Fi ได้ | E (...) app: Received incorrect username and/or PoP for establishing secure session! |
| **2. ป้อน PoP ถูกต้อง** | `abcd1234` | เชื่อมต่อสำเร็จ ผ่านเข้าสู่หน้าเลือกเครือข่าย Wi-Fi ได้ปกติ | I (...) app: Secured session established! |
| **3. ส่ง Custom Data** | `TEST_DATA_999` | ขึ้นสถานะตอบกลับสำเร็จ (ACK / Success) จากอุปกรณ์ | I (...) app: Received custom data: TEST_DATA_999 |

---

## 8. คำถามท้ายการทดลอง (Post-Lab Questions)
1. การใช้ **Proof-of-Possession (PoP)** ช่วยป้องกันการโจมตีประเภทใดได้บ้าง?
```text
ช่วยป้องกันการโจมตีแบบ Man-in-the-Middle (MitM), การดักฟังข้อมูล (Eavesdropping), การโจมตีซ้ำ (Replay Attack) และการสวมรอยหรือควบคุมอุปกรณ์โดยไม่ได้รับอนุญาต (Unauthorized Provisioning)
```
2. หากไม่มีการใช้ PoP (เช่น ใน Security 0) ผู้โจมตีที่อยู่ในรัศมีสัญญาณบลูทูธสามารถทำสิ่งใดกับอุปกรณ์ได้บ้าง?
```text
ผู้โจมตีสามารถดักฟังข้อมูลที่ส่งเป็นข้อความธรรมดา (Plaintext) เช่น SSID และรหัสผ่าน Wi-Fi หรือชิงเชื่อมต่อเพื่อส่งค่าคอนฟิกและเข้าควบคุมอุปกรณ์ได้ทันทีโดยไม่มีการยืนยันตัวตน
```
3. ในการประยุกต์ใช้งานเชิงพาณิชย์จริง เราสามารถนำ **Custom Data Endpoint** ไปใช้ส่งข้อมูลประเภทใดได้อีกบ้าง (ยกตัวอย่าง 2 กรณี)?
```text
สามารถนำไปใช้ส่ง Device Activation Token (หรือ User Account Binding ID) เพื่อผูกอุปกรณ์เข้ากับบัญชีคลาวด์ของผู้ใช้ทันที และส่ง Initial Device Configuration (เช่น พิกัด Timezone หรือ URL ของ MQTT Broker เฉพาะองค์กร) ในขั้นตอนเริ่มต้นได้
```
4. ในฟังก์ชัน `custom_prov_data_handler()` เหตุใดหน่วยความจำที่จัดสรรให้ `*outbuf` จึงถูก Free โดย Protocomm Layer อัตโนมัติหลังจากส่งข้อมูลเสร็จ?
```text
เพราะ Protocomm Layer ถูกออกแบบโครงสร้างกรรมสิทธิ์หน่วยความจำ (Memory Ownership) ให้ผู้เรียกใช้งานเป็นผู้รับผิดชอบในการล้างบัฟเฟอร์ เพื่อป้องกันปัญหาหน่วยความจำรั่วไหล (Memory Leak) หลังจากการส่งข้อมูลแพ็กเก็ตตอบกลับไปยังไคลเอนต์เสร็จสิ้น
```

