# Report ใบงานที่ 7.3 การคอนฟิก Wi-Fi ผ่าน BLE Scheme และการสืบสวน GATT Services (BLE Forensics)
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

<img width="1407" height="262" alt="image" src="https://github.com/user-attachments/assets/1d77b733-ec94-44bc-bb7a-b81fc6af6bf0" />

<img width="826" height="592" alt="image" src="https://github.com/user-attachments/assets/10529c6d-13bf-4626-830f-322c16dc457c" />

---

## 6. ตารางบันทึกผลการทดลอง (Experiment Results)

| รายการตรวจสอบ | ผลการทดลอง / ข้อมูลที่สังเกตได้ |
| :--- | :--- |
| **1. BLE Device Name ที่สแกนเจอ** | `PROV_PROV_A550C4` |
| **2. Primary Service UUID (128-bit)** | 021a9004-0382-4aea-bff4-6b3f1c5adfb4 |
| **3. Characteristic Endpoint ที่พบ (0x2901)** | 1. prov-session<br/> 2. prov-config <br/>3. custom-data|
| **4. พฤติกรรมไฟ LED 2 (GPIO 4) ช่วงรอ vs ช่วงต่อ BLE** | ช่วงรอ: กระพริบช้าๆ หรือตามจังหวะปกติ <br/>ช่วงต่อ:กระพริบเร็วขึ้น (100ms) เมื่อมี BLE มาเชื่อมต่อ |
| **5. พฤติกรรมเมื่อต่อ Wi-Fi สำเร็จ** |มี Log คืนหน่วยความจำ (BTDM memory released) |

---

## 7. คำถามท้ายการทดลอง (Post-Lab Questions)
1. เหตุใด BLE Provisioning จึงไม่ส่งผลให้สัญญาณ Wi-Fi บนสมาร์ตโฟนของผู้ใช้หลุดระหว่างทำรายการ?

   **ตอบ** เพราะการสื่อสารเกิดขึ้นผ่านคลื่น Bluetooth (BLE) ซึ่งแยกส่วนกันกับการเชื่อมต่อ Wi-Fi ของมือถือ ทำให้มือถือยังคงเชื่อมต่อกับเร้าเตอร์และใช้งานอินเทอร์เน็ตได้ตามปกติระหว่างตั้งค่าอุปกรณ์
   
3. Descriptor `0x2901` มีความสำคัญอย่างไรต่อการที่แอปพลิเคชันมือถือจะทราบว่า Characteristic แต่ละตัวใช้ทำหน้าที่อะไร?

    **ตอบ** Descriptor 0x2901 ทำหน้าที่เก็บ ข้อความสตริง เช่น prov-config เพื่อบอกให้แอปรู้ว่า Characteristic หมายเลขนี้ทำหน้าที่เป็น Endpoint สำหรับรับส่งข้อมูลเรื่องอะไร ช่วยให้แอปส่งข้อมูลไปได้ถูกช่องทาง
   
5. การที่ ESP-IDF มีฟังก์ชัน `esp_bt_mem_release()` มีประโยชน์อย่างไรต่อการทำงานของแอปพลิเคชัน IoT หลังเชื่อมต่อ Wi-Fi สำเร็จ?

   **ตอบ** ช่วยลบ Bluetooth Stack ออกจากหน่วยความจำเมื่อไม่ใช้งานแล้ว และคืนพื้นที่ RAM (DRAM) กลับมาให้ระบบ ทำให้แอปพลิเคชันหลักมีพื้นที่หน่วยความจำเหลือเฟือสำหรับการทำงานอื่นๆ ที่ซับซ้อนขึ้น

