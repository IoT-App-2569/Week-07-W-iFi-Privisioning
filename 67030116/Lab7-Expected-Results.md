# Week 07: ผลที่คาดว่าจะได้ (Lab 7.1 - 7.4)

> **สถานะ: ผลที่คาดว่าจะได้ ยังไม่ได้ทดสอบบนบอร์ดจริง**
> ทุกค่าเป็นการคาดการณ์จากโค้ดและใบงาน ต้องแฟลชลงบอร์ดแล้วนำค่าจริงไปกรอกในตารางของใบงาน ห้ามใช้เป็นผลการทดลอง

## สิ่งสำคัญ: เปลี่ยน Library จากใบงาน

ESP-IDF v6 เอา `wifi_provisioning` ออกจากตัว IDF แล้ว จึงเปลี่ยนโค้ดไปใช้ `espressif/network_provisioning` ซึ่ง IDF จะดาวน์โหลดให้ตอน build ครั้งแรก (ต้องมีอินเทอร์เน็ต) ชื่อในโค้ดจึงต่างจากใบงาน:

| ในใบงาน | ในโค้ด |
| :--- | :--- |
| `wifi_prov_mgr_*` | `network_prov_mgr_*` |
| `wifi_prov_mgr_is_provisioned` | `network_prov_mgr_is_wifi_provisioned` |
| `WIFI_PROV_CRED_RECV / SUCCESS / FAIL` | `NETWORK_PROV_WIFI_CRED_RECV / SUCCESS / FAIL` |
| `WIFI_PROV_EVENT`, `WIFI_PROV_START`, `WIFI_PROV_END` | `NETWORK_PROV_EVENT`, `NETWORK_PROV_START`, `NETWORK_PROV_END` |

ลำดับเหตุการณ์ควรเหมือนเดิม และควรใช้กับแอป ESP SoftAP/BLE Provisioning ได้ แต่ส่วนนี้ยังไม่ได้ทดสอบกับมือถือจริง

## Lab 7.1: Reset Mechanisms

แฟลช Lab 7.2 แล้ว Provision ให้สำเร็จก่อน จึงแฟลช Lab 7.1 ด้วย `idf.py flash` (ไม่ลบ NVS) เพื่อให้มีข้อมูลให้ล้าง

| รูปแบบ | คำสั่ง | LED 1 (GPIO 2) | สถานะใน NVS |
| :--- | :--- | :--- | :--- |
| CLI Erase | `idf.py erase-flash` แล้วแฟลชใหม่ | ดับ | ว่าง: `[STATUS]: Device is NOT provisioned` |
| Menuconfig | เปิด `Reset Provisioned state` | ดับ | ถูกล้างทุกครั้งที่บูต: Log `CONFIG_EXAMPLE_RESET_PROVISIONED is set` แล้วตามด้วย `NOT provisioned` |
| ปุ่ม GPIO 18 | กดค้าง 3 วินาทีตอนบูต | ดับหลังล้าง | Log `Holding button...` และ `FACTORY RESET TRIGGERED!` แล้วตามด้วย `NOT provisioned` |

ถ้ายังมีข้อมูล Wi-Fi: Log `Already provisioned!` และ LED 1 ติด 200 ms ดับ 800 ms เมื่อได้ IP

`idf.py erase-flash` ลบเฟิร์มแวร์ด้วย ต้องแฟลชใหม่ก่อนดูผล

## Lab 7.2: SoftAP Provisioning

| รายการ | ค่าที่คาดว่าจะได้ |
| :--- | :--- |
| SSID | `PROV_XXXXXX` (6 หลักท้ายของ MAC แบบ Hex) |
| PoP | `abcd1234` |
| QR Payload | `{"ver":"v1","name":"PROV_XXXXXX","pop":"abcd1234","transport":"softap"}` |
| LED 3 (GPIO 5) | ช่วงรอ: ติด/ดับ 500 ms, ช่วงส่งข้อมูล: ติด/ดับ 100 ms |
| IP | IP จาก Router ของเครือข่ายที่เลือก |
| เวลาทั้งหมด | ประมาณ 20 - 60 วินาที ขึ้นกับความเร็วการกดในแอป |

Log ตามลำดับ: `SoftAP Provisioning Started!`, `Mobile Phone connected`, `[CREDENTIALS RECEIVED]`, `[SUCCESS]`, `[ONLINE]: Got IP`

ถ้าใส่รหัส Wi-Fi ผิด: Log `[ERROR]: Wi-Fi Connection failed with provided credentials!`

## Lab 7.3: BLE Provisioning

| รายการ | ค่าที่คาดว่าจะได้ |
| :--- | :--- |
| BLE Name | `PROV_XXXXXX` |
| Service UUID | `021a9004-0382-4aea-bff4-6b3f1c5adfb4` |
| Characteristic (0x2901) | `prov-session`, `prov-config`, `proto-ver` และอาจมี `prov-scan` |
| LED 2 (GPIO 4) | ช่วงรอ: ติด/ดับ 500 ms, ช่วงเชื่อมต่อ BLE: ติด/ดับ 100 ms |
| Log คืนหน่วยความจำ Bluetooth | มี: `De-initializing BLE & Releasing BT Memory...` |

## Lab 7.4: Custom Data & Security

| สถานการณ์ | PoP | ผลบนแอป | Log |
| :--- | :--- | :--- | :--- |
| PoP ผิด | `wrong1234` | สร้าง Session ไม่สำเร็จ | `[SECURITY ALERT]: INVALID PoP / Unauthorized Access!` |
| PoP ถูก | `abcd1234` | ไปขั้นเลือก Wi-Fi ได้ | `[SECURITY SUCCESS]: Valid PoP! Secured Session OK!` |
| Custom Data | `TEST_DATA_999` | ได้รับ `ACK_FROM_ESP32_SUCCESS` | `[CUSTOM DATA RECEIVED]: TEST_DATA_999` |

## สิ่งที่แก้ในโค้ดเพิ่ม

- **Lab 7.1:** เพิ่ม `main/Kconfig.projbuild` และ `CONFIG_EXAMPLE_RESET_PROVISIONED` เนื่องจากใบงานให้ทดลองตัวเลือกนี้ แต่โค้ดเดิมไม่มี
- **ทุก Lab:** เพิ่ม `main/idf_component.yml` เพื่อระบุ Dependency
