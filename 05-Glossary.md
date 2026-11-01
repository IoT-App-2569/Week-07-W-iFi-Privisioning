# 05: อภิธานศัพท์และคำย่อทางเทคนิค (Glossary)

| คำศัพท์ / คำย่อ          | คำเต็ม                                      | คำอธิบายและความหมาย                                                                                                               |
| :----------------------- | :------------------------------------------ | :-------------------------------------------------------------------------------------------------------------------------------- |
| **Wi-Fi Provisioning**   | Wi-Fi Provisioning                          | กระบวนการส่งมอบข้อมูลการตั้งค่าเครือข่าย Wi-Fi (SSID, Password, Security) ไปยังอุปกรณ์ IoT ใหม่                                   |
| **Protocomm**            | Protocol Communication                      | เลเยอร์การสื่อสารเชิงนามธรรม (Abstraction Layer) ของ ESP-IDF ที่จัดการความปลอดภัยและการส่งต่อ Endpoint โดยไม่ขึ้นกับสื่อส่งสัญญาณ |
| **Protobuf**             | Protocol Buffers (Google)                   | รูปแบบการแปลงโครงสร้างข้อมูลเป็นไบนารีขนาดเล็ก พัฒนาโดย Google มีประสิทธิภาพสูงกว่า JSON/XML                                      |
| **Nanopb**               | Small-footprint Protocol Buffers            | ไลบรารี C สำหรับแปลง Protobuf ที่กินทรัพยากร RAM/ROM ต่ำมาก เหมาะกับไมโครคอนโทรลเลอร์                                             |
| **BLE**                  | Bluetooth Low Energy                        | เทคโนโลยีบลูทูธพลังงานต่ำ ใช้เป็นสื่อกลางในการ Provisioning อุปกรณ์ IoT ยุคใหม่                                                   |
| **GATT**                 | Generic Attribute Profile                   | โครงสร้างข้อมูลใน BLE ที่แบ่งการทำงานออกเป็น Services และ Characteristics                                                         |
| **UUID**                 | Universally Unique Identifier               | หมายเลขระบุเอกลักษณ์เฉพาะขนาด 128 บิต ใช้สำหรับระบุ GATT Services และ Characteristics                                             |
| **SoftAP**               | Software Access Point                       | โหมดที่ไมโครคอนโทรลเลอร์ทำหน้าที่กระจายสัญญาณ Wi-Fi เป็น Access Point จำลอง                                                       |
| **PoP**                  | Proof-of-Possession                         | รหัสลับเฉพาะเครื่อง (Shared Secret) เพื่อยืนยันว่าผู้สั่ง Provisioning อยู่หน้าอุปกรณ์จริง                                        |
| **X25519**               | Curve25519 ECDH                             | อัลกอริทึมการแลกเปลี่ยนคีย์แบบวงรี (Elliptic Curve Diffie-Hellman) ที่มีความเร็วและปลอดภัยสูง                                     |
| **AES-CTR**              | Advanced Encryption Standard - Counter Mode | โหมดการเข้ารหัสแบบ Stream Cipher ป้องกันการดักฟังข้อมูล                                                                           |
| **SRP6a**                | Secure Remote Password Protocol             | โปรโตคอลพิสูจน์ตัวตนแบบ Zero-Knowledge Proof ป้องกันการเปิดเผยรหัสผ่านจริงบนเครื่องเป้าหมาย                                       |
| **AES-GCM**              | AES Galois/Counter Mode                     | โหมดการเข้ารหัสแบบ Authenticated Encryption ที่ตรวจจับการดัดแปลงข้อมูลได้ในตัว                                                    |
| **NVS**                  | Non-Volatile Storage                        | ระบบจัดเก็บข้อมูลแบบ Key-Value บน Flash Memory ของ ESP32 ทนทานต่อการดับของกระแสไฟฟ้า                                              |
| **State Machine**        | Finite State Machine (FSM)                  | แบบจำลองสถานะการทำงานของระบบที่มีเงื่อนไขการเปลี่ยนสถานะชัดเจน                                                                    |
| **Endpoint**             | Communication Endpoint                      | ช่องทางการรับส่งข้อมูลเสมือนที่ระบุด้วยชื่อ เช่น `prov-session`, `prov-config`                                                    |
| **Headless Device**      | Headless Device                             | อุปกรณ์อิเล็กทรอนิกส์ที่ไม่มีหน้าจอแสดงผลและไม่มีแป้นพิมพ์สำหรับป้อนข้อมูล                                                        |
| **Zero-Knowledge Proof** | Zero-Knowledge Proof (ZKP)                  | วิธีการพิสูจน์ว่าตนเองรู้ความลับ โดยไม่ต้องเปิดเผยความลับนั้นให้อีกฝ่ายเห็น                                                       |
| **Salt**                 | Cryptographic Salt                          | ข้อมูลสุ่ม (เช่น 16 ไบต์) ที่นำมาผสมกับรหัสผ่านก่อนคำนวณ Cryptographic Hash หรือ Verifier เพื่อป้องกันการโจมตีแบบ Rainbow Table |
| **SRP Verifier**         | Secure Remote Password Verifier             | ค่าทางคณิตศาสตร์ที่คำนวณจากรหัสผ่านร่วมกับ Salt สำหรับฝังใน ESP32 เพื่อยืนยันตัวตนใน Security 2 โดยไม่ต้องเก็บรหัสผ่านจริงไว้ใน Flash  |
| **Wear-Leveling**        | Flash Memory Wear-Leveling                  | กลไกการกระจายการเขียน/ลบข้อมูลลงบน Flash Memory ทุกบล็อกอย่างทั่วถึง เพื่อยืดอายุการใช้งานและป้องกันเซกเตอร์ชำรุดจากการเขียนซ้ำ  |
