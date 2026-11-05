## Build command 

```powershell
# ตั้ง Root ที่ D:\GithubRepos\ENGEDU\03376134_APP_IOT\Week-07-W-iFi-Privisioning
docker run --rm    --mount "type=bind,source=$((Get-Location).Path),target=/workspace"  -w /workspace/Example_codes/Lab7-1-Reset-and-NVS-Forensics  espressif/idf:release-v5.5  idf.py -B build-v5 build
```

## **Flash command (Window)**
Esp32 connected to COM6


```powershell
cd D:\GithubRepos\ENGEDU\03376134_APP_IOT\Week-07-W-iFi-Privisioning\Example_codes\Lab7-1-Reset-and-NVS-Forensics

 python -m esptool --chip esp32 -b 460800 --before default_reset --after hard_reset write_flash --flash_mode dio --flash_size 2MB --flash_freq 40m 0x1000 build-v5/bootloader/bootloader.bin 0x8000 build-v5/partition_table/partition-table.bin 0x10000 build-v5/lab7_1_reset_nvs_forensics.bin && idf monitor -p COM6
```
