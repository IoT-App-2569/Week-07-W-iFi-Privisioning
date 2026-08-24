## 5. กิจกรรมถอดรหัสซอร์สโค้ดและเขียนผังงาน (Code Deconstruction & Flowchart Assignment)

ให้นักศึกษาศึกษาโค้ดใน `main/main.c` และ `main/led_indicator.c` แล้วเขียน **ผังงาน (Flowchart / State Diagram)** เพื่ออธิบายการตัดสินใจและการทำงานของระบบ:

### ภารกิจที่ 1  ผังงานการตัดสินใจช่วง Bootstrapping & Reset Decision
ให้นักศึกษาวาด Flowchart แสดงลำดับตรรกะการตรวจสอบเงื่อนไขตั้งแต่เริ่มต้นรันฟังก์ชัน `app_main()` โดยต้องครอบคลุม:
1. การตรวจสอบสถานะปุ่ม **GPIO 18** (ตรวจจับการกดค้าง 3 วินาที)
2. การทำงานของ `nvs_flash_init()` และกรณีที่ต้อง `nvs_flash_erase()`
3. การตรวจสอบ Macro `#ifdef CONFIG_EXAMPLE_RESET_PROVISIONED`
4. การเรียกฟังก์ชัน `wifi_prov_mgr_is_provisioned(&provisioned)`
5. จุดแยกสายการทำงานเข้าสู่โหมด **Provisioning Mode** หรือ **Station Mode**

![alt text](/Report/img/image.png)

### ภารกิจที่ 2 ผังสถานะการเปลี่ยนจังหวะไฟ LED 1 (Wi-Fi STA Indicator)
ให้นักศึกษาวาด State Diagram แสดงการเปลี่ยนสถานะของ **LED 1 (GPIO 2)**:
- เงื่อนไขใดทำให้ LED 1 เข้าสู่สถานะ `LED_STA_MODE_DISCONNECTED` (กระพริบ 200ms Mark / 200ms Space)
- เงื่อนไขหรือ Event ใดทำให้เปลี่ยนเป็น `LED_STA_MODE_CONNECTED` (Heartbeat 200ms ทุก 1s)

```
stateDiagram-v2
    [*] --> LED_STA_OFF : เริ่มทำงาน (Initial State)
    
    LED_STA_OFF --> LED_STA_DISCONNECTED : เมื่อเกิด Event <br> WIFI_EVENT_STA_START
    
    LED_STA_DISCONNECTED --> LED_STA_CONNECTED : เมื่อได้รับ IP Address <br> (IP_EVENT_STA_GOT_IP)
    
    LED_STA_CONNECTED --> LED_STA_DISCONNECTED : เมื่อสัญญาณหลุด <br> (WIFI_EVENT_STA_DISCONNECTED)
    
    note right of LED_STA_DISCONNECTED
        รูปแบบ: กระพริบเร็ว (Alert)
        - ติด 200ms
        - ดับ 200ms
    end note
    
    note right of LED_STA_CONNECTED
        รูปแบบ: จังหวะหัวใจ (Heartbeat)
        - ติด 200ms
        - ดับ 800ms
    end note
```

---

## 6. ตารางบันทึกผลการทดลอง (Experiment Results)

| รูปแบบการ Reset                  | คำสั่ง / พฤติกรรมที่ทำ               | พฤติกรรมของ LED แต่ละดวงหลังเปิดเครื่อง | สถานะใน Serial Monitor |
| :------------------------------- | :----------------------------------- | :-------------------------------------- | :--------------------- |
| **1. CLI Erase**                 | `idf.py erase-flash`                 |                   LED ดับ                         |      บอร์ดไม่สามารถทำงานได้ จนกว่าจะสั่ง idf.py flash เข้าไปใหม่               | 
| **2. Menuconfig Flag**           | `CONFIG_EXAMPLE_RESET_PROVISIONED=y` |              LED 1 กระพริบเร็ว (Alert) เพื่อรอการ Provisioning ใหม่เสมอทุกครั้งที่เปิดเครื่อง                           |     [STATUS]: Device is NOT provisioned (NVS is empty)                   |
| **3. Hardware Button (GPIO 18)** | กดปุ่ม GPIO 18 ค้าง 3 วินาที         |         LED 1 กระพริบเร็ว (Alert) เพื่อกลับสู่โหมด Provisioning                                |      >>> FACTORY RESET TRIGGERED! ERASING NVS FLASH <<< ตามด้วย Device is NOT provisioned                  |

