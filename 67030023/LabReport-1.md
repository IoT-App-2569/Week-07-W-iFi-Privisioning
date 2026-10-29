### ภารกิจที่ 1  ผังงานการตัดสินใจช่วง Bootstrapping & Reset Decision
ให้นักศึกษาวาด Flowchart แสดงลำดับตรรกะการตรวจสอบเงื่อนไขตั้งแต่เริ่มต้นรันฟังก์ชัน `app_main()` โดยต้องครอบคลุม:
1. การตรวจสอบสถานะปุ่ม **GPIO 18** (ตรวจจับการกดค้าง 3 วินาที)
2. การทำงานของ `nvs_flash_init()` และกรณีที่ต้อง `nvs_flash_erase()`
3. การตรวจสอบ Macro `#ifdef CONFIG_EXAMPLE_RESET_PROVISIONED`
4. การเรียกฟังก์ชัน `wifi_prov_mgr_is_provisioned(&provisioned)`
5. จุดแยกสายการทำงานเข้าสู่โหมด **Provisioning Mode** หรือ **Station Mode**

```text
flowchart TD
    Start([เริ่มรัน app_main]) --> InitNVS[เรียก nvs_flash_init]
    
    InitNVS --> CheckNVS{พบ NVS_ERR_NO_FREE_PAGES <br/>หรือ NVS_ERR_NEW_VERSION_FAILED?}
    CheckNVS -- ใช่ --> EraseNVS[nvs_flash_erase]
    EraseNVS --> ReInitNVS[nvs_flash_init อีกครั้ง]
    CheckNVS -- ไม่ใช่/เสร็จสิ้น --> CheckBtn[ตรวจสอบสถานะปุ่ม GPIO 18]
    ReInitNVS --> CheckBtn

    CheckBtn --> BtnPressed{มีการกด GPIO 18 <br/>ค้างไว้ ≥ 3 วินาที?}
    BtnPressed -- ใช่ --> ResetProv1[เรียก wifi_prov_mgr_reset_provisioning]
    ResetProv1 --> CheckMacro
    BtnPressed -- ไม่ใช่ --> CheckMacro{มี Macro <br/>#ifdef CONFIG_EXAMPLE_RESET_PROVISIONED?}

    CheckMacro -- มี --> ResetProv2[เรียก wifi_prov_mgr_reset_provisioning]
    ResetProv2 --> CheckProv
    CheckMacro -- ไม่มี --> CheckProv[เรียก wifi_prov_mgr_is_provisioned &provisioned]

    CheckProv --> IsProv{สถานะ <br/>provisioned == true?}
    IsProv -- ใช่ --> StaMode[เข้าสู่ Station Mode <br/> เชื่อมต่อ Wi-Fi AP ที่บันทึกไว้]
    IsProv -- ไม่ใช่ --> ProvMode[เข้าสู่ Provisioning Mode <br/> เปิด SoftAP/BLE รับค่า Wi-Fi ใหม่]
```

### ภารกิจที่ 2 ผังสถานะการเปลี่ยนจังหวะไฟ LED 1 (Wi-Fi STA Indicator)
ให้นักศึกษาวาด State Diagram แสดงการเปลี่ยนสถานะของ **LED 1 (GPIO 2)**:
- เงื่อนไขใดทำให้ LED 1 เข้าสู่สถานะ `LED_STA_MODE_DISCONNECTED` (กระพริบ 200ms Mark / 200ms Space)
- เงื่อนไขหรือ Event ใดทำให้เปลี่ยนเป็น `LED_STA_MODE_CONNECTED` (Heartbeat 200ms ทุก 1s)
stateDiagram-v2
    [*] --> LED_STA_MODE_DISCONNECTED : เริ่มระบบ / ยังไม่เคยเชื่อมต่อ Wi-Fi
    
    LED_STA_MODE_DISCONNECTED --> LED_STA_MODE_CONNECTED : ได้รับ Event IP_EVENT_STA_GOT_IP
    LED_STA_MODE_CONNECTED --> LED_STA_MODE_DISCONNECTED : ได้รับ Event WIFI_EVENT_STA_DISCONNECTED

    state LED_STA_MODE_DISCONNECTED {
        [*] --> DisconnectedBlink
        DisconnectedBlink : กระพริบถี่แบบสถานะค้นหา
        DisconnectedBlink : High 200ms / Low 200ms
    }

    state LED_STA_MODE_CONNECTED {
        [*] --> ConnectedHeartbeat
        ConnectedHeartbeat : กระพริบจังหวะ Heartbeat
        ConnectedHeartbeat : High 200ms แล้ว Low 800ms (รวม 1s)
    }
---

