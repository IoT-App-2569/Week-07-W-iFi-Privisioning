### ภารกิจที่ 1: ผังโครงสร้าง GATT Tree & Endpoint Mapping
ให้นักศึกษาวาดโครงสร้างต้นไม้ (Tree Diagram / Block Diagram) แสดงความสัมพันธ์ระหว่าง:

Primary Service (128-bit UUID: 021a9004-...)
Characteristic UUIDs แต่ละตัว
Descriptor 0x2901 (User Description) ที่ผูกเข้ากับ Protocomm Endpoints (prov-session, prov-config, prov-scan, proto-ver, custom-data)

<img width="1276" height="433" alt="image" src="https://github.com/user-attachments/assets/00abeff7-1ddf-48d2-a179-3a8ed65c4020" />

ภารกิจที่ 2: ผังลำดับการคืนหน่วยความจำ Bluetooth (BLE Lifecycle & Memory Reclaim Flow)
ให้นักศึกษาวาด Flowchart / Sequence แสดงว่า:

การเชื่อมต่อ BLE ถูกตรวจพบผ่าน Event PROTOCOMM_TRANSPORT_BLE_CONNECTED (LED 2 กระพริบเร็ว 100ms)
เมื่อเชื่อมต่อ Wi-Fi สำเร็จ (WIFI_PROV_CRED_SUCCESS) 
→
 เกิด Event WIFI_PROV_END
Provisioning Manager สั่งเรียก esp_bt_mem_release() เพื่อปล่อย DRAM คืนสู่ระบบอย่างไร

<img width="220" height="616" alt="image" src="https://github.com/user-attachments/assets/b883f610-7934-4540-bf1d-7804dd2a542f" />


## 6. ตารางบันทึกผลการทดลอง (Experiment Results)

| รายการตรวจสอบ | ผลการทดลอง / ข้อมูลที่สังเกตได้ |
| :--- | :--- |
| **1. BLE Device Name ที่สแกนเจอ** | `PROV_PROV_1D88 |
| **2. Primary Service UUID (128-bit)** | 021a9004-0382-4aea-bff4-6b3f1c5adfb4 |
| **3. Characteristic Endpoint ที่พบ (0x2901)** |1. prov-session
2. prov-config
3. prov-scan |
| **4. พฤติกรรมไฟ LED 2 (GPIO 4) ช่วงรอ vs ช่วงต่อ BLE** | ช่วงรอ: กระพริบช้าๆ
ช่วงต่อ: ติดสว่างค้าง หรือกระพริบเร็วขึ้นเมื่อมีสมาร์ตโฟนเชื่อมต่อ |
| **5. พฤติกรรมเมื่อต่อ Wi-Fi สำเร็จ** | มี ปรากฏ Log wifi_prov_scheme_ble: BT memory released, BTDM memory released |

---

## 7. คำถามท้ายการทดลอง (Post-Lab Questions)
1. เหตุใด BLE Provisioning จึงไม่ส่งผลให้สัญญาณ Wi-Fi บนสมาร์ตโฟนของผู้ใช้หลุดระหว่างทำรายการ?
```text
เพราะ BLE ส่งข้อมูลผ่านชิปและช่องสัญญาณบลูทูธแยกต่างหาก จึงไม่มีผลกระทบต่อการเชื่อมต่อของเสาสัญญาณ Wi-Fi บนสมาร์ตโฟน
```
2. Descriptor `0x2901` มีความสำคัญอย่างไรต่อการที่แอปพลิเคชันมือถือจะทราบว่า Characteristic แต่ละตัวใช้ทำหน้าที่อะไร?
```text
Descriptor 0x2901 (Characteristic User Description) ทำหน้าที่เก็บข้อความอธิบายความหมายที่มนุษย์อ่านเข้าใจ (เช่น "prov-session" หรือ "Temperature") ทำให้แอปพลิเคชันมือถือสามารถอ่านและจับคู่ฟังก์ชันการทำงานของ Characteristic แต่ละตัวได้ถูกต้องโดยไม่ต้องฮาร์ดโค้ด UUID
```
3. การที่ ESP-IDF มีฟังก์ชัน `esp_bt_mem_release()` มีประโยชน์อย่างไรต่อการทำงานของแอปพลิเคชัน IoT หลังเชื่อมต่อ Wi-Fi สำเร็จ?
```text
มีประโยชน์ในการคืนหน่วยความจำ RAM ขนาดใหญ่ที่บลูทูธเคยจองไว้ กลับมาเป็นฮีป (Heap) ว่างให้ระบบ เพื่อให้แอปพลิเคชันหลักของ IoT มีแรมเหลือใช้อย่างเต็มประสิทธิภาพ
```
