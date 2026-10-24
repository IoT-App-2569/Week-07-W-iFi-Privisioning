## 6. ตารางบันทึกผลการทดลอง (Experiment Results)

| รูปแบบการ Reset                  | คำสั่ง / พฤติกรรมที่ทำ               | พฤติกรรมของ LED แต่ละดวงหลังเปิดเครื่อง | สถานะใน Serial Monitor |
| :------------------------------- | :----------------------------------- | :-------------------------------------- | :--------------------- |
| **1. CLI Erase**                 | `idf.py erase-flash`                 |   ไม่มีไฟกระพริบ เพราะเฟิร์มแวร์ถูกลบไปด้วย ต้อง idf.py flash ใหม่ก่อน — หลัง flash ใหม่ LED2 (Provisioning indicator) จะกระพริบเข้าสู่โหมด Provisioning เพราะไม่มี Wi-Fi credential เหลืออยู่                         |    ก่อน flash ใหม่ ไม่มี output หรือ error เช่น invalid header หลัง flash ใหม่: ขึ้น log Starting WiFi provisioning พร้อม SSID/QR code                    |
| **2. Menuconfig Flag**           | `CONFIG_EXAMPLE_RESET_PROVISIONED=y` |   App ยังรันปกติ แต่ในโค้ดจะสั่งลบ credential ใน NVS ตอน boot แล้วเข้าสู่โหมด Provisioning ทันทีที่ LED2 กระพริบแบบ Provisioning แทนที่จะเข้า LED_STA_MODE_CONNECTED/DISCONNECTED                                      |       log แสดง Resetting provisioned credentials... ตามด้วย Starting provisioning                 |
| **3. Hardware Button (GPIO 18)** | กดปุ่ม GPIO 18 ค้าง 3 วินาที         |      ระหว่างกด LED ยังทำงานตามสถานะเดิม เช่น Heartbeat ถ้าเชื่อมต่ออยู่ เมื่อครบ 3 วิ ระบบลบ NVS provisioning แล้ว reboot LED กลับไปกระพริบโหมด Provisioning                                   |   นับเวลากดปุ่มใน log เช่น Holding... 1s 2s 3s ตามด้วย Factory reset triggered, restarting                     |

---

## 7. คำถามท้ายการทดลอง (Post-Lab Questions)
1. เพราะเหตุใดการกดปุ่ม BOOT (GPIO 0) ค้างไว้ในจังหวะรีเซ็ตบอร์ด จึงทำให้โปรแกรมค้างอยู่ที่ ROM Bootloader และไม่ยอมทำงานต่อ?
```text
เพราะการกดค้างไว้ทำให้ขา GPIO 0 มีสถานะเป็น LOW ขณะรีเซ็ต ซึ่งเป็นเงื่อนไขฮาร์ดแวร์บังคับให้ชิปเข้าโหมดแฟลชโปรแกรม (Download Mode) เพื่อรอรับเฟิร์มแวร์ใหม่แทนที่จะบูตโปรแกรมเดิม
```
2. เพราะเหตุใดคำสั่ง `idf.py erase-flash` จึงทำให้ข้อมูลเฟิร์มแวร์ Application หายไปด้วย ในขณะที่ `nvs_flash_erase()` ไม่ทำให้เฟิร์มแวร์หาย?
```text
เพราะ idf.py erase-flash สั่งลบข้อมูลทั้งหมดทุกพาร์ทิชันบนชิป Flash (รวมโค้ดโปรแกรม) ในขณะที่ nvs_flash_erase() สั่งลบเฉพาะพื้นที่พาร์ทิชัน NVS ที่ใช้เก็บค่าคอนฟิกเท่านั้น
```
3. การออกแบบปุ่ม Factory Reset บนอุปกรณ์ IoT เชิงพาณิชย์ เหตุใดจึงต้องกำหนดให้ผู้ใช้กดปุ่มค้างไว้ 3-5 วินาที แทนที่จะสั่งลบข้อมูลทันทีที่แตะปุ่มเพียงเสี้ยววินาที?
```text
เพื่อป้องกันการลบข้อมูลโดยไม่ได้ตั้งใจจากการเผลอไปสัมผัส การสั่นสะเทือน หรือสัญญาณรบกวนทางไฟฟ้า (Contact Bounce)
```
4. หากอุปกรณ์ IoT ถูกติดตั้งอยู่บนเสาสูงหรือฝังอยู่ในผนัง วิธีการ Reset ทางกายภาพรูปแบบใดเหมาะสมที่สุด?
```text
การตัด-ต่อไฟเลี้ยงตามจังหวะที่กำหนด (Power-cycle sequence) หรือการใช้แม่เหล็กแตะสั่งงานสวิตช์ Reed switch ภายนอกกล่องเคส
```

