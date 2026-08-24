## 📖 สรุปเนื้อหาใบงาน (Labs Overview)

### จัดทำโดย : นายพรพรม สุขใจมิตร 67030323

คลิกที่ลิงก์โฟลเดอร์เพื่อไปยังโค้ด หรือลิงก์รายงานเพื่อดูผลการทดลองแบบละเอียด:

### 🛠️ [Lab 1 & 2: พื้นฐาน Wi-Fi Provisioning (SoftAP)](./Lab/Lab-2/)

- **เนื้อหาหลัก:** การเปิดใช้งาน ESP32 ในโหมด Access Point ชั่วคราว (SoftAP) เพื่อให้สมาร์ตโฟนเชื่อมต่อและส่งข้อมูล SSID/Password ของเร้าเตอร์หลักไปให้ 
- **สิ่งที่ได้เรียนรู้:** โครงสร้างของ `network_provisioning` component ใน ESP-IDF v6.0+ 
- **📄 อ่านรายงาน:** [Report/Lab-1.md](./Report/Lab-1.md) | [Report/Lab-2.md](./Report/Lab-2.md) 

---

### 🛜 [Lab 3: การคอนฟิก Wi-Fi ผ่าน BLE Scheme & BLE Forensics](./Lab/Lab-3/)

- **เนื้อหาหลัก:** เปลี่ยนรูปแบบการ Provisioning จาก Wi-Fi มาใช้ **Bluetooth Low Energy (BLE)** เพื่อความสะดวกของผู้ใช้ (ไม่ต้องสลับ Wi-Fi ในมือถือ) 
- **ไฮไลท์:** 
  - ใช้เครื่องมือ **nRF Connect** ส่องโครงสร้าง GATT Services และ Characteristics 
  - การคืนหน่วยความจำ Bluetooth (`esp_bt_mem_release`) ทันทีที่ตั้งค่าเสร็จเพื่อประหยัด RAM 
  - การขยาย Partition Table เป็น 2MB (`partitions.csv`) เพื่อรองรับ BLE Stack 
- **📄 อ่านรายงาน:** [Report/Lab-3.md](./Report/Lab-3.md) 

---

### 🔒 [Lab 4: Security Schemes (PoP) และ Custom Data Endpoints](./Lab/Lab-4/)

- **เนื้อหาหลัก:** เจาะลึกระบบรักษาความปลอดภัย **Security 1** โดยใช้ Proof-of-Possession (PoP) ป้องกันการสวมรอย 
- **ไฮไลท์:** 
  - จำลองการโจมตีโดยป้อน PoP ผิดพลาด และดูการปฏิเสธการเชื่อมต่อของระบบ 
  - การสร้าง **Custom Data Endpoint** (`custom-data`) เพื่อรับส่งข้อมูลเฉพาะกิจระหว่างมือถือและบอร์ด (เช่น ส่ง Student ID) 
- **📄 อ่านรายงาน:** [Report/Lab-4.md](./Report/Lab-4.md) 

---