## 6. ตารางบันทึกผลการทดลอง (Experiment Results)
รูปแบบการ Reset,คำสั่ง / พฤติกรรมที่ทำ,พฤติกรรมของ LED แต่ละดวงหลังเปิดเครื่อง,สถานะใน Serial Monitor
1. CLI Erase,idf.py erase-flash,- LED 1 (GPIO 2): กระพริบ 200ms Mark / 200ms Space (ยังไม่มี Wi-Fi)- LED 2 (Provisioning): สว่างค้าง (เข้าสู่ Provisioning Mode),แสดงข้อความ Provisioning started พร้อม QR Code/PoP บน Terminal เพื่อรอการรับค่า SSID/Password
2. Menuconfig Flag,CONFIG_EXAMPLE_RESET_PROVISIONED=y,- LED 1: กระพริบถี่ 200ms/200ms- LED 2: สว่างค้าง,แสดง Logs ลบค่า Credentials เก่า: Resetting provisioning state... และเข้าสู่ Starting provisioning...
3. Hardware Button (GPIO 18),กดปุ่ม GPIO 18 ค้าง 3 วินาที,- LED 1: เปลี่ยนจาก Heartbeat เป็นกระพริบถี่ 200ms/200ms- LED 2: สว่างขึ้นมาค้างไว้,ตรวจจับการกดค้างได้ แสดง Log: Reset button pressed! Resetting provisioning... และเริ่มกระบวนการ Provisioning ใหม่
---

## 7. คำถามท้ายการทดลอง (Post-Lab Questions)
1. เพราะเหตุใดการกดปุ่ม BOOT (GPIO 0) ค้างไว้ในจังหวะรีเซ็ตบอร์ด จึงทำให้โปรแกรมค้างอยู่ที่ ROM Bootloader และไม่ยอมทำงานต่อ?

คำตอบ เนื่องจากขา GPIO 0 ถูกออกแบบให้เป็น Strapping Pin ของชิป ESP32 เพื่อเลือกโหมดการ Boot เมื่อกดปุ่ม BOOT ดึงแรงดันขา GPIO 0 ลงเป็น LOW ในขณะ รีเซ็ต Hardware ตัวชิปจะเปลี่ยนเส้นทางการ Boot จาก Internal Flash ไปเป็น ROM Serial Bootloader Mode เพื่อรอรับการ Flash โปรแกรมใหม่ผ่าน Serial (UART) ทำให้โปรแกรมหลักใน Flash ไม่ถูกโหลดขึ้นมาทำงาน

2. เพราะเหตุใดคำสั่ง idf.py erase-flash จึงทำให้ข้อมูลเฟิร์มแวร์ Application หายไปด้วย ในขณะที่ nvs_flash_erase() ไม่ทำให้เฟิร์มแวร์หาย?

คำตอบ idf.py erase-flash เป็นคำสั่งลบข้อมูลในหน่วยความจำ Flash ทั้งหมด (Entire Flash Chip Erasure) ตั้งแต่แอดเดรส 0x000000 รวมถึง Bootloader, Partition Table และ Application Binary

nvs_flash_erase() จะสั่งลบเฉพาะพื้นที่ Partition ที่ชื่อว่า nvs ตามที่ระบุไว้ใน Partition Table (พื้นที่จัดเก็บ Key-Value Pair เช่น Wi-Fi Credentials) โดยไม่ส่งผลกระทบต่อพื้นที่ Partition factory หรือ ota_0 ที่ใช้เก็บตัวโค้ดเฟิร์มแวร์

3. การออกแบบปุ่ม Factory Reset บนอุปกรณ์ IoT เชิงพาณิชย์ เหตุใดจึงต้องกำหนดให้ผู้ใช้กดปุ่มค้างไว้ 3-5 วินาที แทนที่จะสั่งลบข้อมูลทันทีที่แตะปุ่มเพียงเสี้ยววินาที?

คำตอบ เพื่อป้องกัน Accidental Trigger (การลบข้อมูลโดยอุบัติเหตุ) จากกรณีมือไปโดนปุ่มโดยไม่ได้ตั้งใจ ปัญหาแรงสั่นสะเทือน หรือสัญญาณรบกวนทางไฟฟ้า (Contact Bounce) การหน่วงเวลา 3–5 วินาที เป็นการยืนยันเจตนาที่แท้จริงของผู้ใช้ว่าต้องการลบข้อมูลตั้งค่าทั้งหมด (Reset to Factory Defaults)

4. หากอุปกรณ์ IoT ถูกติดตั้งอยู่บนเสาสูงหรือฝังอยู่ในผนัง วิธีการ Reset ทางกายภาพรูปแบบใดเหมาะสมที่สุด?

คำตอบ ควรใช้เทคนิคดังต่อไปนี้แทนการกดปุ่มบนตัวเครื่อง:

Power Cycle Sequences (Reset ผ่านสวิตช์ไฟ): ตรวจจับจังหวะการเปิด-ปิดสวิตช์ไฟ AC ซ้ำกันอย่างรวดเร็ว เช่น ปิด-เปิดไฟติดต่อกัน 5 ครั้งภายใน 10 วินาที เพื่อเข้าสู่ Factory Reset Mode

Software/Remote Reset: สั่งการผ่านโปรโตคอลการสื่อสาร เช่น แอปพลิเคชันมือถือ (BLE/Wi-Fi Local Provisioning), MQTT Response Command หรือ OTA Trigger

External Magnetic Sensor (Hall Effect Sensor / Reed Switch): นำแม่เหล็กมาแตะบริเวณตำแหน่งที่ซ่อนอยู่ภายนอกเคสอุปกรณ์ โดยไม่ต้องสัมผัสบอร์ดโดยตรง

