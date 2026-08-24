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


![alt text](/Report/img/image-2.png)

---

## 6. ตารางบันทึกผลการทดลอง (Experiment Results)

| รายการตรวจสอบ | ผลการทดลอง / ข้อมูลที่สังเกตได้ |
| :--- | :--- |
| **1. BLE Device Name ที่สแกนเจอ** | PROV_1D88 |
| **2. Primary Service UUID (128-bit)** | 021a9004-0382-4aea-bff4-6b3f1c5adfb4 |
| **3. Characteristic Endpoint ที่พบ (0x2901)** | 1. prov-session<br/>2. prov-config3. prov-scan (หรือเพิ่ม proto-ver, custom-data) |
| **4. พฤติกรรมไฟ LED 2 (GPIO 4) ช่วงรอ vs ช่วงต่อ BLE** | ช่วงรอ: .กระพริบช้าๆ<br/>ช่วงต่อ: ติดสว่างค้าง หรือกระพริบเร็วขึ้นเมื่อมีสมาร์ตโฟนเชื่อมต่อ |
| **5. พฤติกรรมเมื่อต่อ Wi-Fi สำเร็จ** |มี (wifi_prov_scheme_ble: BT memory released, BTDM memory released) |

---

## 7. คำถามท้ายการทดลอง (Post-Lab Questions)
1. เหตุใด BLE Provisioning จึงไม่ส่งผลให้สัญญาณ Wi-Fi บนสมาร์ตโฟนของผู้ใช้หลุดระหว่างทำรายการ?
```
เพราะกระบวนการสื่อสารทั้งหมดวิ่งอยู่บนคลื่นความถี่วิทยุ Bluetooth Low Energy (BLE) คนละย่านและคนละโปรโตคอลกับการเชื่อมต่อ Wi-Fi หลักของสมาร์ตโฟน ผู้ใช้จึงสามารถใช้งานอินเทอร์เน็ตมือถือหรือแอปพลิเคชันอื่นพร้อมกับทำ Provisioning ไปด้วยได้โดยที่ Wi-Fi ไม่ขาดตอน
```

2. Descriptor `0x2901` มีความสำคัญอย่างไรต่อการที่แอปพลิเคชันมือถือจะทราบว่า Characteristic แต่ละตัวใช้ทำหน้าที่อะไร?
```
Descriptor 0x2901 (Characteristic User Description) ทำหน้าที่เก็บข้อความอธิบายแบบอ่านออกเสียงได้ (Human-readable text) เช่น "prov-session" หรือ "prov-config" ทำให้แอปพลิเคชันบนสมาร์ตโฟนสามารถอ่านข้อความเหล่านี้เพื่อจับคู่และระบุหน้าที่ของแต่ละ Characteristic ได้ถูกต้องโดยไม่ต้องฮาร์ดโค้ด UUID
```

3. การที่ ESP-IDF มีฟังก์ชัน `esp_bt_mem_release()` มีประโยชน์อย่างไรต่อการทำงานของแอปพลิเคชัน IoT หลังเชื่อมต่อ Wi-Fi สำเร็จ?
```
ช่วยคืนหน่วยความจำ RAM (DRAM) ที่เคยถูกจองไว้สำหรับระบบ Bluetooth Stack ทั้งหมดกลับคืนสู่ระบบหลัก (Heap) ทำให้ไมโครคอนโทรลเลอร์มีพื้นที่หน่วยความจำเหลือเฟือสำหรับรันฟังก์ชันหลักของแอปพลิเคชัน IoT ตัวเองได้อย่างเสถียรและมีประสิทธิภาพมากขึ้น
```
