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

![alt text](/Report/img/image-3.png)

![alt text](/Report/img/image-4.png)

---

## 7. ตารางบันทึกผลการทดลอง (Experiment Results)

| สถานการณ์ทดสอบ | ค่า PoP ที่ป้อน | ผลลัพธ์บนแอปมือถือ | ข้อความ Log ใน Serial Monitor |
| :--- | :--- | :--- | :--- |
| **1. ป้อน PoP ผิดพลาด** | `wrong1234` |ขึ้นแจ้งเตือนข้อผิดพลาด (Authentication Failed / Incorrect PoP) ไม่สามารถผ่านเข้าหน้าตั้งค่า Wi-Fi ได้ | E (...) app: Received incorrect username and/or PoP for establishing secure session!|
| **2. ป้อน PoP ถูกต้อง** | `abcd1234` | เชื่อมต่อสำเร็จผ่านเข้าสู่หน้าเลือกเครือข่าย Wi-Fi ได้ปกติ| I (...) app: Secured session established!|
| **3. ส่ง Custom Data** | `TEST_DATA_999` |ขึ้นสถานะตอบกลับสำเร็จ (ACK / Success) จากอุปกรณ์ | I (...) app: Received custom data: TEST_DATA_999|

---

## 8. คำถามท้ายการทดลอง (Post-Lab Questions)
1. การใช้ **Proof-of-Possession (PoP)** ช่วยป้องกันการโจมตีประเภทใดได้บ้าง?
```
ช่วยป้องกันการโจมตีแบบสวมรอย (Rogue Provisioning / Man-in-the-Middle Attack) และการแอบอ้างเชื่อมต่อหรือตั้งค่าอุปกรณ์จากบุคคลภายนอกที่ไม่ได้รับอนุญาต ซึ่งไม่มีรหัสผ่านหรือ QR Code ประจำตัวเครื่อง
```

2. หากไม่มีการใช้ PoP (เช่น ใน Security 0) ผู้โจมตีที่อยู่ในรัศมีสัญญาณบลูทูธสามารถทำสิ่งใดกับอุปกรณ์ได้บ้าง?
```
ผู้โจมตีสามารถเชื่อมต่อเข้ามายังอุปกรณ์เพื่ออ่านข้อมูลภายใน ส่งคำสั่งกำหนดค่า (Configuration) หรือแอบดูสถานะและโครงสร้างของ Endpoint ได้อย่างอิสระโดยไม่ต้องผ่านกระบวนการยืนยันตัวตน
```

3. ในการประยุกต์ใช้งานเชิงพาณิชย์จริง เราสามารถนำ **Custom Data Endpoint** ไปใช้ส่งข้อมูลประเภทใดได้อีกบ้าง (ยกตัวอย่าง 2 กรณี)?
```
1. ส่งข้อมูล MQTT Broker URL หรือ Client Credentials เพื่อให้อุปกรณ์เชื่อมต่อไปยังเซิร์ฟเวอร์เฉพาะของลูกค้าแต่ละรายได้ทันที
2. ส่งข้อมูล Device Owner ID / Activation Token เพื่อผูกบัญชีผู้ใช้งานเข้ากับตัวฮาร์ดแวร์ในระบบ Cloud
```

4. ในฟังก์ชัน `custom_prov_data_handler()` เหตุใดหน่วยความจำที่จัดสรรให้ `*outbuf` จึงถูก Free โดย Protocomm Layer อัตโนมัติหลังจากส่งข้อมูลเสร็จ?
```
เนื่องจากสถาปัตยกรรมของ Protocomm ออกแบบมาให้ตัวกลาง (Protocomm Layer) เป็นผู้รับผิดชอบวงจรชีวิต (Lifecycle) ของบัฟเฟอร์ขาออก เมื่อส่งข้อมูลตอบกลับไปยังฝั่งมือถือเสร็จสิ้น ระบบจะจัดการเคลียร์หน่วยความจำของพอยเตอร์ *outbuf ให้โดยอัตโนมัติเพื่อป้องกันปัญหาหน่วยความจำรั่วไหล (Memory Leak)
```