---

## 7. คำถามท้ายการทดลอง (Post-Lab Questions)
1. เพราะเหตุใดการกดปุ่ม BOOT (GPIO 0) ค้างไว้ในจังหวะรีเซ็ตบอร์ด จึงทำให้โปรแกรมค้างอยู่ที่ ROM Bootloader และไม่ยอมทำงานต่อ?
```
ขา GPIO 0 บนชิป ESP32 ทำหน้าที่เป็น Strapping Pin สำหรับเลือกโหมดการบูต หากขานี้มีสถานะลอจิกเป็น Low (0) ในช่วงที่จ่ายไฟหรือรีเซ็ต ชิปจะเข้าใจว่าผู้ใช้ต้องการเข้าสู่โหมด "ROM Serial Bootloader" เพื่อรอรับการแฟลชโปรแกรมใหม่ผ่านทาง USB ทำให้ตัวประมวลผลหยุดทำงาน และไม่ยอมเข้าไปรันคำสั่งในฟังก์ชัน app_main() ตามปกติ
```

2. เพราะเหตุใดคำสั่ง `idf.py erase-flash` จึงทำให้ข้อมูลเฟิร์มแวร์ Application หายไปด้วย ในขณะที่ `nvs_flash_erase()` ไม่ทำให้เฟิร์มแวร์หาย?
```
คำสั่ง idf.py erase-flash เป็นการสั่งลบข้อมูลแบบหมดจด (Wipe) ทุก Partition บนหน่วยความจำ Flash ตั้งแต่ Address แรกจนถึงสุดท้าย รวมถึงตัวโปรแกรมหลักด้วย ในขณะที่ฟังก์ชัน nvs_flash_erase() เป็นคำสั่งทางซอฟต์แวร์ที่เจาะจงลบข้อมูลเฉพาะในพื้นที่พาร์ทิชันที่ชื่อว่า "nvs" (Non-Volatile Storage) เท่านั้น ซึ่งเป็นที่เก็บข้อมูลคอนฟิกต่างๆ โดยไม่กระทบกับพื้นที่พาร์ทิชัน "factory" ที่เก็บตัวโปรแกรมหลัก
```

3. การออกแบบปุ่ม Factory Reset บนอุปกรณ์ IoT เชิงพาณิชย์ เหตุใดจึงต้องกำหนดให้ผู้ใช้กดปุ่มค้างไว้ 3-5 วินาที แทนที่จะสั่งลบข้อมูลทันทีที่แตะปุ่มเพียงเสี้ยววินาที?
```
เพื่อป้องกันความผิดพลาดจากการเผลอไปโดนปุ่มโดยไม่ได้ตั้งใจ (Accidental Press) เช่น มือไปปัดโดน หรือปุ่มถูกกดทับระหว่างการขนย้าย การหน่วงเวลา 3-5 วินาทีช่วยยืนยันเจตนาที่แท้จริงของผู้ใช้ว่าต้องการจะลบข้อมูลสำคัญทิ้งจริงๆ
```

4. หากอุปกรณ์ IoT ถูกติดตั้งอยู่บนเสาสูงหรือฝังอยู่ในผนัง วิธีการ Reset ทางกายภาพรูปแบบใดเหมาะสมที่สุด?
```
หากไม่สามารถเข้าถึงอุปกรณ์เพื่อกดปุ่มได้อย่างสะดวก วิธีที่เหมาะสมคือการใช้ "Power Cycle Reset Sequence" หรือการเปิด-ปิดสวิตช์ไฟของอุปกรณ์นั้นๆ ติดต่อกันหลายๆ ครั้งตามรูปแบบที่กำหนดไว้ (เช่น ปิด-เปิดไฟ 3 ครั้งติดต่อกัน) โดยโปรแกรมในอุปกรณ์จะนับจำนวนครั้งการรีบูต หากทำครบตามเงื่อนไข อุปกรณ์จะล้างข้อมูล Factory Reset ให้เองโดยไม่ต้องปีนขึ้นไปกดปุ่ม
```
